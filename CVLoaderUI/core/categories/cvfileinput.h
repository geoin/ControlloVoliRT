#ifndef CV_FILE_INPUT_H
#define CV_FILE_INPUT_H

#include "core/categories/cvcontrol.h"

#include <QStringList>

namespace CV {
namespace Core {

class CVFileInput : public CVObject {
	Q_OBJECT

public:
	CVFileInput(QObject *parent);
	~CVFileInput();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();
	
	virtual bool remove();

	inline void origin(const QString& file) { _origin = file; }
	inline const QString& origin() const { return _origin; }

	inline void target(const QString& file) { _target = file; }
	inline const QString& target() const { return _target; }

	QStringList& data() { return _data; }

private:
	QString _origin, _target;
	QStringList _data;
};

} // namespace Core
} // namespace CV

#endif // CV_FILE_INPUT_H
