#include "cvlidarfinaldetail.h"

#include "cvrawstripfolderdetail.h"
#include "cvfinaldatadetail.h"
#include "cvareadetail.h"
#include "cvrawstripfolderdetail.h"

#include <QFormLayout>
#include <QLineEdit>

namespace CV {
namespace GUI {
namespace Details {

CVLidarFinalDetail::CVLidarFinalDetail(QWidget* p, Core::CVControl* c, TabPosition pos) : CVBaseTabWidget(p, pos) {
	int i = 0;
	
	CVFinalDataDetail* fold = new CVFinalDataDetail(this, c->at(i));
    fold->title("Elaborati finali");
	fold->description("Inserire cartelle dati finali lidar");

	addTab(fold, "");
    setTabToolTip(i, tr("Elaborati finali"));
	setTabIcon(i, QIcon(":/graphics/icons/lidar/final.png"));

	i++;

	CVAreaDetail* area = new CVAreaDetail(p, c->at(i), c->type());
    addTab(area, "");
    setTabToolTip(i, tr("Aree da cartografare"));
	setTabIcon(i, QIcon(":/graphics/icons/plan/areas.png"));
    
	i++;
	
	addTab(new CVFolderDetail(this, c->at(i)), "");
    setTabToolTip(i, tr("Strisciate"));
	setTabIcon(i, QIcon(":/graphics/icons/lidar/strip.png"));
}

CVLidarFinalDetail::~CVLidarFinalDetail() {

}

} // namespace Details
} // namespace GUI
} // namespace CV
