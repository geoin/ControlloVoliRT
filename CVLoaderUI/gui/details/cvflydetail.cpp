#include "cvflydetail.h"
#include "cvflyaxis_p.h"
#include "cvdemdetail.h"
#include "cvareadetail.h"
#include "cvflyattitudedetail.h"

#include <QLabel>

namespace CV {
namespace GUI {
namespace Details {

CVFlyDetail::CVFlyDetail(QWidget* p, Core::CVControl* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	CVAreaDetail* area = new CVAreaDetail(p, c->at(0), c->type());
    addTab(area, "");
    setTabToolTip(0, tr("Aree da cartografare"));
	setTabIcon(0, QIcon(":/graphics/icons/plan/areas.png"));
    
	CVDemDetail* dem = new CVDemDetail(p, c->at(1), c->type());
	addTab(dem, "");
    setTabToolTip(1, tr("DEM"));
	setTabIcon(1, QIcon(":/graphics/icons/plan/dem.png"));

	CVFlyAttitudeDetail* fs = new CVFlyAttitudeDetail(p, static_cast<Core::CVFlyAttitude*>(c->at(2)));
	addTab(fs, "");
    setTabToolTip(2, tr("File degli assetti"));
	setTabIcon(2, QIcon(":/graphics/icons/fly/attitude.png"));
	
	/*CVFlyAxis_p* axis = new CVFlyAxis_p(p, c->at(0));
    addTab(axis, "");
    setTabToolTip(4, tr("Assi di volo"));*/

	_control = c;
}

} // namespace Details
} // namespace GUI
} // namespace CV
