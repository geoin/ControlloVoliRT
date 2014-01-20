#ifndef CV_GUI_HELPER_CVACTIONSLINKER_H
#define CV_GUI_HELPER_CVACTIONSLINKER_H

#include <QWidget>
#include <QMap>

#include <assert.h>

namespace CV {
namespace GUI {
namespace Helper {

enum Action_t {
    NEW_PROJECT = 0,
    LOAD_PROJECT,
	CLOSE_PROJECT,
	REMOVE_PROJECT
};

class CVActionsLinker : public QObject {
    Q_OBJECT
public:
    CVActionsLinker(QObject* = NULL);

    QAction* add(QWidget* context, Action_t);
    QAction* add(Action_t);

    bool on(Action_t, const QObject* target, const char* slot);

private:
    QMap<Action_t, QAction*> _actions;

    Q_DISABLE_COPY(CVActionsLinker)
};

class CVActionHandle {
public:
	static void create(QWidget* context) {
		assert(_linker == NULL);
        _linker = new CVActionsLinker(context);
    }

    static CVActionsLinker* get() {
        assert(_linker != NULL);
        return _linker;
    }
private:
    static CVActionsLinker* _linker;
};

} // namespace Helper
} // namespace GUI
} // namespace CV

#endif // CV_GUI_HELPER_CVACTIONSLINKER_H
