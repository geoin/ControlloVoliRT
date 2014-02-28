#include "cvshapelayer.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

namespace CV {
namespace Core {

CVShapeLayer::CVShapeLayer(QObject* p) : CVObject(p), _shapeCharSet("CP1252"), _utm32_SRID(32632), _colName("GEOM") {
	_rows = 0;
}

CVShapeLayer::~CVShapeLayer() {

}

bool CVShapeLayer::isValid() const {
	return _isValid;
}

bool CVShapeLayer::remove() { 
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString());
		cnn.remove_layer(_table.toStdString());
	} catch (const CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	} 
	return true; 
}

bool CVShapeLayer::persist() {
	_isValid = false;
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString());

		_rows = cnn.load_shapefile(
			_shp.toStdString(),
			_table.toStdString(),
			_shapeCharSet.toStdString(),
			_utm32_SRID,
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
		Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
		e->control = controlType();  
		e->object = type();
		e->uri = _shp;
		e->db = uri();
		Core::CVJournal::add(e);
	}

	return load();
}

QStringList& CVShapeLayer::data() {
	return _info;
}

bool CVShapeLayer::load() {
	_info.clear();

	//TODO: add count query

	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString());
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
		return false;
	} 

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

} // namespace Core
} // namespace CV
