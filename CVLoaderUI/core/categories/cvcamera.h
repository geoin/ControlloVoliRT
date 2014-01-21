#ifndef CVCAMERA_H
#define CVCAMERA_H

#include "core/categories/cvcategory.h"
#include "core/cvbaseinputverifier.h"

#include <photo_util/vdp.h>

namespace CV {
namespace Core {

class CVCamera : public CVObject {
	Q_OBJECT

public:
	CVCamera(QObject* p = 0);
	~CVCamera();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();

	inline Camera& data() { return _cam; }

private:
	Camera _cam;
	bool _isValid;

	//Validator
};

} // namespace Core
} // namespace CV

#endif // CVCAMERA_H
