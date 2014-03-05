#ifndef CV_CORE_CATEGORIES_CONTROL_H
#define CV_CORE_CATEGORIES_CONTROL_H

#include <QList>
#include <QObject>
#include <QUuid>

#include "core/cvjournal.h"

namespace CV {
namespace Core {

class CVObject;

class CVControl : public QObject {
    Q_OBJECT
public:
    enum Type { 
		UNKNOWN_CATEGORY = 0, 
		PLAN = 3, GPS_DATA, FLY, ORTO, 
		LIDAR_PLAN, LIDAR_GPS_DATA, LIDAR_FLY, LIDAR_RAW
	};

	explicit CVControl(Type t, QObject* p) : QObject(p), _type(t) {}
	virtual ~CVControl() {}

	inline Type type() const { return _type; }

	int count() const;
	CVObject* at(int i) const;
	void remove(int i);
	bool isComplete() const;
	void load();
	void insert(CVObject* obj, bool setPath = true);

	inline void uri(const QString& uri) { 
		_uri = uri;
	}

	inline const QString& uri() const { 
		return _uri;
	}

protected:
	QList<CVObject*> _objects;
	Type _type;

private:
	QString _uri;
};

class CVObject : public QObject {
    Q_OBJECT
public:
	enum Type { 
		UNKNOWN_OBJECT = 0, 
		CAMERA, 
		AVOLOP,
		CARTO,
		DEM,
		MISSION,
		FLY_RINEX,
		STATION,
		AVOLOV,
		ATTITUDE,
		QUADRO,
		CONTOUR,
		SENSOR,
		TEST_CLOUD,
		CLOUD_CONTROL_POINTS,
		LIDAR_RAW_STRIP_DATA
	};

	explicit CVObject(QObject* p) : QObject(p) {}
	virtual ~CVObject() {}

	inline void type(Type t) { _type = t; }
	inline Type type() const { return _type; }
	
	inline void controlType(CVControl::Type t) { _controlType = t; }
	inline CVControl::Type controlType() const { return _controlType; }

	inline void uri(const QString& t) { _uri = t; }
	inline const QString& uri() const { return _uri; }

	virtual bool isValid() const = 0;
	virtual bool persist() = 0;
	virtual bool load() = 0;
	virtual bool remove() = 0;
	
	virtual void init() {}

signals:
	void persisted();

protected:
	bool _isValid;

private:
	QString _uri;

	Type _type;
	CVControl::Type _controlType;
};

class CVMissionDevice : public CVObject {
	Q_OBJECT
public:
	explicit CVMissionDevice(QObject* p) : CVObject(p) {}

	virtual bool load(const QString& mId) = 0;

	inline bool isPlanning() const { return _isPlanning; }
	inline void isPlanning(bool b) { _isPlanning = b; }

	inline void mission(const QString& id) {
		_mission = id;
	}
protected:
	bool _isPlanning;
	QString _mission;
};

}
}

#endif
