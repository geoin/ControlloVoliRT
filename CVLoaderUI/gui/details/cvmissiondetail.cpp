#include "cvmissiondetail.h"
#include "cvcameradetail.h"
#include "cvsensordetail.h"
#include "cvstationsdetail.h"
#include "cvrinexdetail.h"
#include "cvflyaxis_p.h"

namespace CV {
namespace GUI {
namespace Details {

CVMissionDetail::CVMissionDetail(QWidget* p, Core::CVMissionObject* mission, TabPosition pos) : CVBaseTabWidget(p, pos) {
	assert(mission != NULL);
	_mission = mission;

	Core::CVControl::Type t = _mission->missionType();
	if (t == Core::CVControl::GPS_DATA) {
		addTab(new CVCameraDetail(p, _mission->at(0)), "");
		setTabToolTip(0, tr("Fotocamera"));
		setTabIcon(0, QIcon(":/graphics/icons/plan/camera.png"));
	} else {
		CVSensorDetail* container = new CVSensorDetail(p, _mission->at(0));
		addTab(container, "");
		setTabToolTip(0, tr("Sensore lidar"));
		setTabIcon(0, QIcon(":/graphics/icons/plan/camera.png"));
	}

    addTab(new CVRinexDetail(this, _mission->at(1)), "");
    setTabToolTip(1, tr("Rinex"));
	setTabIcon(1, QIcon(":/graphics/icons/gps/fly_rinex.png"));

    addTab(new CVStationsDetail(this, _mission->at(2)), "");
    setTabToolTip(2, tr("Stazioni permanenti"));
	setTabIcon(2, QIcon(":/graphics/icons/gps/station.png"));
}

} // namespace Details
} // namespace GUI
} // namespace CV
