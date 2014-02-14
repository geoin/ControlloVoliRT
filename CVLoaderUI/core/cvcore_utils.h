#ifndef CV_CORE_COREUTILS
#define CV_CORE_COREUTILS

#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <QVariant>

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
		try {
			foreach (const std::string& file, files) {
				Poco::Path p(file);
				c.addFile(p, p.getFileName());
			}
		} catch (const std::exception& e) {
			Q_UNUSED(e);
			//TODO remove file
		}
		c.close();
		out.close();
		return tot;
	}

	static int unzip(const std::string& origin, const std::string& folder) {
		int tot = 0;
		std::ifstream in(origin.c_str(), std::ios::binary);
		Poco::Zip::Decompress dec(in, Poco::Path(folder)); 
		dec.decompressAllFiles();
		in.close();
		return tot;
	}
};

class CVSettings {
public:
	static void set(const QString& k, const QVariant& value) {
		QSettings().setValue(k, value);
	}

	static QVariant get(const QString& k) {
		return QSettings().value(k, QVariant());
	}
};

}
}

#endif