#ifndef CV_GUI_DETAILS_PLANDETAIL_H
#define CV_GUI_DETAILS_PLANDETAIL_H

#include "gui/cvbasetabwidget.h"
#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {
namespace Details {

class CVPlanDetail : public CVBaseTabWidget {
    Q_OBJECT
public:
    explicit CVPlanDetail(QWidget* = 0, Core::CVControl* = 0, TabPosition = East);

private:
	Core::CVControl* _control;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_PLANDETAIL_H
