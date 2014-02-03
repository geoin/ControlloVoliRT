#ifndef CV_GUI_DETAILS_CVORTODETAIL_H
#define CV_GUI_DETAILS_CVORTODETAIL_H

#include "gui/cvbasetabwidget.h"
#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {
namespace Details {

class CVOrtoDetail : public CVBaseTabWidget {
    Q_OBJECT
public:
    explicit CVOrtoDetail(QWidget* = 0, Core::CVControl* = 0, TabPosition = East);

private:
	Core::CVControl* _control;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVFLYDETAIL_H
