#ifndef CV_GUI_DETAILS_CVRINEXDETAIL_H
#define CV_GUI_DETAILS_CVRINEXDETAIL_H

#include "cvbasedetail.h"
#include "core/categories/cvrinex.h"

class QListWidget;
class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVRinexDetail : public CVBaseDetail {
    Q_OBJECT
public:
	explicit CVRinexDetail(QWidget *parent, Core::CVRinex*);

	virtual void clearAll();

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
	QLabel* _name;
	QListWidget* _details;
	
	QString _base, _station;
    QList<QString> _files;
	
	Core::CVRinex* _rinex;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVRINEXDETAIL_H
