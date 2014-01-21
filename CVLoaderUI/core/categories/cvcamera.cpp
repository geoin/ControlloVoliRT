#include "cvcamera.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QUuid>
#include <QdateTime>

namespace CV {
namespace Core {

CVCamera::CVCamera(QObject* p) : CVObject(p) {
	_isValid = false;
}

CVCamera::~CVCamera() {

}

bool CVCamera::isValid() const { 
	return true; 
}

bool CVCamera::persist() { 
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.create(uri().toStdString()); 
		if (cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA) {
			cnn.initialize_metdata(); 
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	bool ret = cnn.is_valid();
	if (!ret) {
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	if (isValid()) {
		ret = q->update( //TODO: real id
			"CAMERA", 
			QStringList() << "FOC=?1" << "DIMX=?2" << "DIMY=?3" << "DPIX=?4" << "XP=?5" << "YP=?6",
			QStringList() << "ID=?7",
			QVariantList() << _cam.foc << _cam.dimx << _cam.dimy << _cam.dpix << _cam.xp << _cam.yp << "{3584ed05-2166-4029-9189-a4b142599bf0}"//QUuid::createUuid().toString() 
		);
		
	} else {	
		ret = q->insert(
			"CAMERA", 
			QStringList() << "ID" << "FOC" << "DIMX" << "DIMY" << "DPIX" << "XP" << "YP",
			QStringList() << "?1" << "?2" << "?3" << "?4" << "?5" << "?6" << "?7",
			QVariantList() << QUuid::createUuid().toString() << _cam.foc << _cam.dimx << _cam.dimy << _cam.dpix << _cam.xp << _cam.yp
		);
	}
	if (!ret) {
		return false;
	}

	_isValid = ret;

	ret = q->insert(
		"JOURNAL", 
		QStringList() << "ID" << "DATE" << "URI" << "NOTE" << "CONTROL" << "OBJECT",
		QStringList() << "?1" << "?2" << "?3" << "?4" << "?5" << "?6",
		QVariantList() << QUuid::createUuid().toString() << QDateTime::currentMSecsSinceEpoch() << "" << "" << 1 << 0
	);
	return ret; 
}

bool CVCamera::load() { 
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.create(uri().toStdString()); 
		if (cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA) {
			cnn.initialize_metdata(); 
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	bool ret = cnn.is_valid();
	if (!ret) {
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);

	try {
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "ID" << "FOC" << "DIMX" << "DIMY" << "DPIX" << "XP" << "YP",
			QStringList() << "CAMERA", 
			QStringList(),// << "CONTROL = ?1",
			QVariantList()// << 1
		);

		if (!set.eof()) {
			int i = 0;
			_cam.foc = set[++i].toDouble();
			_cam.dimx = set[++i].toDouble();
			_cam.dimy = set[++i].toDouble();
			_cam.dpix = set[++i].toDouble();
			_cam.xp = set[++i].toDouble();
			_cam.yp = set[++i].toDouble();
			
			_isValid = true;
		} 

	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	return ret; 
}

} // namespace Core
} // namespace CV
