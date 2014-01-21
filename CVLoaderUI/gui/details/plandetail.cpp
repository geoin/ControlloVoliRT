#include "plandetail.h"
#include "cvcameradetail.h"

#include <QLabel>

namespace CV {
namespace GUI {
namespace Details {

CVPlanDetail::CVPlanDetail(QWidget* p, Core::CVCategory* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	CVCameraDetail* container = new CVCameraDetail(p, static_cast<Core::CVCamera*>(c->at(0)));
    addTab(container, "");
    setTabToolTip(0, "Fotocamera");
    addTab(new QLabel(p), "");
    setTabToolTip(1, "Assi di volo");
    addTab(new QLabel(p), "");
    setTabToolTip(2, "DEM");
    addTab(new QLabel(p), "");
    setTabToolTip(3, "Aree da cartografare");

	_category = c;
}

} // namespace Details
} // namespace GUI
} // namespace CV
