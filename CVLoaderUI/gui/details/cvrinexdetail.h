#ifndef CV_GUI_DETAILS_CVRINEXDETAIL_H
#define CV_GUI_DETAILS_CVRINEXDETAIL_H

#include "cvbasedetail.h"
#include "core/categories/cvrinex.h"

namespace CV {
namespace GUI {
namespace Details {

class CVRinexDetail : public CVBaseDetail {
    Q_OBJECT
public:
	explicit CVRinexDetail(QWidget *parent, Core::CVRinex*);

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
    QScopedPointer<QFileInfo> _file;
	Core::CVRinex* _rinex;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVRINEXDETAIL_H
