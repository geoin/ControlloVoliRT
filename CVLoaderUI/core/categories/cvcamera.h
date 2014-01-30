#ifndef CVCAMERA_H
#define CVCAMERA_H

#include "core/categories/cvcategory.h"
//#include "core/cvbaseinputverifier.h"

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

	virtual bool remove() { return true; }

	bool load(const QString& mId);

	inline Camera& data() { return _cam; }

	inline bool isPlanning() const { return _isPlanning; }
	inline void isPlanning(bool b) { _isPlanning = b; }

	inline void mission(const QString& id) {
		_mission = id;
	}

	inline QString id() const { return QString(_cam.id.c_str()); }

private:
	Camera _cam;
	bool _isPlanning;
	QString _mission;


	//Validator
};

} // namespace Core
} // namespace CV

#endif // CVCAMERA_H
