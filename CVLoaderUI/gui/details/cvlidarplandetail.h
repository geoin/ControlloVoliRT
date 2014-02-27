#ifndef CVLIDARPLANDETAIL_H
#define CVLIDARPLANDETAIL_H

#include "gui/cvbasetabwidget.h"
#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {
namespace Details {

class CVLidarPlanDetail : public CVBaseTabWidget {
	Q_OBJECT

public:
	CVLidarPlanDetail(QWidget* p, Core::CVControl* = 0, TabPosition = East);
	~CVLidarPlanDetail();

private:
	
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVLIDARPLANDETAIL_H
