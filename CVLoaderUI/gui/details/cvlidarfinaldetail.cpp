#include "cvlidarfinaldetail.h"

#include "cvrawstripfolderdetail.h"

namespace CV {
namespace GUI {
namespace Details {

CVLidarFinalDetail::CVLidarFinalDetail(QWidget* p, Core::CVControl* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	int i = 0;
	
	addTab(new CVRawStripDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Dati grezzi finali"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));

	i++;

	addTab(new CVRawStripDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Dati intensità"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));

	i++;

	addTab(new CVRawStripDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Dati ground ellissoidici"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));

	i++;

	addTab(new CVRawStripDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Dati ground ortometrici"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));

	i++;

	addTab(new CVRawStripDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Dati overground ellissoidici"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));

	i++;

	addTab(new CVRawStripDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Dati overground ortometrici"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));

	i++;

	addTab(new CVRawStripDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Dati mds"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));

	i++;

	addTab(new CVRawStripDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Dati mdt"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));
}

CVLidarFinalDetail::~CVLidarFinalDetail() {

}

} // namespace Details
} // namespace GUI
} // namespace CV
