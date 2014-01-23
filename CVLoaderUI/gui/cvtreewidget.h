#ifndef CVTREEWIDGET_H
#define CVTREEWIDGET_H

#include <QTreeWidget>

namespace CV {
namespace GUI {

class CVTreeNode;

class CVTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit CVTreeWidget(QWidget *parent = 0);

    CVTreeNode* insertProjectTree(const QString&);
    CVTreeNode* insertNode(CVTreeNode* parent, const QString&);

public slots:
	void onCloseProject();
};

}
}

#endif // CV_GUI_CVTREEWIDGET_H
