#include "cvactionslinker.h"

#include <QAction>

namespace CV {
namespace GUI {
namespace Helper {

CVActionsLinker* CVActionHandle::_linker = NULL;

CVActionsLinker::CVActionsLinker(QObject* p) : QObject(p) {

}

QAction* CVActionsLinker::add(QWidget* context, Action_t a) {
    QAction* action = new QAction(context);
    _actions.insert(a, action);
    return action;
}

QAction* CVActionsLinker::add(Action_t a) {
    QAction* action = new QAction(parent());
    _actions.insert(a, action);
    return action;
}

bool CVActionsLinker::on(Action_t ev, const QObject* target, const char* slot) {
    bool ok = _actions.contains(ev);
    if (ok == true) {
        connect(_actions.value(ev), SIGNAL(triggered()), target, slot);
    }
    return ok;
}

} // namespace Helper
} // namespace GUI
} // namespace CV
