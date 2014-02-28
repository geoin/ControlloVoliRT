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
	
	inline bool isPlanning() const { return _sensor.planning; }
	inline void isPlanning(bool b) { _sensor.planning = b; }

	struct SensorData {
		SensorData() : fov(0.0f), ifov(0.0f), freq(0.0f), scan_rate(0.0f), planning(false) {}

		double fov, ifov, freq, scan_rate;
		bool planning;
		QString id;
	};
	
	SensorData& data() { return _sensor; }

private:
	SensorData _sensor;
};

} // namespace Core
} // namespace CV

#endif // CVSENSOR_H