#include "cvsensor.h"

#include "core/cvjournal.h"
#include "core/categories/cvcontrol.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include  "assert.h"

namespace CV {
namespace Core {

CVSensor::CVSensor(QObject *parent) : CVMissionDevice(parent) {
	_isValid = false;
	isPlanning(false);
}

CVSensor::~CVSensor() {

}

bool CVSensor::isValid() const {
	return _isValid;
}

bool CVSensor::load(const QString& mId) {
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	bool ret = cnn.is_valid();
	if (!ret) {
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	QStringList fields;
	fields << "ID" << "FOV" << "IFOV" << "FREQ" << "SCAN_RATE";
	if (isPlanning()) {
		fields << "SPEED";
	}

	try {
		CV::Util::Spatialite::Recordset set = q->select(
			fields,
			QStringList() << "SENSOR", 
			QStringList() << "ID = ?1",
			QVariantList() << mId
		);

		if (!set.eof()) {
			int i = 0;

			_sensor.id = set[i].toString().c_str();
			_sensor.fov = set[++i].toDouble();
			_sensor.ifov = set[++i].toDouble();
			_sensor.freq = set[++i].toDouble();
			_sensor.scan_rate = set[++i].toDouble();
			if (isPlanning()) { 
				_sensor.speed = set[++i].toDouble();
			}

			_isValid = true;
		} 

	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	return ret; 
}

bool CVSensor::persist() {
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	bool ret = cnn.is_valid();
	if (!ret) {
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	try {
		if (isValid()) {
			QStringList fields; fields << "FOV=?1" << "IFOV=?2" << "FREQ=?3" << "SCAN_RATE=?4" << "PLANNING=?5";
			QStringList idx; idx << "ID=?6";
			QVariantList vals; vals << _sensor.fov << _sensor.ifov << _sensor.freq << _sensor.scan_rate << isPlanning() << _sensor.id;

			if (isPlanning()) {
				fields << "SPEED=?7";
				vals << _sensor.speed;
			}
			ret = q->update( //TODO: real id
				"SENSOR", 
				fields,
				idx,
				vals
			);
			
		} else {	
			_sensor.id = QUuid::createUuid().toString();

			QStringList fields; fields << "ID" << "FOV" << "IFOV" << "FREQ" << "SCAN_RATE" << "PLANNING";
			QStringList idx; idx << "?1" << "?2" << "?3" << "?4" << "?5" << "?6";
			QVariantList vals; vals << _sensor.id << _sensor.fov << _sensor.ifov << _sensor.freq << _sensor.scan_rate << isPlanning();

			if (isPlanning()) {
				fields << "SPEED";
				idx << "?7";
				vals << _sensor.speed;
			}

			ret = q->insert(
				"SENSOR", 
				fields,
				idx,
				vals
			);
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}
	
	_isValid = ret;
	if (!ret) {
		return false;
	}

	if (!isPlanning()) {
		assert(!_mission.isEmpty());
		ret = q->update( //TODO: real id
			"MISSION", 
			QStringList() << "ID_SENSOR=?1",
			QStringList() << "ID=?2",
			QVariantList()  << _sensor.id << _mission
		);
	}
	
	/*Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
	e->control = isPlanning() ? Core::CVControl::LIDAR_PLAN : Core::CVControl::LIDAR_GPS_DATA;
	e->object = Core::CVObject::SENSOR;
	e->uri = uri();
	Core::CVJournal::add(e);*/

	log();
	return ret;
}

bool CVSensor::load() {
	_isValid = false;

	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();

	bool ret = cnn.is_valid();
	if (!ret) {
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);

	QStringList fields;
	fields << "ID" << "FOV" << "IFOV" << "FREQ" << "SCAN_RATE";

	if (isPlanning()) {
		fields << "SPEED";
	}

	try {
		CV::Util::Spatialite::Recordset set = q->select(
			fields,
			QStringList() << "SENSOR", 
			QStringList() << "PLANNING = ?1",
			QVariantList() << 1
		);

		if (!set.eof()) {
			int i = 0;
			_sensor.id = set[i].toString().c_str();
			_sensor.fov = set[++i].toDouble();
			_sensor.ifov = set[++i].toDouble();
			_sensor.freq = set[++i].toDouble();
			_sensor.scan_rate = set[++i].toDouble();

			if (isPlanning()) {
				_sensor.speed = set[++i].toDouble();
			}
			
			_isValid = true;
		} 

	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	return ret; 
}

bool CVSensor::remove() {
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	bool ret = cnn.is_valid();
	if (!ret) {
		return false;
	}

	ret = q->remove(
		"SENSOR",
		QStringList() << "ID = ?1",
		QVariantList() << _sensor.id
	);

	_sensor = SensorData();
	_isValid = false;
	return ret; 
}

} // namespace Core
} // namespace CV
