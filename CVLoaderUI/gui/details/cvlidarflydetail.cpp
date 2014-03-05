#include "cvlidarflydetail.h"

#include "cvsensordetail.h"
#include "cvflyaxis_p.h"
#include "cvdemdetail.h"
#include "cvareadetail.h"

namespace CV {
namespace GUI {
namespace Details {

CVLidarFlyDetail::CVLidarFlyDetail(QWidget* p, Core::CVControl* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	int i = 0;

	CVFlyAxis_p* axis = new CVFlyAxis_p(p, c->at(i));
    addTab(axis, "");
    setTabToolTip(i, tr("Assi di volo"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/axis.png"));

	i++;

	CVAreaDetail* area = new CVAreaDetail(p, c->at(i), c->type());
    addTab(area, "");
    setTabToolTip(i, tr("Aree da cartografare"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));
    
	i++;

	CVDemDetail* dem = new CVDemDetail(p, c->at(i), c->type());
	addTab(dem, "");
    setTabToolTip(i, tr("DEM"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/dem.png"));
}

CVLidarFlyDetail::~CVLidarFlyDetail() {

}

} // namespace Details
} // namespace GUI
} // namespace CV
