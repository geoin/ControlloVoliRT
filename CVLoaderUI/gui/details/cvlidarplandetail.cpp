#include "cvlidarplandetail.h"

#include "cvsensordetail.h"
#include "cvflyaxis_p.h"
#include "cvdemdetail.h"
#include "cvareadetail.h"

namespace CV {
namespace GUI {
namespace Details {

CVLidarPlanDetail::CVLidarPlanDetail(QWidget* p, Core::CVControl* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	Core::CVObject* s = c->at(0);
	CVSensorDetail* container = new CVSensorDetail(p, s);
    addTab(container, "");
    setTabToolTip(0, tr("Sensore lidar"));
	setTabIcon(0, QIcon(":/graphics/icons/plan/camera.png"));

	CVFlyAxis_p* axis = new CVFlyAxis_p(p, c->at(1));
    addTab(axis, "");
    setTabToolTip(1, tr("Assi di volo"));
	setTabIcon(1, QIcon(":/graphics/icons/plan/axis.png"));

	CVAreaDetail* area = new CVAreaDetail(p, c->at(2), c->type());
    addTab(area, "");
    setTabToolTip(2, tr("Aree da cartografare"));
	setTabIcon(2, QIcon(":/graphics/icons/plan/areas.png"));
    
	CVDemDetail* dem = new CVDemDetail(p, c->at(3), c->type());
	addTab(dem, "");
    setTabToolTip(3, tr("DEM"));
	setTabIcon(3, QIcon(":/graphics/icons/plan/dem.png"));
}

CVLidarPlanDetail::~CVLidarPlanDetail() {

}

} // namespace Details
} // namespace GUI
} // namespace CV
