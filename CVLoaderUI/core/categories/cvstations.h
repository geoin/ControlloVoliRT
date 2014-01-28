#ifndef CV_INPUT_STATIONS_H
#define CV_INPUT_STATIONS_H

#include "core/categories/cvcategory.h"
#include "core/categories/cvrinex.h"

#include <QStringList>

#include <Poco/Zip/Compress.h>
#include <Poco/Zip/Decompress.h>
#include <fstream>

namespace CV {
namespace Core {

class CVZip {
public:
	static int zip(const std::vector<std::string> files, const std::string& outZip) {
		int tot = 0;
		std::ofstream out(outZip.c_str(), std::ios::binary);
		Poco::Zip::Compress c(out, true);
		foreach (const std::string& file, files) {
			Poco::Path p(file);
			c.addFile(p, p.getFileName());
		}
		c.close();
		return tot;
	}

	static int unzip(const std::string& origin, const std::string& folder) {
		int tot = 0;
		std::ifstream in(origin.c_str(), std::ios::binary);
		Poco::Zip::Decompress dec(in, Poco::Path(folder)); 
		dec.decompressAllFiles();
		return tot;
	}
};

class CVStation : public CVObject{
	Q_OBJECT

public:
	CVStation(QObject *parent);
	CVStation(QObject* parent, const QString& id);
	~CVStation();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();

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

	inline const QString& mission() const { return _mission; }
	inline void mission(const QString& m) { _mission = m; }

	inline void add(Core::CVStation* r) { _ps.append(r); }

	inline int count() const { return _ps.length(); }
	inline Core::CVStation* at(int i) const { return _ps.at(i); }

	QString getZipFromStation(const QString& name, const QString& outPath, QString& id); //change sign

private:
	QString _mission;
	QList<Core::CVStation*> _ps;
};

} // namespace Core
} // namespace CV

#endif // CV_FILE_INPUT_H
