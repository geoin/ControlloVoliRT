#include "cvdetailsfactory.h"

#include "gui/details/missionlistdetails.h"
#include "gui/details/plandetail.h"
#include "gui/details/cvflydetail.h"
#include "gui/details/cvortodetail.h"

namespace CV {
namespace GUI {
namespace Helper {

using namespace Status;

QWidget* CVDetailsFactory::build(QWidget* p, CVNodeInfo::Type t, CV::Core::CVCategory* category) {
    QWidget* detail = NULL;
    switch (t) {
        case CVNodeInfo::FLY_PLAN:
            detail = new Details::CVPlanDetail(p, category);
            break;
        case CVNodeInfo::GPS_DATA:
            detail = new Details::CVMissionListDetails(p, category);
            break;
        case CVNodeInfo::FLY:
			detail = new Details::CVFlyDetail(p, category);
           break;
        case CVNodeInfo::ORTO:
			detail = new Details::CVOrtoDetail(p, category);
           break;
        default:
            detail = new QWidget(p);
            break;
    }
    return detail;
}

} // namespace Helper
} // namespace GUI
} // namespace CV
