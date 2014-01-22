#include "cvfileinput.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

namespace CV {
namespace Core {

CVFileInput::CVFileInput(QObject* p) : CVObject(p) {

}

CVFileInput::~CVFileInput() {

}

bool CVFileInput::isValid() const {
	return _isValid;
}

bool CVFileInput::persist() {
	return true;
}

bool CVFileInput::load() {
	return true;
}

} // namespace Core
} // namespace CV
