#include "cvortodetail.h"

#include "cvuniondetail.h"
#include "cvcontourdetail.h"

#include <QLabel>

namespace CV {
namespace GUI {
namespace Details {

CVOrtoDetail::CVOrtoDetail(QWidget* p, Core::CVControl* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	//TODO
	CVUnionDetail* un = new CVUnionDetail(p, c->at(0));
    addTab(un, "");
    setTabToolTip(0, tr("Quadro di unione"));
	setTabIcon(0, QIcon(":/graphics/icons/orto/quadro.png"));

	CVContourDetail* cont = new CVContourDetail(p, c->at(1));
    addTab(cont, "");
    setTabToolTip(1, tr("Contorno regione"));
	setTabIcon(1, QIcon(":/graphics/icons/orto/contour.png"));

	_control = c;
}

} // namespace Details
} // namespace GUI
} // namespace CV
