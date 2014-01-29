#ifndef CV_INPUT_STATIONS_H
#define CV_INPUT_STATIONS_H

#include "core/categories/cvcategory.h"
#include "core/categories/cvrinex.h"

#include <QStringList>
#include <QSharedPointer>

namespace CV {
namespace Core {

class CVStation : public CVObject{
	Q_OBJECT

public:
	typedef QSharedPointer<Core::CVStation> Ptr;

	CVStation(QObject *parent);
	CVStation(QObject* parent, const QString& id);
	~CVStation();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();
	virtual bool remove() { return true; }

	inline void id(const QString& id) { _id = id; }
	inline const QString& id() const { return _id; }

	inline void origin(const QString& file) { _origin = file; }
	inline const QString& origin() const { return _origin; }
	
	inline void name(const QString& n) { _name = n; }
	inline const QString& name() const { return _name; }

	inline const QString& mission() const { return _mission; }
	inline void mission(const QString& m) { _mission = m; }

	inline const QString& path() const { return _path; }
	inline void path(const QString& m) { _path = m; }

	void list(QStringList& list);

private:
	QString _id, _origin, _mission, _name, _path;
	bool _toUpdate;
};

class CVStations : public CVObject {
	Q_OBJECT

public:
	CVStations(QObject* p) : CVObject(p) {}
	~CVStations() {}
	
	virtual bool isValid() const { return true; }
	virtual bool persist() { return true; }
	virtual bool load() { return true; }
	virtual bool remove() { return true; }

	inline const QString& mission() const { return _mission; }
	inline void mission(const QString& m) { _mission = m; }

	inline void add(CVStation* r) { _ps.append(CVStation::Ptr(r)); }
	inline void add(CVStation::Ptr r) { _ps.append(r); }

	inline int count() const { return _ps.length(); }
	inline Core::CVStation* at(int i) const { return _ps.at(i).data(); }

	QString getZipFromStation(const QString& name, const QString& outPath, QString& id); //change sign

private:
	QString _mission;
	QList<CVStation::Ptr> _ps;
};

} // namespace Core
} // namespace CV

#endif // CV_FILE_INPUT_H
