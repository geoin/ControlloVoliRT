#include "cvproject.h"
#include "core/sql/querybuilder.h"
#include "core/cvjournal.h"

namespace CV {
namespace Core {

CVProject::CVProject(QObject *parent) : QObject(parent), timestamp(0), datum(32632) {

}

QString CVProject::loadFrom(const QDir& dir) {
	_db = dir.absolutePath() + dir.separator() + SQL::database;
	if (!SQL::Database::open(_db)) {
		return QString();
	}

	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);

	try {
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "ID" << "DATE" << "URI" << "NOTE" << "CONTROL",
			QStringList() << "JOURNAL", 
			QStringList() << "CONTROL < ?1", //TODO: change this
			QVariantList() << CVProject::LIDAR + 1
		);
		
		if (!set.eof()) {
			name = dir.dirName();
			id = QString(set[0].toString().c_str());
			timestamp = set[1].toInt64();
			path = dir.absolutePath();
			//notes = QString(set[3].toString().c_str());
			type = set[4].toInt() == 1 ? Core::CVProject::PHOTOGRAMMETRY : Core::CVProject::LIDAR;
		} 

		set = q->select(
			QStringList() << "NOTE" << "SCALE" << "DATUM",
			QStringList() << "PROJECT", 
			QStringList(),
			QVariantList()
		);
		
		if (!set.eof()) {
			notes = QString(set[0].toString().c_str());
			scale = QString(set[1].toString().c_str());
			datum = set[1].toInt();
		} 

		return _db;
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return QString();
	}
}

bool CVProject::_addResource(const QString& resource, CV::Util::Spatialite::Connection& cnn) {
	QResource sql(resource);
	QFile res(sql.absoluteFilePath());
	if (!res.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}
	QTextStream str(&res);
	QString query = str.readAll();

	QStringList tables = query.split(";", QString::SkipEmptyParts);
	foreach(QString t, tables) {
		try {
			QString q = t.simplified().trimmed();
			if (!q.isEmpty()) {
				cnn.execute_immediate(q.toStdString());
			}
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			cnn.rollback_transaction();
			return false;
		}
	}
	return true;
}

bool CVProject::create(const QString& d) {
	QDir dir;
	bool ret = dir.mkdir(path);
	if (!ret) {
		return false;
	}
	dir.cd(path);

	_db = dir.absolutePath() + dir.separator() + d;

	if (!SQL::Database::create(_db)) {
		return false;
	}

	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	if (cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA) {
		cnn.initialize_metdata(); 
	}

	bool isValid = cnn.is_valid();
	if (!isValid) {
		return false;
	}

	cnn.begin_transaction();

	if (!_addResource(":/sql/db.sql", cnn)) {
		return false;
	}

	if (!_addResource(":/sql/update.sql", cnn)) {
		return false;
	}

	bool projSpec = false;
	if (type == PHOTOGRAMMETRY) {
		_addResource(":/sql/photogrammetry.sql", cnn);
	} else {
		_addResource(":/sql/lidar.sql", cnn);
	}

	cnn.commit_transaction();

	id = QUuid::createUuid().toString();

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	q->insert(
		"JOURNAL", 
		QStringList() << "ID" << "DATE" << "URI" << "NOTE" << "CONTROL",
		QStringList() << "?1" << "?2" << "?3" << "?4" << "?5",
		QVariantList() << id << QDateTime::currentMSecsSinceEpoch() << path << notes << type
	);

	q->insert(
		"PROJECT", 
		QStringList() << "NAME" << "NOTE" << "SCALE" << "TYPE" << "DATUM",
		QStringList() << "?1" << "?2" << "?3" << "?4" << "?5",
		QVariantList() << name << notes << scale << type << datum
	);

	return true;
}

void CVProject::insert(CVControl* control) {
	_controls.insert(control->type(), control);
}

CVControl* CVProject::get(CVControl::Type t) {
	return _controls.value(t);
}

void CVProject::missionList(QStringList& ids) {
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	try {
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "ID",
			QStringList() << "MISSION", 
			QStringList(),
			QVariantList()
		);
		
		while (!set.eof()) {
			ids << set[0].toString().c_str();
			set.next();
		} 
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return;
	}
}

QDateTime CVProject::creationDate() {
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	try {
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "DATE",
			QStringList() << "JOURNAL", 
			QStringList() << "CONTROL=?1",
			QVariantList() << type,
			QStringList() << "DATE ASC",
			1
		);
		
		if (!set.eof()) {
			return QDateTime::fromMSecsSinceEpoch(set[0].toInt64());
		} 

	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
	}
	return QDateTime();
}
	
QDateTime CVProject::lastModificationDate() {
	CVJournalEntry::EntryList l = Core::CVJournal::last(
		_db,
		QStringList(),
		QVariantList(),
		1
	);

	if (l.size()) {
		return l.at(0)->date;
	}
	return QDateTime();
}

bool CVProject::persist() {
	bool ret = false;
	
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	ret = q->update(
		"PROJECT",
		QStringList() << "NOTE=?1" << "SCALE=?2",
		QStringList(),
		QVariantList() << notes << scale
	);

	if (ret) {
		Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
		e->note = notes;
		e->control = type;
		e->uri = name;
		//e->db = _db;
		Core::CVJournal::add(e);
	}
	return ret;
}

QString CVProject::projectNotes() {
	return QString();
}

} // namespace Core
} // namespace CV
