#include "cvdetailsfactory.h"

#include "gui/details/missionlistdetails.h"
#include "gui/details/plandetail.h"
#include "gui/details/cvflydetail.h"
#include "gui/details/cvortodetail.h"
#include "gui/details/cvlidarplandetail.h"
#include "gui/details/cvlidarflydetail.h"
#include "gui/details/cvlidarrawdetail.h"

#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {
namespace Helper {

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
		case Core::CVControl::LIDAR_PLAN:
			detail = new Details::CVLidarPlanDetail(p, control);
           break;
        case Core::CVControl::LIDAR_GPS_DATA:
            detail = new Details::CVMissionListDetails(p, control);
           break;
        case Core::CVControl::LIDAR_FLY:
            detail = new Details::CVLidarFlyDetail(p, control);
           break;
        case Core::CVControl::LIDAR_RAW:
            detail = new Details::CVLidarRawDetail(p, control);
           break;
		case Core::CVControl::LIDAR_FINAL:
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
