#include "cvrinex.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QUuid>
#include <QRegExp>

//TODO: this must be the base class, move all specialized logic in derived

namespace CV {
namespace Core {

CVRinex::CVRinex(QObject* p) : CVObject(p) {
	_isValid = false;
}

CVRinex::~CVRinex() {

}

bool CVRinex::isValid() const {
	return _isValid;
}

bool CVRinex::persist() {
	QFile file(origin());
	bool open = file.open(QIODevice::ReadOnly);
	if (!open) {
        return false;
    }

	QFileInfo info(file);

	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(QString(uri()).toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	bool ret = q->update(
		"MISSION", 
		QStringList() << "RINEX=?1",
		QStringList() << "ID=?2",
		QVariantList() << file.readAll() << mission()
	);
	if (!ret) {
		return false;
	}

	return load();
}

bool CVRinex::load() {
	_isValid = false;
	
	/*if (_target.isEmpty()) {
		CV::Util::Spatialite::Connection cnn;
		try {
			cnn.open(QString(uri() + QDir::separator() + SQL::database).toStdString());
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			return false;
		}

		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "ID" << "URI",
			QStringList() << "DEM", 
			QStringList(),
			QVariantList(),
			QStringList(),
			1
		);
		if (!set.eof()) {
			target(set[1].toString().c_str());
		} else {
			return false;
		}
	}

	QFile file(_target);
    bool open = file.open(QIODevice::ReadOnly | QIODevice::Text);
	if (!open) {
		return false;
	}

	QTextStream str(&file);
	str.setCodec("UTF-8");

	int i = 0;
	int rows = 5; // TODO: handle this the right way
	while (i < rows) { 
		QString line = str.readLine();
		QStringList l = line.split(QRegExp("[\\t*\\s*]"), QString::SkipEmptyParts);
		if (l.size() < 2) {
			data() << "";
		} else {
			data() << l.at(1);
		}
		i++;
	}
	_isValid = data().size() == rows;*/
	return _isValid;
}

} // namespace Core
} // namespace CV
