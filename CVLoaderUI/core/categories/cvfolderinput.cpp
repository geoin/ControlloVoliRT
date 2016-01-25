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

	_folder = "FOLDER";
}

CVFolderInput::~CVFolderInput() {

}

bool CVFolderInput::isValid() const {
	return _isValid;
}

bool CVFolderInput::remove() {
	_isValid = false;
	bool ret = false;

	_data.clear();
	
	try {
		CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		ret = q->remove(
			table(), 
			QStringList(),
			QVariantList()
		);
		return ret;
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}
}

bool CVFolderInput::persist() {
	if (!remove()) {
		return false;
	}

	try {
		QFileInfo info(origin());

		CV::Util::Spatialite::Connection& cnn = SQL::Database::get();

		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		bool ret = q->insert(
			table(), 
			QStringList() << "ID" << folder(),
			QStringList() << "?1" << "?2" ,
			QVariantList() << QUuid::createUuid().toString() << info.absoluteFilePath()
		);

		if (!ret) {
			return false;
		} else {
			/*Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
			e->control = controlType();  
			e->object = type();
			e->uri = origin();
			//e->db = uri();
			Core::CVJournal::add(e);*/

			log(info.absoluteFilePath(), "");
		}

	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	return load();
}

bool CVFolderInput::load() {
	_isValid = false;
	_data.clear();
	
	try {
		CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << folder(),
			QStringList() << table(), 
			QStringList(),
			QVariantList()
		);
		if (!set.eof()) {
			_data << QString(set[0].toString().c_str());
		} 

		_isValid = _data.size() == 1;

		return _isValid;

	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}
}

} // namespace Core
} // namespace CV
