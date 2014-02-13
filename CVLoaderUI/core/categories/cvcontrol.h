#ifndef CV_CORE_CATEGORIES_CONTROL_H
#define CV_CORE_CATEGORIES_CONTROL_H

#include <QList>
#include <QObject>
#include <QUuid>

#include "core/cvjournal.h"

namespace CV {
namespace Core {

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
		CONTOUR
	};

	explicit CVObject(QObject* p) : QObject(p) {}
	virtual ~CVObject() {}

	inline void type(Type t) { _type = t; }
	inline Type type() const { return _type; }

	inline void uri(const QString& t) { _uri = t; }
	inline const QString& uri() const { return _uri; }

	virtual bool isValid() const = 0;
	virtual bool persist() = 0;
	virtual bool load() = 0;
	virtual bool remove() = 0;
	
	virtual void init() {}

protected:
	bool _isValid;

private:
	QString _uri;
	Type _type;
};

class CVControl : public QObject {
    Q_OBJECT
public:
	enum Type { UNKNOWN_CATEGORY = 0, PLAN = 3, GPS_DATA, FLY, ORTO };

	explicit CVControl(Type t, QObject* p) : QObject(p), _type(t) {}

	inline Type type() const { return _type; }

	inline int count() const { return _objects.length(); }

	inline CVObject* at(int i) const { return _objects.at(i); }

	inline void remove(int i) { _objects.removeAt(i); }

	inline bool isComplete() const { 
		foreach(CVObject* obj, _objects) {
			if (!obj->isValid()) {
				return false;
			}
		}
		return true;
	};

	inline void load() { 
		foreach(CVObject* obj, _objects) {
			obj->load();
		}
	};

	inline void insert(CVObject* obj, bool setPath = true) { 
		if (setPath) {
			obj->uri(uri());
			obj->init();
		}
		_objects.append(obj); 
	}

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

}
}

#endif