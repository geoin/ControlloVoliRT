#ifndef CVDEMDETAIL_P_H
#define CVDEMDETAIL_P_H

#include "core/categories/cvfileinput.h"

#include "cvbasedetail.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>
#include <QShowEvent>

class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVDemDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVDemDetail(QWidget *parent, Core::CVFileInput*);
	~CVDemDetail();

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
			QStringList& data = _layer->data();
			for (int i = 0; i < data.size(); ++i) {
				_labels.at(i)->setText(data.at(i));
			}
		}
	}

private:
    QScopedPointer<QFileInfo> _file;
	QString _uri;
	QList<QLabel*> _labels;

	Core::CVFileInput* _layer;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVDEMDETAIL_P_H
