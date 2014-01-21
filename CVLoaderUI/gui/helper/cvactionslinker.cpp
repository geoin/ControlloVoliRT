#include "cvactionslinker.h"

#include <QAction>

namespace CV {
namespace GUI {
namespace Helper {

CVActionsLinker* CVActionHandle::_linker = NULL;

CVSignalLinker* CVSignalHandle::_linker = NULL;

CVActionsLinker::CVActionsLinker(QObject* p) : QObject(p) { }

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

bool CVActionsLinker::trig(Action_t ev) {
	bool ok = _actions.contains(ev);
    if (ok == true) {
		QAction* a = _actions.value(ev);
		a->trigger();
    }
    return ok;
}

void CVSignalLinker::add(Signal_t t, QObject* c, const char* sig) {
	_SigPtr d(new _SigData(c, sig));
	_signals.insert(t, d);
}
    
bool CVSignalLinker::on(Signal_t ev, const QObject* target, const char* slot) {
	bool ok = _signals.contains(ev);
    if (ok == true) {
		_SigPtr d = _signals.value(ev);
		assert(!d.isNull());
		connect(d->ptr, d->signal, target, slot);
    }
    return ok;
}

} // namespace Helper
} // namespace GUI
} // namespace CV
