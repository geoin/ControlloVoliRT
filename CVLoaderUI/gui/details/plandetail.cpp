#include "plandetail.h"
#include "cvcameradetail.h"
#include "cvflyaxis_p.h"
#include "cvdemdetail.h"
#include "cvareadetail.h"

#include <QLabel>

namespace CV {
namespace GUI {
namespace Details {

CVPlanDetail::CVPlanDetail(QWidget* p, Core::CVControl* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	//TODO: remove indexes
	Core::CVCamera* cam = static_cast<Core::CVCamera*>(c->at(0));
	cam->isPlanning(true);
	CVCameraDetail* container = new CVCameraDetail(p, cam);
    addTab(container, "");
    setTabToolTip(0, tr("Fotocamera"));
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

	_control = c;
}

} // namespace Details
} // namespace GUI
} // namespace CV
