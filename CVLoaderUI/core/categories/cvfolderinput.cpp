#include "cvfolderinput.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <QUuid>

#include "core/cvcore_utils.h"

namespace CV {
namespace Core {

CVFolderInput::CVFolderInput(QObject* p) : CVObject(p) {
	_isValid = false;
}

CVFolderInput::~CVFolderInput() {

}

bool CVFolderInput::isValid() const {
	return _isValid;
}

bool CVFolderInput::remove() {
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
	/*ret = q->update(
		"MISSION", 
		QStringList() << "RINEX=?1" <<  "RINEX_NAME= ?2",
		QStringList() << "ID=?3",
		QVariantList() << QByteArray("0") << QString("") << mission()
	);*/
	return ret;
}

bool CVFolderInput::persist() {
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
	bool ret = false;/*q->update(
		"MISSION", 
		QStringList() << "RINEX=?1" << "RINEX_NAME=?2",
		QStringList() << "ID=?3",
		QVariantList() << file.readAll() << info.baseName() << mission()
	);*/
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

bool CVFolderInput::load() {
	_isValid = false;
	
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(QString(uri()).toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	/*CV::Util::Spatialite::Recordset set = q->select(
		QStringList() << "RINEX_NAME",
		QStringList() << "MISSION", 
		QStringList() << "ID=?1",
		QVariantList() << mission()
	);
	if (!set.eof()) {
		_rin = QString(set[0].toString().c_str());
	} 

	_isValid = !_rin.isEmpty();*/

	return _isValid;
}

} // namespace Core
} // namespace CV
