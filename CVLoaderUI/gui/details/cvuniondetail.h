#ifndef CV_UNION_DETAIL_H
#define CV_UNION_DETAIL_H

#include "core/categories/cvshapelayer.h"

#include "gui/cvshapeviewer.h"
#include "cvbasedetail.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>

class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVUnionDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVUnionDetail(QWidget *parent, Core::CVObject*);
	~CVUnionDetail();

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

#endif // CV_UNION_DETAIL_H
