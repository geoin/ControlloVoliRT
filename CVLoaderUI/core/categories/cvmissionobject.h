#ifndef CV_CORE_CATEGORIES_MISSIONCATEGORY_H
#define CV_CORE_CATEGORIES_MISSIONCATEGORY_H

#include "cvcontrol.h"

#include <QList>
#include <QObject>
#include <QUuid>

namespace CV {
namespace Core {

class CVMissionObject : public CVObject {
	Q_OBJECT
public:
	explicit CVMissionObject(QObject* p);
	explicit CVMissionObject(QObject* p, QString key);
	~CVMissionObject() {
	
	}

	virtual bool persist();
	virtual bool isValid() const;
	virtual bool load();
	
	virtual bool remove();

	virtual void init();

	inline void missionType(CVControl::Type t) { 
		_missionType = t; 
		controlType(t);
		_deviceId = t == CVControl::GPS_DATA ? "ID_CAMERA" : "ID_SENSOR";
	}

    inline CVControl::Type missionType() const { return _missionType; }

	inline QString id() const { return _id; }

	inline QString name() const { return _name; }
	inline void name(const QString& n) { _name = n; }
	
	inline void insert(CVObject* obj, bool setPath = true) { 
		if (setPath) {
			obj->uri(uri());
		}
		_objects.append(obj); 
		obj->controlType(missionType());
	}
	
	inline CVObject* at(int i) const { return _objects.at(i); }

private:
	bool _isValid;

	QList<CVObject*> _objects;
	QString _id;
	QString _name;

	QString _deviceId;

	CVControl::Type _missionType;
};

}
}

#endif
