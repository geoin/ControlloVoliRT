#include "gui/cvtreewidget.h"
#include "gui/cvtreenode.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QLabel>

namespace CV {
namespace GUI {

CVTreeWidget::CVTreeWidget(QWidget* parent) : QTreeWidget(parent) {
    setColumnCount(1);
    setHeaderHidden(true);
    setFrameShape(QFrame::NoFrame);

    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setMinimumWidth(200);

    createProjectTree(tr("Progetto di test 1"));
    createProjectTree(tr("Progetto di test 2"));
    createProjectTree(tr("Progetto di test 3"));
}

void CVTreeWidget::createProjectTree(const QString& title) {
    auto root = new CVTreeNode(this, QStringList() << title);
    addTopLevelItem(root);
    new CVTreeNode(root, QStringList() << tr("Progettazione"));
    new CVTreeNode(root, QStringList() << tr("Dati GPS"));
    new CVTreeNode(root, QStringList() << tr("Voli"));
}

}
}
