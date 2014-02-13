#include "cvdetailsfactory.h"

#include "gui/details/missionlistdetails.h"
#include "gui/details/plandetail.h"
#include "gui/details/cvflydetail.h"
#include "gui/details/cvortodetail.h"

#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {
namespace Helper {

using namespace Status;

QWidget* CVDetailsFactory::build(QWidget* p, Core::CVControl::Type t, CV::Core::CVControl* control) {
    QWidget* detail = NULL;
    switch (t) {
		case Core::CVControl::PLAN:
            detail = new Details::CVPlanDetail(p, control);
            break;
        case Core::CVControl::GPS_DATA:
            detail = new Details::CVMissionListDetails(p, control);
            break;
        case Core::CVControl::FLY:
			detail = new Details::CVFlyDetail(p, control);
           break;
        case Core::CVControl::ORTO:
			detail = new Details::CVOrtoDetail(p, control);
           break;
        default:
            break;
    }
    return detail;
}

} // namespace Helper
} // namespace GUI
} // namespace CV
