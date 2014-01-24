#include "gui/cvtreenode.h"
#include "gui/cvtreenodedelegate.h"

namespace CV {
namespace GUI {

using namespace Status;

CVTreeNode::CVTreeNode(QTreeWidget* p, const QStringList& s) : QTreeWidgetItem(p) {
    _delegate = new CVTreeNodeDelegate(p, s);
    p->setItemWidget(this, 0, _delegate);

    _info.reset(new CVNodeInfo);
    _info->isProjectRoot(true);

    setSizeHint(0, QSize(22, 22));
}

CVTreeNode::CVTreeNode(QTreeWidgetItem* p, const QStringList& s) : QTreeWidgetItem(p) {
    QTreeWidget* tree = p->treeWidget();
    _delegate = new CVTreeNodeDelegate(tree, s);
    tree->setItemWidget(this, 0, _delegate);

    _info.reset(new CVNodeInfo);
    _info->isProjectRoot(false);

    setSizeHint(0, QSize(22, 22));
}

CVTreeNodeDelegate* CVTreeNode::delegate() const {
    return _delegate;
}

CVNodeInfo* CVTreeNode::info() const {
    return _info.data();
}

} // namespace GUI
} // namespace CV
