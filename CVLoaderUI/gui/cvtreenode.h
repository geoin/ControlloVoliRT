#ifndef CV_GUI_CVTREENODE_H
#define CV_GUI_CVTREENODE_H

#include "status/cvnodeinfo.h"

#include <QTreeWidgetItem>

namespace CV {
namespace GUI {

class CVTreeNodeDelegate;

class CVTreeNode : public QTreeWidgetItem {

public:
    explicit CVTreeNode(QTreeWidget* = 0, const QStringList& s = QStringList());
    explicit CVTreeNode(QTreeWidgetItem* = 0, const QStringList& s = QStringList());

    CVTreeNodeDelegate* delegate() const;
    Status::CVNodeInfo* info() const;

signals:

public slots:

private:
    CVTreeNodeDelegate* _delegate;
    QScopedPointer<Status::CVNodeInfo> _info;
};

} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVTREENODE_H
