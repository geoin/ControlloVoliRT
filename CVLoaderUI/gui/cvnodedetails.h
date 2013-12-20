#ifndef CV_GUI_CVNODEDETAILS_H
#define CV_GUI_CVNODEDETAILS_H

#include <QWidget>

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

private:
    QStackedWidget* _stack;
};

} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVNODEDETAILS_H
