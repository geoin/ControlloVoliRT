#ifndef CVCAMERA_H
#define CVCAMERA_H

#include "core/categories/cvcontrol.h"
//#include "core/cvbaseinputverifier.h"

#include <photo_util/vdp.h>

namespace CV {
namespace Core {

class CVCamera : public CVMissionDevice {
	Q_OBJECT

public:
	CVCamera(QObject* p = 0);
	~CVCamera();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();

	virtual bool remove();

	virtual bool load(const QString& mId);

	inline Camera& data() { return _cam; }

	inline QString id() const { return QString(_cam.id.c_str()); }

private:
	Camera _cam;

	//Validator
};

} // namespace Core
} // namespace CV

#endif // CVCAMERA_H
