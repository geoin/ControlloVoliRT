#ifndef CV_CONTOUR_DETAIL_H
#define CV_CONTOUR_DETAIL_H

#include "core/categories/cvshapelayer.h"

#include "cvbasedetail.h"
#include "gui/cvshapeviewer.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>

class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVContourDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVContourDetail(QWidget *parent, Core::CVObject*);
	~CVContourDetail();

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

	ShapeViewer* _shape;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVAREADETAIL_H
