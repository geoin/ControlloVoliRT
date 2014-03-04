#ifndef CV_GUI_CVNODEDETAILS_H
#define CV_GUI_CVNODEDETAILS_H

#include "gui/status/cvnodeinfo.h"
#include "core/categories/cvcontrol.h"

#include <QWidget>
#include <QMap>

class QStackedWidget;
class QTreeWidgetItem;

namespace CV {
namespace GUI {

class CVNodeDetails : public QWidget {
    Q_OBJECT
public:
    enum Type { PLAN  = 0 };

    explicit CVNodeDetails(QWidget *parent = 0);

    QWidget* getDetail(Core::CVControl::Type t) const { return _details.contains(t) ? _details.value(t) : NULL; }

signals:

public slots:
    void onProjectItemActivated(QTreeWidgetItem* item, int col);
	void onControlAdded(CV::Core::CVControl::Type, Core::CVControl* = NULL);
	void onProjectAdded(Core::CVProject*);
	void onClear();

private:
    QStackedWidget* _stack;
    QMap<Core::CVControl::Type, QWidget*> _details;
    QSharedPointer<QWidget> _projDetail;
};

} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVNODEDETAILS_H
