#include "gui/cvtreewidget.h"
#include "gui/cvtreenode.h"
#include "gui/cvtreenodedelegate.h"

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
    setCurrentItem(root);
    return root;
}

CVTreeNode* CVTreeWidget::insertNode(CVTreeNode* parent, const QString& text) {
	CVTreeNode* node = new CVTreeNode(parent, QStringList() << text);
    return node;
}

void CVTreeWidget::onCloseProject() {
	QTreeWidgetItem* inv = invisibleRootItem();
	if (inv->childCount()) {
		QTreeWidgetItem* root = takeTopLevelItem(0);
		setCurrentItem(root);
		delete root;
	}
}

}
}
