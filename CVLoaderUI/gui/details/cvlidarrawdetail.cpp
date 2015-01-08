#include "cvlidarrawdetail.h"

#include "cvrawstripfolderdetail.h"
#include "cvcontrolpointsdetail.h"

namespace CV {
namespace GUI {
namespace Details {

CVLidarRawDetail::CVLidarRawDetail(QWidget* p, Core::CVControl* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	int i = 0;

	addTab(new CVControlPointsDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Punti di controllo"));
	setTabIcon(i, QIcon(":/graphics/icons/lidar/points.png"));

	i++;

	addTab(new CVFolderDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Dati grezzi"));
	setTabIcon(i, QIcon(":/graphics/icons/lidar/strip.png"));
}

CVLidarRawDetail::~CVLidarRawDetail() {

}

} // namespace Details
} // namespace GUI
} // namespace CV
