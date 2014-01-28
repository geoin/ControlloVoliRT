#include "cvmissionobject.h"

#include "cvcamera.h"
#include "cvproject.h"
#include "cvrinex.h"
#include "cvstations.h"

#include "core/sql/querybuilder.h"

#include <QByteArray>

namespace CV {
namespace Core {

CVMissionObject::CVMissionObject(QObject* p) : CVObject(p), _isValid(false) {
	_id = QUuid::createUuid().toString();
}

CVMissionObject::CVMissionObject(QObject* p, QString key) : CVObject(p), _isValid(false) {
	_id = key;
}

bool CVMissionObject::persist() { 
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
	ret = q->insert(
		"MISSION", 
		QStringList() << "ID" << "NAME" << "RINEX",
		QStringList() << "?1" << "?2" << "?3",
		QVariantList() << id() << name() << '0'
	);

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
	CV::Util::Spatialite::Recordset& set = q->select(
		QStringList() << "NAME" << "ID_CAMERA" << "RINEX",
		QStringList() << "MISSION",
		QStringList() << "id=?1",
		QVariantList() << id()
	);

	QString idCam;
	QByteArray ba;
	if (!set.eof()) {
		name(set[0].toString().c_str());
		idCam = QString(set[1].toString().c_str());
		const std::vector<unsigned char>& b = set[2];
		for (std::vector<unsigned char>::const_iterator it = b.begin(); it < b.end(); ++it) {
			ba.append(*it);
		}
		ret = true;
	}

	if (!idCam.isEmpty()) {
		CVCamera* cam = static_cast<CVCamera*>(at(0));
		cam->mission(id());
		cam->load(idCam);
	}

	if (ba.length() > 1) {
		CVRinex* r = static_cast<CVRinex*>(at(1));
		r->mission(id());
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
		s->mission(id());
		s->name(QString(set[1].toString().c_str()));
		stations->add(s);
		set.next();
	}

	_isValid = ret;
	return ret; 
}

void CVMissionObject::init() {
	CVCamera* cam = new CVCamera(this);
	cam->isPlanning(false);
	cam->uri(uri());
	cam->mission(id());
	insert(cam);

	CVRinex* rinex = new CVRinex(this);
	rinex->uri(uri());
	rinex->mission(id());
	insert(rinex);

	CVStations* stations = new CVStations(this);
	stations->mission(id());
	insert(stations);
}
	
}
}