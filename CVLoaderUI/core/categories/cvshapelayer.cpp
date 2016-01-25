#include "cvshapelayer.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

namespace CV {
namespace Core {

CVShapeLayer::CVShapeLayer(QObject* p) : CVObject(p), _shapeCharSet("CP1252"), _colName("GEOM") {
	_rows = 0;
	_isValid = false;
	
	_cols = fields();
}

CVShapeLayer::~CVShapeLayer() {

}

bool CVShapeLayer::isValid() const {
	return _isValid;
}

QStringList CVShapeLayer::fields() const {	
	QStringList f;
	try {
		CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		f = q->columns(_table);
	} catch(...) {}
	return f;
}

bool CVShapeLayer::remove() { 
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	try {
		cnn.remove_layer(_table.toStdString());
	} catch (const CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	} 
	return true; 
}

bool CVShapeLayer::persist() {
	remove();

	_isValid = false;
	try {
		CV::Util::Spatialite::Connection& cnn = SQL::Database::get();

		SQL::Query::Ptr q = SQL::QueryBuilder::build(cnn);
		Util::Spatialite::Recordset set = q->select(QStringList() << "DATUM", QStringList() << "PROJECT", QStringList(), QVariantList());

		_rows = cnn.load_shapefile(
			_shp.toStdString(),
			_table.toStdString(),
			_shapeCharSet.toStdString(),
			set[0].toInt(),
			_colName.toStdString(),
			true,
			false,
			false
		);
	} catch (const CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	} 

	if (_rows) {
		/*Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
		e->control = controlType();  
		e->object = type();
		e->uri = _shp;
		//e->db = uri();
		Core::CVJournal::add(e);*/
		log(_shp, "");
	}

	return load();
}

QStringList& CVShapeLayer::data() {
	return _info;
}

void CVShapeLayer::_initLoadProcess() {
	_info.clear();
	_isValid = false;

	//TODO: add count query

	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	try {
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "count(*)",
			QStringList() << _table, 
			QStringList(),
			QVariantList(),
			QStringList(),
			1
		);

		if (!set.eof()) {
			_rows = set[0].toInt();
			_isValid = true;
		} 

	} catch (const CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
	} 
}

bool CVShapeLayer::load() {
	_initLoadProcess();
	if (isValid() == false) {
		return false;
	}
	
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	try {
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		CV::Util::Spatialite::Recordset set = q->select(
			_cols,
			QStringList() << _table, 
			QStringList(),
			QVariantList(),
			QStringList(),
			1
		);

		if (!set.eof()) {
			int i = 0;
			while (i < set.fields_count()) {
				_info << QString(set[i].toString().c_str());	
				i++;
			}
		} 

	} catch (const CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	} 

	return true;
}

/* META COLS */

QMap<QString, QString> CVShapeLayerWithMeta::refColumns() const {
	QMap<QString, QString> map;
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	try {
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList(),
			QStringList() << "_META_COLUMNS_", 
			QStringList() << "CONTROL = ?1" << " OBJECT = ?2",
			QVariantList() << this->controlType() << this->type()
		);

		while (!set.eof()) {
			map.insert(QString(set[2].toString().c_str()), QString(set[3].toString().c_str()));

			set.next();
		}
	} catch (const std::exception&) {
		return QMap<QString, QString>();
	}
	return map;
}

bool CVShapeLayerWithMeta::edit(QString ref, QString target) {
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	try {
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		return q->update(
			"_META_COLUMNS_",
			QStringList() << "TARGET = ?1",
			QStringList() << "REF = ?2",
			QVariantList() << target << ref
		);
	} catch (const std::exception&) {
		return false;
	}
}

} // namespace Core
} // namespace CV
