#ifndef CV_FILE_INPUT_H
#define CV_FILE_INPUT_H

#include "core/categories/cvcategory.h"

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

private:
};

} // namespace Core
} // namespace CV

#endif // CV_FILE_INPUT_H
