#ifndef CV_LIDAR_FINAL_INPUT_H
#define CV_LIDAR_FINAL_INPUT_H

#include "core/categories/cvcontrol.h"

#include <QMap>
#include <QStringList>

namespace CV {
namespace Core {

class CVLidarFinalInput : public CVObject {
	Q_OBJECT

public:
	CVLidarFinalInput(QObject *parent);
	~CVLidarFinalInput();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();
	
	virtual bool remove();

	inline void origin(const QString& file) { _origin = file; }
	inline const QString& origin() const { return _origin; }

	QStringList& data() { return _data; }

	void setFolder(QString f) {
		_folder = f;
	} 

	const QString& folder() const { return _folder; } 

	void set(const QString& table, const QString& column, const QString& value);
	void set(const QString& table, const QString& column, const int& value);

	QStringList tables() const { return _tables; }

	int tileSize() const { return _tileSize; }

private:
	QString _origin;
	QStringList _data;

	QString _folder;

	QMap<QString, QStringList> _tablesInfo;
	QList<QString> _tables;
	
	QString _tileTable, _tileColumn;
	int _tileSize;
};

} // namespace Core
} // namespace CV

#endif // CV_LIDAR_FINAL_INPUT_H
