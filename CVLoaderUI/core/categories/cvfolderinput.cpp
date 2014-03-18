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
	ret = q->remove(
		table(), 
		QStringList(),
		QVariantList()
	);
	return ret;
}

bool CVFolderInput::persist() {
	QFileInfo info(origin());

	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(QString(uri()).toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	bool ret = q->insert(
		table(), 
		QStringList() << "ID" << "FOLDER",
		QStringList() << "?1" << "?2" ,
		QVariantList() << QUuid::createUuid().toString() << info.absoluteFilePath()
	);

	if (!ret) {
		return false;
	} else {
		Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
		e->control = controlType();  
		e->object = type();
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
	CV::Util::Spatialite::Recordset set = q->select(
		QStringList() << "FOLDER",
		QStringList() << table(), 
		QStringList(),
		QVariantList()
	);
	if (!set.eof()) {
		_data << QString(set[0].toString().c_str());
	} 

	_isValid = _data.size() == 1;

	return _isValid;
}

} // namespace Core
} // namespace CV
