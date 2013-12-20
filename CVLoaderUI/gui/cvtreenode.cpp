#include "gui/cvtreenode.h"
#include "gui/cvtreenodedelegate.h"

namespace CV {
namespace GUI {

CVTreeNode::CVTreeNode(QTreeWidget* p, const QStringList& s) : QTreeWidgetItem(p) {
    _delegate = new CVTreeNodeDelegate(p, s);
    p->setItemWidget(this, 0, _delegate);
}

CVTreeNode::CVTreeNode(QTreeWidgetItem* p, const QStringList& s) : QTreeWidgetItem(p) {
    auto tree = p->treeWidget();
    _delegate = new CVTreeNodeDelegate(tree, s);
    tree->setItemWidget(this, 0, _delegate);
}

CVTreeNodeDelegate* CVTreeNode::delegate() const {
    return _delegate;
}


} // namespace GUI
} // namespace CV
