#ifndef CV_GUI_HELPER_CVDETAILSFACTORY_H
#define CV_GUI_HELPER_CVDETAILSFACTORY_H

#include "gui/status/cvnodeinfo.h"
#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {
namespace Helper {

class CVDetailsFactory {
public:
    static QWidget* build(QWidget* parent, Core::CVControl::Type, CV::Core::CVControl*);
};

} // namespace Helper
} // namespace GUI
} // namespace CV

#endif // CV_GUI_HELPER_CVDETAILSFACTORY_H
