#ifndef CV_GUI_CVTREENODE_H
#define CV_GUI_CVTREENODE_H

#include <QTreeWidgetItem>

namespace CV {
namespace GUI {

class CVTreeNodeDelegate;

class CVTreeNode : public QTreeWidgetItem {

public:
    explicit CVTreeNode(QTreeWidget* parent = 0, const QStringList& s = QStringList());
    explicit CVTreeNode(QTreeWidgetItem* parent = 0, const QStringList& s = QStringList());

    CVTreeNodeDelegate* delegate() const;

signals:

public slots:

private:
    CVTreeNodeDelegate* _delegate;
};

} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVTREENODE_H
