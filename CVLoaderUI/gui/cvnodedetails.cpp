#include "gui/cvnodedetails.h"
#include "gui/cvtreenode.h"

#include "gui/helper/cvdetailsfactory.h"

#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>

namespace CV {
namespace GUI {

using namespace Status;

CVNodeDetails::CVNodeDetails(QWidget* p) : QWidget(p) {
    setMinimumWidth(200);

    _stack = new QStackedWidget(this);

    QVBoxLayout* box = new QVBoxLayout;
    box->setContentsMargins(0, 0, 0, 0);
    box->addWidget(_stack);
    setLayout(box);
}

void CVNodeDetails::onControlAdded(CVNodeInfo::Type t, Core::CVCategory* controller) {
    if (_details.contains(t)) {
        return;
    }

    QWidget* detail = Helper::CVDetailsFactory::build(_stack, t, controller);

    _details.insert(t, detail);
    _stack->addWidget(detail);
}

void CVNodeDetails::onProjectItemActivated(QTreeWidgetItem* item, int col) {
    Q_UNUSED(col)

    CVTreeNode* node = static_cast<CVTreeNode*>(item);
	QMap<Status::CVNodeInfo::Type, QWidget*>::iterator iter = _details.find(node->info()->type());
    if (iter != _details.end()) {
        QWidget* widget = *iter;
        _stack->setCurrentWidget(widget);
    }
}

} // namespace GUI
} // namespace CV
