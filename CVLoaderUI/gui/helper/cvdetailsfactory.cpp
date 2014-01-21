#include "cvdetailsfactory.h"

#include "gui/details/missionlistdetails.h"
#include "gui/details/plandetail.h"

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
            detail = new Details::CVMissionListDetails(p);
            break;
        case CVNodeInfo::FLY:
           detail = new QWidget(p);
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
