#ifndef CV_CORE_COREUTILS
#define CV_CORE_COREUTILS

#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <QVariant>
#include <QUuid>

#include <QSharedPointer>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include <QFile>

#include <Poco/Zip/Compress.h>
#include <Poco/Zip/Decompress.h>
#include <fstream>

namespace CV {
namespace Core {
	
#define CV_PATH_PROJECT "/paths/project"
#define CV_PATH_SEARCH "/paths/search"

#define CV_CSV_HEADER "/csv/header"
#define CV_CSV_SEPARATOR "/csv/separator"

class CVSettings {
public:
	static void set(const QString& k, const QVariant& value) {
		QSettings().setValue(k, value);
	}

	static QVariant get(const QString& k, const QVariant& v = QVariant()) {
		return QSettings().value(k, v);
	}

};

static bool removeDir(const QString& dirName) {
	bool result = false;
	QDir dir(dirName); 
	if (dir.exists(dirName)) {
		Q_FOREACH (QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
			if (info.isDir()) {
				result = removeDir(info.absoluteFilePath());
			} else {
				result = QFile::remove(info.absoluteFilePath());
			}
			if (!result) {
				return result;
			}
		}
		result = dir.rmdir(dirName);
	}
	return result;
}

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
		if (!_base.isEmpty()) {
			removeDir(_base);
		}
	}

	inline const QString& toString() const { return _base; }

	inline QDir& dir() { return _d; }

private:
	QString _base;
	QDir _d;

    Q_DISABLE_COPY(CVScopedTmpDir)
};

//zip handling
class CVZip {
public:
	static bool zip(const std::vector<std::string> files, const std::string& outZip) { 
		std::ofstream out(outZip.c_str(), std::ios::binary); 
		Poco::Zip::Compress c(out, true);
        unsigned int i = 0;
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

class Csv {
public:
	typedef QSharedPointer<Csv> Ptr;

	Csv(const QString& n);
	Csv(const QString&, QIODevice::OpenMode);
    virtual ~Csv() { close(); }
	
    bool open(QIODevice::OpenMode = QIODevice::ReadOnly);
    void close();

    QDir dir() const;
    QString path() const;
    QString name() const;
    QString suffix() const;
    QString ext() const;

    inline const QString& separator() const { return _sep; }
    inline const QString& delim() const { return _delim; }

    void setSeparator(const QString&);
    void setDelim(const QString&);

    QString readLine();
    QString readAll();
    void writeLine(const QString& str);
    bool seek(int);

	inline bool atEnd() const {
		return _stream.atEnd();
	}

    void splitAndFormat(const QString& src, QStringList& out, const QString sep, const QString delim = QString());
    void splitAndFormat(const QString& src, QStringList& out);

private:
    void _setFile(const QString& name);
    bool _open(const QString& name, QIODevice::OpenMode);

    QFile _file;
    QTextStream _stream;

    QString _sep, _delim;

	Q_DISABLE_COPY(Csv);
};

}
}

#endif
