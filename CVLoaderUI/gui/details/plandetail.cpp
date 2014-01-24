#include "plandetail.h"
#include "cvcameradetail.h"
#include "cvflyaxis_p.h"
#include "cvdemdetail.h"
#include "cvareadetail.h"

#include <QLabel>

namespace CV {
namespace GUI {
namespace Details {

CVPlanDetail::CVPlanDetail(QWidget* p, Core::CVCategory* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	Core::CVCamera* cam = static_cast<Core::CVCamera*>(c->at(0));
	cam->isPlanning(true);
	CVCameraDetail* container = new CVCameraDetail(p, cam);
    addTab(container, "");
    setTabToolTip(0, "Fotocamera");

	CVFlyAxis_p* axis = new CVFlyAxis_p(p, static_cast<Core::CVShapeLayer*>(c->at(1)));
    addTab(axis, "");
    setTabToolTip(1, "Assi di volo");

	CVAreaDetail* area = new CVAreaDetail(p, static_cast<Core::CVShapeLayer*>(c->at(2)));
    addTab(area, "");
    setTabToolTip(2, "Aree da cartografare");
    
	CVDemDetail* dem = new CVDemDetail(p, static_cast<Core::CVFileInput*>(c->at(3)));
	addTab(dem, "");
    setTabToolTip(3, "DEM");

	_category = c;
}

} // namespace Details
} // namespace GUI
} // namespace CV
