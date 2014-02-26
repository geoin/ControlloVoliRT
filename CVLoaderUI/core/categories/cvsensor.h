#ifndef CVSENSOR_H
#define CVSENSOR_H

#include "core/categories/cvcontrol.h"

namespace CV {
namespace Core {

class CVSensor : public CVObject {
	Q_OBJECT

public:
	CVSensor(QObject *parent);
	~CVSensor();
	
	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();

	virtual bool remove();

private:
	
};

} // namespace Core
} // namespace CV

#endif // CVSENSOR_H
