#ifndef CVRAWSTRIPFOLDERDETAIL_P_H
#define CVRAWSTRIPFOLDERDETAIL_P_H

#include "core/categories/cvfolderinput.h"

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

class CVRawStripDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVRawStripDetail(QWidget *parent, Core::CVObject*);
	~CVRawStripDetail();

	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(QStringList&);
	
	inline Core::CVFolderInput* folder() const { return static_cast<Core::CVFolderInput*>(controller()); }

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

#endif // CVDEMDETAIL_P_H
