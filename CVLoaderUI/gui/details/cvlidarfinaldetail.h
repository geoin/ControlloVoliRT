#ifndef CV_LIDAR_FINAL_DETAIL_H
#define CV_LIDAR_FINAL_DETAIL_H

#include "gui/cvbasetabwidget.h"
#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {
namespace Details {


class CVLidarFinalDetail : public CVBaseTabWidget {
	Q_OBJECT

public:
	CVLidarFinalDetail(QWidget* p, Core::CVControl* = 0, TabPosition = East);
	~CVLidarFinalDetail();

private:
	
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_LIDAR_FINAL_DETAIL_H
