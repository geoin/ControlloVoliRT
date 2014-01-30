#include "cvmissiondetail.h"
#include "cvcameradetail.h"
#include "cvstationsdetail.h"
#include "cvrinexdetail.h"
#include "cvflyaxis_p.h"

namespace CV {
namespace GUI {
namespace Details {

CVMissionDetail::CVMissionDetail(QWidget* p, Core::CVMissionObject* mission, TabPosition pos) : CVBaseTabWidget(p, pos) {
	assert(mission != NULL);
	_mission = mission;

	addTab(new CVCameraDetail(this, static_cast<Core::CVCamera*>(_mission->at(0))), "");
    setTabToolTip(0, tr("Fotocamera"));

    addTab(new CVRinexDetail(this, static_cast<Core::CVRinex*>(_mission->at(1))), "");
    setTabToolTip(1, tr("Rinex"));

    addTab(new CVStationsDetail(this, static_cast<Core::CVStations*>(_mission->at(2))), "");
    setTabToolTip(2, tr("Stazioni permanenti"));
	
	addTab(new CVFlyAxis_p(this, static_cast<Core::CVShapeLayer*>(_mission->at(3))), "");
    setTabToolTip(3, tr("Assi di volo"));
}

} // namespace Details
} // namespace GUI
} // namespace CV
