#include "cvproject.h"
#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QDir>

namespace CV {
namespace Core {

CVProject::CVProject(QObject *parent) : QObject(parent) {

}

void CVProject::loadFrom(const QDir& dir, const QString& d) {
	QString db = dir.absolutePath() + dir.separator() + d;
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
		} else {
			return;
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return;
	}
}

} // namespace Core
} // namespace CV
