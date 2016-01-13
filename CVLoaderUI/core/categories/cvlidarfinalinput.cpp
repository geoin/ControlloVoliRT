#include "cvlidarfinalinput.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <QUuid>

#include "core/cvcore_utils.h"

namespace CV {
namespace Core {

CVLidarFinalInput::CVLidarFinalInput(QObject* p) : CVObject(p), _tileSize(0) {
	_isValid = false;

	_tablesInfo["FINAL_RAW_STRIP_DATA"] << "FOLDER" << "";
	_tablesInfo["FINAL_GROUND_ELL"] << "FOLDER" << "";
	_tablesInfo["FINAL_GROUND_ORTO"] << "FOLDER" << "";
	_tablesInfo["FINAL_OVERGROUND_ELL"] << "FOLDER" << "";
	_tablesInfo["FINAL_OVERGROUND_ORTO"] << "FOLDER" << "";
	_tablesInfo["FINAL_MDS"] << "FOLDER" << "";
	_tablesInfo["FINAL_MDT"] << "FOLDER" << "";
	_tablesInfo["FINAL_INTENSITY"] << "FOLDER" << "";
	_tablesInfo["FINAL_IGM_GRID"] << "GRID" << "";

	_tileTable = "FINAL_RAW_STRIP_DATA";
	_tileColumn = "TILE_SIZE";
}

CVLidarFinalInput::~CVLidarFinalInput() {

}

void CVLidarFinalInput::set(const QString& table, const QString& column, const QString& value) {
	_tablesInfo[table].clear();
	_tablesInfo[table] << column << value;
}

void CVLidarFinalInput::set(const QString& table, const QString& column, const int& value) {
	_tileTable = table;
	_tileColumn = column;
	_tileSize = value;
}


bool CVLidarFinalInput::isValid() const {
	return _isValid;
}

bool CVLidarFinalInput::remove() {
	_isValid = false;
	bool ret = true;

	_data.clear();
	
	try {
		CV::Util::Spatialite::Connection& cnn = SQL::Database::get();

		QMap<QString, QStringList>::iterator it = _tablesInfo.begin();
		QMap<QString, QStringList>::iterator end = _tablesInfo.end();
		for (; it != end; it++) {
			Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
			ret |= q->remove(
				it.key(), 
				QStringList(),
				QVariantList()
			);
		}
		
		return ret;
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}
}

bool CVLidarFinalInput::persist() {
	if (!remove()) {
		return false;
	}

	try {
		bool ret = true;
		CV::Util::Spatialite::Connection& cnn = SQL::Database::get();

		QMap<QString, QStringList>::iterator it = _tablesInfo.begin();
		QMap<QString, QStringList>::iterator end = _tablesInfo.end();
		for (; it != end; it++) {
			QStringList val = it.value();
			if (!val.size()) {
				_data << "";
				continue;
			}
			
			QString column = val.at(0);
			QString path = val.at(1);
			QString table = it.key();

			Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
			ret |= q->insert(
				table, 
				QStringList() << "ID" << column,
				QStringList() << "?1" << "?2" ,
				QVariantList() << QUuid::createUuid().toString() << path
			);
		}
		
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		ret |= q->update(
			_tileTable, 
			QStringList() << _tileColumn + " = ?1",
			QStringList(),
			QVariantList() << _tileSize
		);

		if (!ret) {
			return false;
		} else {
			Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
			e->control = controlType();  
			e->object = type();
			e->uri = origin();
			//e->db = uri();
			Core::CVJournal::add(e);
		}

	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	return load();
}

bool CVLidarFinalInput::load() {
	_isValid = false;

	try {
		CV::Util::Spatialite::Connection& cnn = SQL::Database::get();

		QMap<QString, QStringList>::iterator it = _tablesInfo.begin();
		QMap<QString, QStringList>::iterator end = _tablesInfo.end();
		for (; it != end; it++) {
			QStringList val = it.value();
			QString table = it.key();

			_tables << table;
			if (!val.size()) {
				_data << "";
				continue;
			}
			
			QString column = val.at(0);
			QString path = val.at(1);

			Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
			CV::Util::Spatialite::Recordset set = q->select(
				QStringList() << column,
				QStringList() << table, 
				QStringList(),
				QVariantList()
			);
			if (!set.eof()) {
				QString val(set[0].toString().c_str());
				_data << val;

				_tablesInfo[table].clear();
				_tablesInfo[table] << column << val;
			}  else {
				_data << "";
			}
		}

		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << _tileColumn,
			QStringList() << _tileTable, 
			QStringList(),
			QVariantList()
		);
		if (!set.eof()) {
			_tileSize = set[0].toInt();
		}

		_isValid = true;
		return _isValid;

	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}
}

} // namespace Core
} // namespace CV
