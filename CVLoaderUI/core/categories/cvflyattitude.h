#ifndef CV_FLY_ATTITUDE_H
#define CV_FLY_ATTITUDE_H

#include "core/categories/cvcontrol.h"

#include <QStringList>

namespace CV {
namespace Core {

class CVFlyAttitude : public CVObject {
	Q_OBJECT

public:
	CVFlyAttitude(QObject *parent);
	~CVFlyAttitude();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();
	
	virtual bool remove();

	inline void origin(const QString& file) { _origin = file; }
	inline const QString& origin() const { return _origin; }

	QStringList& data() { return _data; }

signals:
	void persisted();
	void itemInserted(int);

private:
	QString _origin;
	QStringList _data;

	int _count;
};

} // namespace Core
} // namespace CV

#endif // CV_FLY_ATTITUDE_H
