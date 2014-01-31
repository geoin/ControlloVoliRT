#ifndef CVAREADETAIL_H
#define CVAREADETAIL_H

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

class CVAreaDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVAreaDetail(QWidget *parent, Core::CVShapeLayer*);
	~CVAreaDetail();

	virtual void clearAll();
	virtual void searchFile() {}
	virtual void importAll(const QStringList&) {}

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);
	virtual void showEvent(QShowEvent* event) {
		if (_layer->isValid()) {
			QStringList info = _layer->data();
			for (int i = 0; i < info.size(); ++i) {
				QLabel* lab = _labels.at(i);
				lab->setText(info.at(i));
			}
		}
	}

private:
    QScopedPointer<QFileInfo> _file;
	QString _uri;
	QList<QLabel*> _labels;

	Core::CVShapeLayer* _layer;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVAREADETAIL_H
