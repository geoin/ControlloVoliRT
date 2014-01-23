#ifndef CVDEMDETAIL_P_H
#define CVDEMDETAIL_P_H

#include "core/categories/cvfileinput.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>

class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVDemDetail : public QWidget {
	Q_OBJECT

public:
	CVDemDetail(QWidget *parent, Core::CVFileInput*);
	~CVDemDetail();

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

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