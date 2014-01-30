#include "cvflyattitude.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QUuid>
#include <QRegExp>
#include <QTextStream>

//TODO: this must be the base class, move all specialized logic in derived

namespace CV {
namespace Core {

CVFlyAttitude::CVFlyAttitude(QObject* p) : CVObject(p) {
	_isValid = false;
}

CVFlyAttitude::~CVFlyAttitude() {

}

bool CVFlyAttitude::isValid() const {
	return _isValid;
}

bool CVFlyAttitude::remove() { //TODO, should use id
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(QString(uri() + QDir::separator() + SQL::database).toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	bool ret = q->remove(
		"ASSETTI", 
		QStringList(),
		QVariantList()
	);

	return ret;
}

bool CVFlyAttitude::persist() {
	QFile file(_origin);
	bool open = file.open(QIODevice::ReadOnly | QIODevice::Text);
	if (!open) {
		return false;
	}
	remove();

	QTextStream str(&file);
	str.setCodec("UTF-8");

	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	QStringList rec;
	rec << "PHOTO_ID" << "PX" << "PY" << "PZ" << "OMEGA" << "PHI" << "KAPPA";
	
	QStringList ph;
	ph << "?1" << "?2" << "?3" << "?4" << "?5" << "?6" << "?7";

	while (str.status() != QTextStream::ReadPastEnd) { 
		QString line = str.readLine();
		if (line.isEmpty()) { 
			continue;
		}
		QStringList l = line.split(QRegExp("[\\t*\\s*]"), QString::SkipEmptyParts);
		if (l.size() != 7) {
			continue;
		}

		QVariantList data;
		QString photo_id = l.at(0);
		bool ok;
		qreal val;
		for (int i = 0; i < l.size(); ++i) {
			val = l.at(1).toDouble(&ok);
			if (!ok) { break; }
			data << val;
		}
		if (!ok) { continue; }

		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		q->insert(
			"ASSETTI",
			rec,
			ph,
			data
		);
		
	}

	return true;
}

bool CVFlyAttitude::load() {

	return true;
}

} // namespace Core
} // namespace CV
