#ifndef CVSHAPELAYER_H
#define CVSHAPELAYER_H

#include "core/categories/cvcontrol.h"

#include <QStringList>
#include <QMap>

namespace CV {
namespace Core {

class CVShapeLayer : public CVObject {
	Q_OBJECT

public:
	CVShapeLayer(QObject *parent);
	virtual ~CVShapeLayer();

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

	QString table() const { Q_ASSERT(_table.isEmpty() == false); return _table; }


	void columns(const QStringList& cols) {
		_cols = cols;
	}

	int rows() const { return _rows; }

	QStringList fields() const;

protected:
	QStringList _cols;
	QString _table;

private:
	void _initLoadProcess();

	QStringList _info;

	QString _shapeCharSet, _colName;
	short _utm32_SRID;
	QString _shp;

	int _rows;
};

class CVShapeLayerWithMeta : public CVShapeLayer {
public:
	CVShapeLayerWithMeta(QObject* p) : CVShapeLayer(p) {}
	virtual ~CVShapeLayerWithMeta() {}

	QMap<QString, QString> refColumns() const;

	bool edit(QString ref, QString target);
};

} // namespace Core
} // namespace CV

#endif // CVSHAPELAYER_H
