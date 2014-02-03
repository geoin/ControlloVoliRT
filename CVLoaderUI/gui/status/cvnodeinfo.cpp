#include "cvnodeinfo.h"

namespace CV {
namespace GUI {
namespace Status {

CVNodeInfo::CVNodeInfo() {
    _isRoot = false;
	_type = Core::CVControl::UNKNOWN_CATEGORY;
    _status = UNKNOWN_STATUS;
}

}
} // namespace GUI
} // namespace CV
