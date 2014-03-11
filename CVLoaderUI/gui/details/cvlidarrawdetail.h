#ifndef CV_LIDAR_RAW_DETAIL_H
#define CV_LIDAR_RAW_DETAIL_H

#include "gui/cvbasetabwidget.h"
#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {
namespace Details {

class CVLidarRawDetail : public CVBaseTabWidget {
	Q_OBJECT

public:
	CVLidarRawDetail(QWidget* p, Core::CVControl* = 0, TabPosition = East);
	~CVLidarRawDetail();

private:
	
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_LIDAR_RAW_DETAIL_H
