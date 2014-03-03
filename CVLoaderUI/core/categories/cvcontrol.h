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
	enum Type { UNKNOWN_CATEGORY = 0, PLAN = 3, GPS_DATA, FLY, ORTO, LIDAR_PLAN };

	explicit CVControl(Type t, QObject* p) : QObject(p), _type(t) {}

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
		SENSOR
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

}
}

#endif