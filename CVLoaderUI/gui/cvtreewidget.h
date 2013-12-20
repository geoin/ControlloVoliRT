#ifndef CVTREEWIDGET_H
#define CVTREEWIDGET_H

#include <QTreeWidget>

namespace CV {
namespace GUI {

class CVTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit CVTreeWidget(QWidget *parent = 0);

signals:

public slots:
    void createProjectTree(const QString&);

};

}
}

#endif // CV_GUI_CVTREEWIDGET_H
