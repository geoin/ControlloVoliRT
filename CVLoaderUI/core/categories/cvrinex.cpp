#include "cvrinex.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QUuid>
#include <QRegExp>

#include "core/cvcore_utils.h"
#include <sstream>

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

bool CVRinex::remove() {
	_isValid = false;
	bool ret = false;

	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	ret = q->update(
		"MISSION", 
		QStringList() << "RINEX=?1" <<  "RINEX_NAME= ?2",
		QStringList() << "ID=?3",
		QVariantList() << QByteArray("0") << QString("") << mission()
	);
	return ret;
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
		QStringList() << "RINEX=?1" << "RINEX_NAME=?2",
		QStringList() << "ID=?3",
		QVariantList() << file.readAll() << info.baseName() << mission()
	);
	if (!ret) {
		return false;
	} else {
		Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
		e->control = Core::CVControl::FLY;  
		e->object = Core::CVObject::FLY_RINEX;
		e->uri = origin();
		e->db = uri();
		Core::CVJournal::add(e);
	}

	return load();
}

bool CVRinex::load() {
	_isValid = false;
	
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(QString(uri()).toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	CV::Util::Spatialite::Recordset set = q->select(
		QStringList() << "RINEX_NAME",
		QStringList() << "MISSION", 
		QStringList() << "ID=?1",
		QVariantList() << mission()
	);
	if (!set.eof()) {
		_rin = QString(set[0].toString().c_str());
	} 

	_isValid = !_rin.isEmpty();

	return _isValid;
}

void CVRinex::list(QStringList& list) {
	CV::Util::Spatialite::Connection cnn;
	try {
		QString db(uri());
		cnn.open(db.toStdString());
	
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "RINEX",
			QStringList() << "MISSION", 
			QStringList() << "ID = ?1",
			QVariantList() << mission()
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

} // namespace Core
} // namespace CV
