#include "plandetail.h"
#include "cvcameradetail.h"

#include <QLabel>

namespace CV {
namespace GUI {
namespace Details {

CVPlanDetail::CVPlanDetail(QWidget* p, TabPosition pos) : CVBaseTabWidget(p, pos) {
    CVCameraDetail* container = new CVCameraDetail(p);
    addTab(container, "");
    setTabToolTip(0, "Fotocamera");
    addTab(new QLabel(p), "");
    setTabToolTip(1, "Assi di volo");
    addTab(new QLabel(p), "");
    setTabToolTip(2, "DEM");
    addTab(new QLabel(p), "");
    setTabToolTip(3, "Aree da cartografare");
}

} // namespace Details
} // namespace GUI
} // namespace CV
