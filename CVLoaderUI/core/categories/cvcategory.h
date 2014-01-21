#ifndef CV_CORE_CATEGORIES_CATEGORY_H
#define CV_CORE_CATEGORIES_CATEGORY_H

#include <QList>
#include <QObject>

namespace CV {
namespace Core {

class CVObject : public QObject {
    Q_OBJECT
public:
	enum Type { UNKNOWN_OBJECT = 0, CAMERA, FLY_AXIS };

	explicit CVObject(QObject* p = 0) {}
	virtual ~CVObject() {}

	inline void uri(const QString& t) { _uri = t; }
	inline const QString& uri() const { return _uri; }

	virtual bool isValid() const = 0;
	virtual bool persist() = 0;
	virtual bool load() = 0;

private:
	QString _uri;
};

class CVCategory : public QObject {
    Q_OBJECT
public:
	enum Type { UNKNOWN_CATEGORY = 0, PLAN, GPS_DATA, FLY };

	explicit CVCategory(Type t, QObject* p = 0) : _type(t) {}

	Type type() const { return _type; }

	CVObject* at(int i) const { return _objects.at(i); }

	inline bool isComplete() const { 
		foreach(CVObject* obj, _objects) {
			if (!obj->isValid()) {
				return false;
			}
		}
		return true;
	};

	inline void insert(CVObject* obj) { _objects.append(obj); }

protected:
	QList<CVObject*> _objects;
	Type _type;
};

}
}

#endif