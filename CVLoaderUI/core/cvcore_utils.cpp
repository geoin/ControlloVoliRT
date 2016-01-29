#include "cvcore_utils.h"

using namespace CV::Core;

Csv::Csv(const QString& name) : _delim("\""), _sep(",") {
	_setFile(name);

}

Csv::Csv(const QString& name, QIODevice::OpenMode m) : _delim("\""), _sep(",") {
	_open(name, m);
}

void Csv::setSeparator(const QString& s) {
    _sep = s;
}

void Csv::setDelim(const QString& d) {
    _delim = d;
}

void Csv::_setFile(const QString& n) {
    _file.setFileName(n);
    _stream.setDevice(&_file);
}

bool Csv::open(QIODevice::OpenMode mode) {
	close();
    return _file.open(mode);
}

bool Csv::_open(const QString& n, QIODevice::OpenMode mode) {
    _file.setFileName(n);
    _stream.setDevice(&_file);

    return _file.open(mode);
}

void Csv::close() {
    _file.close();
}

QString Csv::readLine() {
    return _stream.readLine();
}

QString Csv::readAll(){
    return _stream.readAll();
}

void Csv::splitAndFormat(const QString& src, QStringList& out, const QString separator, const QString delim) {
	out.clear();

	QStringList list;
	if (separator.length() == 1) {
		list = src.split(separator, QString::SkipEmptyParts);
	} else if (separator.length() > 1) {
		QString r = "%1";
		QRegExp rg(r.arg(separator));
		list = src.split(rg, QString::SkipEmptyParts);
	} 

    foreach (QString col, list) {
        QString c = col.simplified();
		if (!delim.isNull() && c.startsWith(delim) && c.endsWith(delim)) {
            c = c.mid(1, c.size() - 2);
		}
        out << c;
    }
}

void Csv::splitAndFormat(const QString& src, QStringList& out) {
	if (_sep.length()) {
		splitAndFormat(src, out, _sep, _delim);
	}
}

bool Csv::seek(int i){
    return _stream.seek(i);
}

QDir Csv::dir() const {
    QFileInfo info(_file);
    return info.absoluteDir();
}

QString Csv::name() const {
    QFileInfo info(_file);
    return info.baseName();
}

QString Csv::path() const {
    QFileInfo info(_file);
	return info.absoluteFilePath();
}

QString Csv::suffix() const {
    QFileInfo info(_file);
    return info.completeSuffix();
}

QString Csv::ext() const {
    QFileInfo info(_file);
    return info.suffix();
}

void Csv::writeLine(const QString& str) {
    _stream << str << "\n";
    _stream.flush();
}