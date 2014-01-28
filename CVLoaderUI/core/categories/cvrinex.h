#ifndef CV_INPUT_RINEX_H
#define CV_INPUT_RINEX_H

#include "core/categories/cvcategory.h"

#include <QStringList>

namespace CV {
namespace Core {

class CVRinex : public CVObject {
	Q_OBJECT

public:
	CVRinex(QObject *parent);
	~CVRinex();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();

	inline void origin(const QString& file) { _origin = file; }
	inline const QString& origin() const { return _origin; }

	inline const QString& mission() const { return _mission; }
	inline void mission(const QString& m) { _mission = m; }

private:
	QString _origin, _mission;
};

} // namespace Core
} // namespace CV

#endif // CV_FILE_INPUT_H
