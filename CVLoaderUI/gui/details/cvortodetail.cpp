#include "cvortodetail.h"


#include <QLabel>

namespace CV {
namespace GUI {
namespace Details {

CVOrtoDetail::CVOrtoDetail(QWidget* p, Core::CVCategory* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	//TODO
	/*CVFlyAxis_p* axis = new CVFlyAxis_p(p, c->at(0));
    addTab(axis, "");
    setTabToolTip(0, tr("Assi di volo"));

	CVAreaDetail* area = new CVAreaDetail(p, c->at(1));
    addTab(area, "");
    setTabToolTip(1, tr("Aree da cartografare"));
    
	*/

	_category = c;
}

} // namespace Details
} // namespace GUI
} // namespace CV
