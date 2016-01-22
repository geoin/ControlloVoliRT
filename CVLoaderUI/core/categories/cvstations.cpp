#include "cvstations.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QUuid>
#include <QRegExp>

#include <sstream>

#include "core/cvcore_utils.h"

namespace CV {
namespace Core {

CVStation::CVStation(QObject* p) : CVObject(p) {
	_isValid = true;
	_id = QUuid::createUuid().toString();
	_toUpdate = false;
}

CVStation::CVStation(QObject* p, const QString& id) : CVObject(p) {
	_isValid = true;
	_id = id;
	_toUpdate = true;
}

CVStation::~CVStation() {

}

bool CVStation::isValid() const {
	return _isValid;
}

void CVStation::list(QStringList& list) {
	CV::Util::Spatialite::Connection cnn;
	try {
		QString db(uri());
		cnn.open(db.toStdString());
	
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "RINEX",
			QStringList() << "STATION", 
			QStringList() << "ID = ?1",
			QVariantList() << id()
		);
		if (!set.eof()) {
			const std::vector<unsigned char>& blob = set[0].toBlob();
			const char* ptr = reinterpret_cast<const char*>(&blob[0]);
			
			std::stringstream str;
			str.write(ptr, blob.size());
			Poco::Zip::ZipArchive arch(str);
			Poco::Zip::ZipArchive::FileInfos::const_iterator it = arch.fileInfoBegin();
			for (; it != arch.fileInfoEnd(); it++) {
				list.append(it->first.c_str());
			}
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return;
	}
}

bool CVStation::persist() {
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
	bool ret = false;
	if (!_toUpdate) {
		ret = q->insert(
			"STATION", 
			QStringList() << "ID" << "ID_MISSION" << "RINEX" << "NAME",
			QStringList() << "?1" << "?2" << "?3" << "?4",
			QVariantList() << id() << mission() << file.readAll() << info.baseName()
		);
		_toUpdate = ret;
	} else {
		ret = q->update(
			"STATION",
			QStringList() << "RINEX=?1",
			QStringList() << "ID=?2",
			QVariantList() << file.readAll() << id()
		);
	}
	if (ret) {
		/*Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
		e->control = Core::CVControl::FLY;  
		e->object = Core::CVObject::STATION;
		e->uri = origin();
		//e->db = uri();
		Core::CVJournal::add(e);*/
	}
	return ret; //EMPTY
}

bool CVStation::load() {
	//TODO
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(QString(uri()).toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}
	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	/*CV::Util::Spatialite::Recordset set = q->select(
		QStringList() << "NAME",
		QStringList() << "STATION", 
		QStringList() << "ID = ?1",
		QVariantList() << id()
	);*/

	return true; //EMPTY
}

bool CVStations::remove() {
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	bool ret = q->remove(
		"STATION", 
		QStringList() << "ID_MISSION=?1",
		QVariantList() << mission()
	);

	_ps.clear();
	return ret;
}

void CVStations::removeAt(int i) { 
	QString id = _ps.at(i)->id();

	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
    q->remove(
		"STATION", 
		QStringList() << "ID=?1",
		QVariantList() << id
	);

	_ps.removeAt(i); 
}

QString CVStations::getZipFromStation(const QString& name, const QString& outPath, QString& id) {
	QString out;
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(QString(uri()).toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return out;
	}
	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	CV::Util::Spatialite::Recordset set = q->select(
		QStringList() << "ID" << "RINEX",
		QStringList() << "STATION", 
		QStringList() << "NAME = ?1" << "ID_MISSION = ?2",
		QVariantList() << name << mission()
	);

	if (!set.eof()) {
		id = QString(set[0].toString().c_str());
		out = outPath + QDir::separator() + name + ".zip";
		const std::vector<unsigned char>& blob = set[1].toBlob();
		const char* ptr = reinterpret_cast<const char*>(&blob[0]);
		QByteArray ba(ptr, blob.size());
		QFile zip(out);
		zip.open(QIODevice::WriteOnly);
		zip.write(ba);
		zip.close();
	}
	return out;
}

} // namespace Core
} // namespace CV
