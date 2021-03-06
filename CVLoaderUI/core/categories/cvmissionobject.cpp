#include "cvmissionobject.h"

#include "cvcamera.h"
#include "cvsensor.h"
#include "cvrinex.h"
#include "cvstations.h"
#include "cvshapelayer.h"

#include "core/sql/querybuilder.h"
#include "core/cvproject.h"

#include <QByteArray>

namespace CV {
namespace Core {

CVMissionObject::CVMissionObject(QObject* p) : CVObject(p), _isValid(false) {
	_id = QUuid::createUuid().toString();
	missionType(CVControl::GPS_DATA); //set default to photogrammetry
}

CVMissionObject::CVMissionObject(QObject* p, QString key) : CVObject(p), _isValid(false) {
	_id = key;
	missionType(CVControl::GPS_DATA);
}

bool CVMissionObject::remove() { 
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	bool ret = q->remove(
		"MISSION", 
		QStringList() << "ID=?1",
		QVariantList() << _id
	);

	ret = q->remove(
		"STATION", 
		QStringList() << "ID_MISSION=?1",
		QVariantList() << _id
	);

	return ret;
}

bool CVMissionObject::persist() { 
	bool ret = false;

	CV::Util::Spatialite::Connection cnn;
	const QString& db = uri();
	try {
		cnn.open(db.toStdString()); 
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	ret = q->insert(
		"MISSION", 
		QStringList() << "ID" << "NAME" << "RINEX",
		QStringList() << "?1" << "?2" << "?3",
		QVariantList() << id() << name() << '0'
	);

	if (ret) {
		/*Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
		e->control = Core::CVControl::GPS_DATA;  
		e->object = Core::CVObject::MISSION;
		e->uri = name();
		//e->db = db;
		Core::CVJournal::add(e);*/

		log("", "Missione " + name());
	}

	return ret; 
}

bool CVMissionObject::isValid() const  {  
	return _isValid; 
}

bool CVMissionObject::load() { 
	bool ret = false;

	CV::Util::Spatialite::Connection cnn;
	try {
		const QString& db = uri();
		cnn.open(db.toStdString()); 
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
    CV::Util::Spatialite::Recordset set = q->select(
		QStringList() << "NAME" << _deviceId << "RINEX",
		QStringList() << "MISSION",
		QStringList() << "id=?1",
        QVariantList() << id()
	);

	QString idDevice;
	QByteArray ba;
	if (!set.eof()) {
		name(set[0].toString().c_str());
		idDevice = QString(set[1].toString().c_str());
		const std::vector<unsigned char>& b = set[2];
		const char* ptr = reinterpret_cast<const char*>(&b[0]);
		ba.append(ptr, b.size());
		ret = true;
	}

	if (!idDevice.isEmpty()) {
		CVMissionDevice* dev = NULL;
		if (missionType() == CVControl::GPS_DATA) {
			dev = static_cast<CVCamera*>(at(0));
		} else { 
			dev = static_cast<CVSensor*>(at(0));
		}
		dev->mission(id());
		dev->load(idDevice);
		dev->controlType(missionType());
	}

	if (ba.length() > 2) {
		CVRinex* r = static_cast<CVRinex*>(at(1));
		r->mission(id());
		r->controlType(missionType());
		r->load();
	}

	set = q->select(
		QStringList() << "ID" << "NAME",
		QStringList() << "STATION",
		QStringList() << "ID_MISSION=?1",
		QVariantList() << id()
	);

	CVStations* stations = static_cast<CVStations*>(at(2));
	while (!set.eof()) {
		CVStation* s = new CVStation(stations, QString(set[0].toString().c_str()));
		s->type(CVObject::STATION);
		s->controlType(missionType());
		s->mission(id());
		s->uri(uri());
		s->name(QString(set[1].toString().c_str()));
		stations->add(s);
		set.next();
	}

	_isValid = ret;
	return ret; 
}

void CVMissionObject::init() {
	CVMissionDevice* dev = NULL; 
	if (missionType() == CVControl::GPS_DATA) {
		dev = new CVCamera(this);
		dev->type(CVObject::CAMERA);
	} else {
		dev = new CVSensor(this);
		dev->type(CVObject::SENSOR);
	}
	dev->isPlanning(false);
	//dev->uri(uri());
	dev->mission(id());
	insert(dev);

	CVRinex* rinex = new CVRinex(this);
	rinex->uri(uri());
	rinex->type(CVObject::FLY_RINEX);
	rinex->mission(id());
	insert(rinex);

	CVStations* stations = new CVStations(this);
	stations->mission(id());
	stations->type(CVObject::STATION);
	insert(stations);
}
	
}
}
