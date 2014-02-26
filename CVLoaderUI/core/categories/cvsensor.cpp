#include "cvsensor.h"

namespace CV {
namespace Core {

CVSensor::CVSensor(QObject *parent) : CVObject(parent) {

}

CVSensor::~CVSensor() {

}

bool CVSensor::isValid() const {
	return false;
}

bool CVSensor::persist() {
	return false;
}

bool CVSensor::load() {
	return false;
}

bool CVSensor::remove() {
	return false;
}

} // namespace Core
} // namespace CV
