#include "cvlidarrawdetail.h"

namespace CV {
namespace GUI {
namespace Details {

CVLidarRawDetail::CVLidarRawDetail(QWidget* p, Core::CVControl* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	int i = 0;

    addTab(new QWidget(this), "");
    setTabToolTip(i, tr("Punti di controllo"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/axis.png"));

	i++;

    addTab(new QWidget(this), "");
    setTabToolTip(i, tr("Datio grezzi"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));
}

CVLidarRawDetail::~CVLidarRawDetail() {

}

} // namespace Details
} // namespace GUI
} // namespace CV
