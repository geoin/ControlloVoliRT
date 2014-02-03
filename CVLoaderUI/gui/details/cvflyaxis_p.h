#ifndef CVFLYAXIS_P_H
#define CVFLYAXIS_P_H

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

class CVFlyAxis_p : public CVBaseDetail {
	Q_OBJECT

public:
	CVFlyAxis_p(QWidget *parent, Core::CVObject*);
	~CVFlyAxis_p();
	
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

#endif // CVFLYAXIS_P_H
