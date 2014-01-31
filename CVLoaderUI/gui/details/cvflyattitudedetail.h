#ifndef CV_FLY_ATTITUDE_DETAIL_H
#define CV_FLY_ATTITUDE_DETAIL_H

#include "core/categories/cvflyattitude.h"

#include "cvbasedetail.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>

class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVFlyAttitudeDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVFlyAttitudeDetail(QWidget *parent, Core::CVFlyAttitude*);
	~CVFlyAttitudeDetail();

	virtual void clearAll();
	virtual void searchFile() {}
	virtual void importAll(const QStringList&);

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
    QScopedPointer<QFileInfo> _file;
	QList<QLabel*> _labels;

	Core::CVFlyAttitude* _layer;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_FLY_ATTITUDE_DETAIL_H
