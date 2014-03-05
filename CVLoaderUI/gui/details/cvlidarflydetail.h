#ifndef CVLIDARFLYDETAIL_H
#define CVLIDARFLYDETAIL_H

#include "gui/cvbasetabwidget.h"
#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {
namespace Details {

class CVLidarFlyDetail : public CVBaseTabWidget {
	Q_OBJECT

public:
	CVLidarFlyDetail(QWidget* p, Core::CVControl* = 0, TabPosition = East);
	~CVLidarFlyDetail();

private:
	
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVLIDARFLYDETAIL_H
