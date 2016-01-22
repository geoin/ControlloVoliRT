#ifndef CV_FLY_ATTITUDE_DETAIL_H
#define CV_FLY_ATTITUDE_DETAIL_H

#include "cvbasedetail.h"

#include "core/categories/cvflyattitude.h"
#include "gui/cvgui_utils.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>

#include <QtConcurrentRun>
#include <QFuture>

class QLabel;
class QTableWidget;

namespace CV {
namespace GUI {
namespace Details {

class CVFlyAttitudeDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVFlyAttitudeDetail(QWidget *parent, Core::CVObject*);
	~CVFlyAttitudeDetail();

	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(QStringList&);

	inline Core::CVFlyAttitude* layer() const { return static_cast<Core::CVFlyAttitude*>(controller()); }

public slots:
	void onDataPersisted();
	void onItemInserted(int);
	void onAngleChanged(int);

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
    QScopedPointer<QFileInfo> _file;
	QList<QLabel*> _labels;

	QComboBox* _angle;
	QTableWidget* _table;

	CVProgressDialog _dialog;
	QFuture<bool> res;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_FLY_ATTITUDE_DETAIL_H
