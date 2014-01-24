#include "cvmissiondetail.h"
#include "cvcameradetail.h"

namespace CV {
namespace GUI {
namespace Details {

CVMissionDetail::CVMissionDetail(QWidget* p, TabPosition pos) : CVBaseTabWidget(p, pos) {
    _key = QUuid::createUuid().toString();

    addTab(new CVCameraDetail(this), "");
    setTabToolTip(0, tr("Fotocamera"));

    addTab(new CVBaseDetail(this), "");
    setTabToolTip(1, tr("Rinex"));

    addTab(new CVBaseDetail(this), "");
    setTabToolTip(2, tr("Stazioni permanenti"));
}

} // namespace Details
} // namespace GUI
} // namespace CV
