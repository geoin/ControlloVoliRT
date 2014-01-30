#ifndef CVSHAPELAYER_H
#define CVSHAPELAYER_H

#include "core/categories/cvcategory.h"

#include <QStringList>

namespace CV {
namespace Core {

class CVShapeLayer : public CVObject {
	Q_OBJECT

public:
	CVShapeLayer(QObject *parent);
	~CVShapeLayer();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();
	
	virtual bool remove();

	inline void shape(const QString& shp) { _shp = shp; }
	inline QString shape() { return _shp; }

	QStringList& data();

	void table(const QString& tab) {
		_table = tab;
	}

	void columns(const QStringList& cols) {
		_cols = cols;
	}

private:
	QStringList _info;
	QStringList _cols;
	QString _table;

	QString _shapeCharSet, _colName;
	short _utm32_SRID;
	QString _shp;

	int _rows;
};

} // namespace Core
} // namespace CV

#endif // CVSHAPELAYER_H
