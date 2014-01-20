#ifndef CV_GUI_CVNODEDETAILS_H
#define CV_GUI_CVNODEDETAILS_H

#include <gui/status/cvnodeinfo.h>

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

signals:

public slots:
    void onProjectItemActivated(QTreeWidgetItem* item, int col);
    void onControlAdded(CV::GUI::Status::CVNodeInfo::Type);

private:
    QStackedWidget* _stack;
    QMap<Status::CVNodeInfo::Type, QWidget*> _details;
};

} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVNODEDETAILS_H
