#ifndef CVAREADETAIL_H
#define CVAREADETAIL_H

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

class CVAreaDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVAreaDetail(QWidget *parent, Core::CVObject*, Core::CVControl::Type);
	~CVAreaDetail();

	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(QStringList&);
	
	inline Core::CVShapeLayer* layer() const { return static_cast<Core::CVShapeLayer*>(controller()); }

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);
	virtual void showEvent(QShowEvent* event) {
		Core::CVObject* obj = controller(); // check destruction order, maybe a weak pointer?
		if (!obj) {
			return;
		}
		if (obj->isValid()) {
			obj->controlType(_control);
			QStringList info = layer()->data();
			_labels.at(0)->setText(QString::number(layer()->rows()));
		}
	}

private:
    QScopedPointer<QFileInfo> _file;
	QString _uri;
	QList<QLabel*> _labels;

	Core::CVControl::Type _control;

	ShapeViewer* _shape;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVAREADETAIL_H
