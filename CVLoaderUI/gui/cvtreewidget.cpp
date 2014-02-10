#include "gui/cvtreewidget.h"
#include "gui/cvtreenode.h"
#include "gui/cvtreenodedelegate.h"

#include <QLabel>

namespace CV {
namespace GUI {

CVTreeWidget::CVTreeWidget(QWidget* parent) : QTreeWidget(parent) {
    setColumnCount(1);
    setHeaderHidden(true);
    setFrameShape(QFrame::NoFrame);
	setFocusPolicy(Qt::NoFocus);

    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setMinimumWidth(260);
}

CVTreeNode* CVTreeWidget::insertProjectTree(const QString& title, Core::CVProject::Type t) {
	QString type;
	type = t == Core::CVProject::PHOTOGRAMMETRY ? tr("Fotogrammetria") : tr("Lidar");

	CVTreeNode* root = new CVTreeNode(this, QStringList() << type + ": " + title);
	root->info()->projType(t);
    addTopLevelItem(root);
    setCurrentItem(root);
    return root;
}

CVTreeNode* CVTreeWidget::insertNode(CVTreeNode* parent, const QString& text, Core::CVControl::Type t) {
	CVTreeNode* node = new CVTreeNode(parent, QStringList() << text);
	node->info()->type(t);
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
