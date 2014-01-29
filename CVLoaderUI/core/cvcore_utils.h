#ifndef CV_CORE_COREUTILS
#define CV_CORE_COREUTILS

#include <QDir>
#include <QFileInfo>

#include <Poco/Zip/Compress.h>
#include <Poco/Zip/Decompress.h>
#include <fstream>

namespace CV {
namespace Core {

//create a tmp dir, removing all files on destroy
class CVScopedTmpDir {
public:
	CVScopedTmpDir(const QString& base) {
		_base = base + QDir::separator() + "tmp";
		_d.mkpath(_base);
		bool cd = _d.cd(_base);
		if (!cd) {
			_base = QString();
		}
	}

	~CVScopedTmpDir() {
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

}
}

#endif