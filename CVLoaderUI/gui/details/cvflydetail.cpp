#include "cvflydetail.h"
#include "cvflyaxis_p.h"
#include "cvdemdetail.h"
#include "cvareadetail.h"
#include "cvflyattitudedetail.h"

#include <QLabel>

namespace CV {
namespace GUI {
namespace Details {

CVFlyDetail::CVFlyDetail(QWidget* p, Core::CVCategory* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	//TODO
	CVFlyAxis_p* axis = new CVFlyAxis_p(p, c->at(0));
    addTab(axis, "");
    setTabToolTip(0, tr("Assi di volo"));

	CVAreaDetail* area = new CVAreaDetail(p, c->at(1));
    addTab(area, "");
    setTabToolTip(1, tr("Aree da cartografare"));
    
	CVDemDetail* dem = new CVDemDetail(p, c->at(2));
	addTab(dem, "");
    setTabToolTip(2, tr("DEM"));

	CVFlyAttitudeDetail* fs = new CVFlyAttitudeDetail(p, static_cast<Core::CVFlyAttitude*>(c->at(3)));
	addTab(fs, "");
    setTabToolTip(3, tr("File degli assetti"));

	_category = c;
}

} // namespace Details
} // namespace GUI
} // namespace CV
