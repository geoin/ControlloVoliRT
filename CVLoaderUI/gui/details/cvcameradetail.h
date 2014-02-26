#ifndef CV_GUI_DETAILS_CVCAMERADETAIL_H
#define CV_GUI_DETAILS_CVCAMERADETAIL_H

#include "core/categories/cvcamera.h"

#include "cvbasedetail.h"

#include <QWidget>
#include <QMap>
#include <QFileInfo>
#include <QScopedPointer>

class QPlainTextEdit;
class QLineEdit;

namespace CV {
namespace GUI {
namespace Details {

class CVCameraDetail : public CVBaseDetail {
    Q_OBJECT
public:
    explicit CVCameraDetail(QWidget *parent, Core::CVObject*);
	
	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(QStringList&);

	void save();
	void view();

	inline Core::CVCamera* camera() const { return static_cast<Core::CVCamera*>(controller()); }

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
    QScopedPointer<QFileInfo> _file;

    QMap<QString, QLineEdit*> _params;
    QString _uri;

    QPlainTextEdit* _note;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVCAMERADETAIL_H
