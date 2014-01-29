#ifndef CV_CORE_CATEGORIES_MISSIONCATEGORY_H
#define CV_CORE_CATEGORIES_MISSIONCATEGORY_H

#include "cvcategory.h"

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

	virtual bool persist();
	virtual bool isValid() const;
	virtual bool load();
	
	virtual bool remove() { return true; }

	virtual void init();

	inline QString id() const { return _id; }

	inline QString name() const { return _name; }
	inline void name(const QString& n) { _name = n; }
	
	inline void insert(CVObject* obj, bool setPath = true) { 
		if (setPath) {
			obj->uri(uri());
			_objects.append(obj); 
		}
	}
	
	inline CVObject* at(int i) const { return _objects.at(i); }

private:
	bool _isValid;

	QList<CVObject*> _objects;
	QString _id;
	QString _name;
	QString _cam;
};

}
}

#endif