#include "cvproject.h"
#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

namespace CV {
namespace Core {

CVProject::CVProject(QObject *parent) : QObject(parent) {

}

void CVProject::loadFrom(const QDir& dir) {
	QString db = dir.absolutePath() + dir.separator() + SQL::database;
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(db.toStdString()); 
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
			QStringList() << "ID" << "DATE" << "URI" << "NOTE" << "CONTROL",
			QStringList() << "JOURNAL", 
			QStringList() << "CONTROL = ?1",
			QVariantList() << 1
		);
		
		if (!set.eof()) {
			name = dir.dirName();
			id = QString(set[0].toString().c_str());
			timestamp = set[1].toInt64();
			path = dir.absolutePath();
			notes = QString(set[3].toString().c_str());
			type = set[4].toInt() == 1 ? Core::CVProject::PHOTOGRAMMETRY : Core::CVProject::LIDAR;
		} 
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return;
	}
}

bool CVProject::create(const QString& d) {
	QDir dir;
	bool ret = dir.mkdir(path);
	if (!ret) {
		return false;
	}
	dir.cd(path);

	QString db = dir.absolutePath() + dir.separator() + d;

	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.create(db.toStdString()); 
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

	QResource sql(":/sql/db.sql");
	QFile res(sql.absoluteFilePath());
	if (!res.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}
	QTextStream str(&res);
	QString query = str.readAll();

	QStringList tables = query.split(";", QString::SkipEmptyParts);
	cnn.begin_transaction();
	foreach(QString t, tables) {
		try {
			cnn.execute_immediate(t.simplified().toStdString());
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			cnn.rollback_transaction();
			return false;
		}
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

	return true;
}

void CVProject::insert(CVCategory* category) {
	_categories.insert(category->type(), category);
}

CVCategory* CVProject::get(CVCategory::Type t) {
	return _categories.value(t);
}

} // namespace Core
} // namespace CV
