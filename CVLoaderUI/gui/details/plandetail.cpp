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

	CVFlyAxis_p* axis = new CVFlyAxis_p(p, c->at(1));
    addTab(axis, "");
    setTabToolTip(1, tr("Assi di volo"));

	CVAreaDetail* area = new CVAreaDetail(p, c->at(2), c->type());
    addTab(area, "");
    setTabToolTip(2, tr("Aree da cartografare"));
    
	CVDemDetail* dem = new CVDemDetail(p, c->at(3), c->type());
	addTab(dem, "");
    setTabToolTip(3, tr("DEM"));

	_control = c;
}

} // namespace Details
} // namespace GUI
} // namespace CV
