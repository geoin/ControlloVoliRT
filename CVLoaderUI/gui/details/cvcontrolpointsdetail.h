#ifndef CV_CONTROL_POINTS_DETAIL_H
#define CV_CONTROL_POINTS_DETAIL_H

#include "core/categories/cvshapelayer.h"

#include "cvbasedetail.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>

class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVControlPointsDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVControlPointsDetail(QWidget *parent, Core::CVObject*);
	~CVControlPointsDetail();

	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(QStringList&);
	
	inline Core::CVShapeLayer* layer() const { return static_cast<Core::CVShapeLayer*>(controller()); }

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
    QScopedPointer<QFileInfo> _file;
	QString _uri;
	QList<QLabel*> _labels;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_CONTROL_POINTS_DETAIL_H
