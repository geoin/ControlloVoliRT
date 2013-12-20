#include "gui/cvnodedetails.h"
#include "gui/cvtreenode.h"

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>

namespace CV {
namespace GUI {

CVNodeDetails::CVNodeDetails(QWidget* p) : QWidget(p) {
    setMinimumWidth(200);

    _stack = new QStackedWidget(this);
    _stack->addWidget(new QLabel("", this));
}

void CVNodeDetails::onProjectItemActivated(QTreeWidgetItem* item, int col) {
    auto node = static_cast<CVTreeNode*>(item);
    int r = node->treeWidget()->rootIndex().row();
}

} // namespace GUI
} // namespace CV
