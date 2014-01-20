#include "gui/cvtreewidget.h"
#include "gui/cvtreenode.h"

#include <QLabel>

namespace CV {
namespace GUI {

CVTreeWidget::CVTreeWidget(QWidget* parent) : QTreeWidget(parent) {
    setColumnCount(1);
    setHeaderHidden(true);
    //setFrameShape(QFrame::NoFrame);

    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setMinimumWidth(260);
}

CVTreeNode* CVTreeWidget::insertProjectTree(const QString& title) {
    CVTreeNode* root = new CVTreeNode(this, QStringList() << title);
    addTopLevelItem(root);
    root->setExpanded(true);
    setCurrentItem(root);
    return root;
}

CVTreeNode* CVTreeWidget::insertNode(CVTreeNode* parent, const QString& text) {
    return new CVTreeNode(parent, QStringList() << text);
}

}
}
