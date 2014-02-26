#ifndef CV_CORE_COREUTILS
#define CV_CORE_COREUTILS

#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <QVariant>
#include <QUuid>

#include <Poco/Zip/Compress.h>
#include <Poco/Zip/Decompress.h>
#include <fstream>

namespace CV {
namespace Core {

//create a tmp dir, removing all files on destroy
class CVScopedTmpDir {
public:
	CVScopedTmpDir(const QString& base) {
		_base = base + QDir::separator() + QUuid::createUuid().toString();
		_d.mkpath(_base);
		bool cd = _d.cd(_base);
		if (!cd) {
			_base = QString();
		}
	}

	~CVScopedTmpDir() {
		//TODO, must be recursive
		if (_base.isEmpty()) {
			return;
		}

		_d.setNameFilters(QStringList() << "*.*");
		_d.setFilter(QDir::Files);
		foreach (const QString& dirFile, _d.entryList()) {
			_d.remove(dirFile);
		}
		_d.cdUp();
		
		_d.rmdir(_base);
	}

	inline const QString& toString() const { return _base; }
	inline QDir& dir() { return _d; }

private:
	QString _base;
	QDir _d;
};


//zip handling
class CVZip {
public:
	static bool zip(const std::vector<std::string> files, const std::string& outZip) { 
		std::ofstream out(outZip.c_str(), std::ios::binary); 
		Poco::Zip::Compress c(out, true);
		int i = 0;
		try {
			foreach (const std::string& file, files) {
				Poco::Path p(file);
				c.addFile(p, p.getFileName());
				i++;
			}
		} catch (const std::exception& e) { //TODO: handle permissions
			Q_UNUSED(e);
			QFile::remove(outZip.c_str());
			return false;
		}
		c.close();
		out.close();
		return files.size() == i;
	}

	static bool unzip(const std::string& origin, const std::string& folder) {
		try {
			std::ifstream in(origin.c_str(), std::ios::binary);
			Poco::Zip::Decompress dec(in, Poco::Path(folder)); 
			dec.decompressAllFiles();
			in.close();
			return true;
		} catch (const std::exception& e) {
			Q_UNUSED(e);
			return false;
		}
	}
};

class CVSettings {
public:
	static void set(const QString& k, const QVariant& value) {
		QSettings().setValue(k, value);
	}

	static QVariant get(const QString& k, const QVariant& v = QVariant()) {
		return QSettings().value(k, v);
	}

};

}
}

#endif