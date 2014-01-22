#ifndef CVFLYAXIS_P_H
#define CVFLYAXIS_P_H

#include "core/categories/cvshapelayer.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>

class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVFlyAxis_p : public QWidget {
	Q_OBJECT

public:
	CVFlyAxis_p(QWidget *parent, Core::CVShapeLayer*);
	~CVFlyAxis_p();

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
    QScopedPointer<QFileInfo> _file;
	QString _uri;
	QList<QLabel*> _labels;

	Core::CVShapeLayer* _layer;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVFLYAXIS_P_H
