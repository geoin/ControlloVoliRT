#include "cvproject.h"
#include "core/sql/querybuilder.h"
#include "core/cvjournal.h"

#include "CVUtil/cvspatialite.h"

namespace CV {
namespace Core {

CVProject::CVProject(QObject *parent) : QObject(parent) {

}

QString CVProject::loadFrom(const QDir& dir) {
	_db = dir.absolutePath() + dir.separator() + SQL::database;
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(_db.toStdString()); 
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return QString();
	}
	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	try {
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "ID" << "DATE" << "URI" << "NOTE" << "CONTROL",
			QStringList() << "JOURNAL", 
			QStringList() << "CONTROL < ?1",
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
			QStringList() << "NOTE" << "SCALE",
			QStringList() << "PROJECT", 
			QStringList(),
			QVariantList()
		);
		
		if (!set.eof()) {
			notes = QString(set[0].toString().c_str());
			scale = QString(set[1].toString().c_str());
		} 

		return _db;
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return QString();
	}
}

bool CVProject::_addResource(const QString& res, CV::Util::Spatialite::Connection& cnn) {
	QResource sql(":/sql/db.sql");
	QFile res(sql.absoluteFilePath());
	if (!res.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}
	QTextStream str(&res);
	QString query = str.readAll();

	QStringList tables = query.split(";", QString::SkipEmptyParts);
	foreach(QString t, tables) {
		try {
			if (!t.isEmpty()) {
				cnn.execute_immediate(t.simplified().toStdString());
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

	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.create(_db.toStdString()); 
		if (cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA) {
			cnn.initialize_metdata(); 
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	bool isValid = cnn.is_valid();
	if (!isValid) {
		return false;
	}

	cnn.begin_transaction();

	QResource sql(":/sql/db.sql");
	QFile res(sql.absoluteFilePath());
	if (!res.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}
	QTextStream str(&res);
	QString query = str.readAll();

	QStringList tables = query.split(";", QString::SkipEmptyParts);
	foreach(QString t, tables) {
		try {
			if (!t.isEmpty()) {
				cnn.execute_immediate(t.simplified().toStdString());
			}
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			cnn.rollback_transaction();
			return false;
		}
	}

	res.close();

	QResource udpSql(":/sql/update.sql");
	QFile upd(udpSql.absoluteFilePath());
	if (!upd.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}

	str.setDevice(&upd);
	query = str.readAll();

	tables = query.split(";", QString::SkipEmptyParts);
	foreach(QString t, tables) {
		try {
			if (!t.isEmpty()) {
				cnn.execute_immediate(t.simplified().toStdString());
			}
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			cnn.rollback_transaction();
			return false;
		}
	}

	cnn.commit_transaction();

	upd.close();

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
		QStringList() << "NAME" << "NOTE" << "SCALE" << "TYPE",
		QStringList() << "?1" << "?2" << "?3" << "?4",
		QVariantList() << name << notes << scale << type
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
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.create(db().toStdString()); 
		if (cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA) {
			cnn.initialize_metdata(); 
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return;
	}

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
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.create(db().toStdString()); 
		if (cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA) {
			cnn.initialize_metdata(); 
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	try {
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "DATE",
			QStringList() << "JOURNAL", 
			QStringList() << "CONTROL=?1",
			QVariantList() << 1,
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
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(db().toStdString()); 
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
	}

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
		e->control = PHOTOGRAMMETRY;
		e->uri = name;
		e->db = _db;
		Core::CVJournal::add(e);
	}
	return ret;
}

QString CVProject::projectNotes() {
	return QString();
}

} // namespace Core
} // namespace CV
