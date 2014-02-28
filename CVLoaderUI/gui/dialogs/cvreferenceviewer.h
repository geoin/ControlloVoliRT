#ifndef CV_GUI_DIALOGS_CVREFERENCEVIEWER_H
#define CV_GUI_DIALOGS_CVREFERENCEVIEWER_H

#include "core/cvproject.h"

#include <QDialog>
#include <QDomDocument>

class QTableWidget;

namespace CV {
namespace GUI {
namespace Dialogs {

class CVReferenceViewer : public QDialog {
    Q_OBJECT
public:
    explicit CVReferenceViewer(QWidget* p = 0);

	void updateFrom(const Core::CVProject&);

protected:
	virtual void showEvent(QShowEvent*);

private:
	void _viewTarget(const QDomElement&, const QString&);

	QTableWidget* _viewer;
	QString _title;
};

} // namespace Dialogs
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DIALOGS_CVREFERENCEVIEWER_H
