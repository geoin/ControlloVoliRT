#ifndef CV_GUI_DETAILS_CVFLYDETAIL_H
#define CV_GUI_DETAILS_CVFLYDETAIL_H

#include "gui/cvbasetabwidget.h"
#include "core/categories/cvcategory.h"

namespace CV {
namespace GUI {
namespace Details {

class CVFlyDetail : public CVBaseTabWidget {
    Q_OBJECT
public:
    explicit CVFlyDetail(QWidget* = 0, Core::CVCategory* = 0, TabPosition = East);

private:
	Core::CVCategory* _category;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVFLYDETAIL_H
