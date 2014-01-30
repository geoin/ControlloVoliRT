#include "cvflydetail.h"
#include "cvflyaxis_p.h"
#include "cvdemdetail.h"
#include "cvareadetail.h"

#include <QLabel>

namespace CV {
namespace GUI {
namespace Details {

CVFlyDetail::CVFlyDetail(QWidget* p, Core::CVCategory* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	//TODO
	CVFlyAxis_p* axis = new CVFlyAxis_p(p, static_cast<Core::CVShapeLayer*>(c->at(0)));
    addTab(axis, "");
    setTabToolTip(0, "Assi di volo");

	CVAreaDetail* area = new CVAreaDetail(p, static_cast<Core::CVShapeLayer*>(c->at(1)));
    addTab(area, "");
    setTabToolTip(1, "Aree da cartografare");
    
	CVDemDetail* dem = new CVDemDetail(p, static_cast<Core::CVFileInput*>(c->at(2)));
	addTab(dem, "");
    setTabToolTip(2, "DEM");

	_category = c;
}

} // namespace Details
} // namespace GUI
} // namespace CV
