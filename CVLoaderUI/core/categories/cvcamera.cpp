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
	return _isValid; 
}

bool CVCamera::persist() { 
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString()); 
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
			QStringList() << "FOC=?1" << "DIMX=?2" << "DIMY=?3" << "DPIX=?4" << "XP=?5" << "YP=?6"  << "SERIAL_NUMBER=?7" << "MODEL=?8" << "DESCR=?9" << "PLANNING=?10",
			QStringList() << "ID=?11",
			QVariantList()  << _cam.foc << _cam.dimx << _cam.dimy << _cam.dpix << _cam.xp << _cam.yp << QString(_cam.serial.c_str()) << QString(_cam.model.c_str()) << QString(_cam.descr.c_str()) << _cam.planning
							<< QString(_cam.id.c_str())
		);
		
	} else {	
		ret = q->insert(
			"CAMERA", 
			QStringList() << "ID" << "FOC" << "DIMX" << "DIMY" << "DPIX" << "XP" << "YP" << "SERIAL_NUMBER" << "MODEL" << "DESCR" << "PLANNING",
			QStringList() << "?1" << "?2" << "?3" << "?4" << "?5" << "?6" << "?7" << "?8" << "?9" << "?10" << "?11",
			QVariantList() << QUuid::createUuid().toString() << _cam.foc << _cam.dimx << _cam.dimy << _cam.dpix << _cam.xp << _cam.yp << QString(_cam.serial.c_str()) << QString(_cam.model.c_str()) << QString(_cam.descr.c_str()) << _cam.planning
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
		cnn.open(uri().toStdString()); 
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
			QStringList() << "ID" << "FOC" << "DIMX" << "DIMY" << "DPIX" << "XP" << "YP" << "SERIAL_NUMBER" << "MODEL" << "DESCR" << "PLANNING",
			QStringList() << "CAMERA", 
			QStringList(),// << "CONTROL = ?1",
			QVariantList()// << 1
		);

		if (!set.eof()) {
			int i = 0;
			_cam.id = set[i].toString();
			_cam.foc = set[++i].toDouble();
			_cam.dimx = set[++i].toDouble();
			_cam.dimy = set[++i].toDouble();
			_cam.dpix = set[++i].toDouble();
			_cam.xp = set[++i].toDouble();
			_cam.yp = set[++i].toDouble();
			_cam.serial = set[++i].toString();
			_cam.model = set[++i].toString();
			_cam.descr = set[++i].toString();
			_cam.planning = set[++i].toInt() ? true : false;
			
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
