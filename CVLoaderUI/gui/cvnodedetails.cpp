#include "gui/cvnodedetails.h"
#include "gui/cvtreenode.h"

#include "gui/helper/cvdetailsfactory.h"

#include "gui/cvprojectoverviewdetail.h"

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

void CVNodeDetails::onControlAdded(Core::CVControl::Type t, Core::CVControl* controller) {
    if (_details.contains(t)) {
        return;
    }

    QWidget* detail = Helper::CVDetailsFactory::build(_stack, t, controller);

	if (detail != NULL) {
		_details.insert(t, detail);
		_stack->addWidget(detail);
	}
}

void CVNodeDetails::onProjectItemActivated(QTreeWidgetItem* item, int col) {
    Q_UNUSED(col)

    CVTreeNode* node = static_cast<CVTreeNode*>(item);
	if (node->info()->isProjectRoot()) {
        _stack->setCurrentWidget(_projDetail.data());
	} else {
		QMap<Core::CVControl::Type, QWidget*>::iterator iter = _details.find(node->info()->type());
		if (iter != _details.end()) {
			QWidget* widget = *iter;
			_stack->setCurrentWidget(widget);
		}
	}
}

void CVNodeDetails::onClear() {
	for (int i = 0; i < _stack->count(); ++i) {
		QWidget* w = _stack->widget(i);
		_stack->removeWidget(w);
	}

	foreach (QWidget* w, _details) {
		delete w;
	}
	_details.clear();
    _projDetail.clear();
}

void CVNodeDetails::onProjectAdded(Core::CVProject* proj) {
    Details::CVProjectOverviewDetail* d = new Details::CVProjectOverviewDetail(NULL, proj); //(CHECK) 0xOO.. already has reference counting if I set parent
    _projDetail = QSharedPointer<QWidget>(d);
    _stack->addWidget(d);
}

} // namespace GUI
} // namespace CV
