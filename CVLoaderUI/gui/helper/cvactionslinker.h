#ifndef CV_GUI_HELPER_CVACTIONSLINKER_H
#define CV_GUI_HELPER_CVACTIONSLINKER_H

#include <QWidget>
#include <QMap>

#include <assert.h>

/*
Everything in here is intended to use only for GUI related event handling, 
is not thread safe and it shouldn't be.
*/

namespace CV {
namespace GUI {
namespace Helper {

enum Action_t {
    NEW_PROJECT = 0,
    LOAD_PROJECT,
	CLOSE_PROJECT,
	REMOVE_PROJECT
};

enum Signal_t {
    ITEM_SELECTED = 0,
	CAM_LOADED
};

class CVActionsLinker : public QObject {
    Q_OBJECT
public:
    CVActionsLinker(QObject* = NULL);

    QAction* add(QWidget* context, Action_t);
    QAction* add(Action_t);

	bool trig(Action_t ev);

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

class CVSignalLinker : public QObject {
    Q_OBJECT
public:
	CVSignalLinker(QObject* p = NULL) : QObject(p) {}

	void add(Signal_t, QObject*, const char*);
    bool on(Signal_t, const QObject* target, const char* slot);

private:
	struct _SigData {
		QObject* ptr;
		const char* signal;
	
		_SigData(QObject* ptr, const char* sig) : ptr(ptr), signal(sig) {}
	};

	typedef QSharedPointer<_SigData> _SigPtr;
    QMap<Signal_t, _SigPtr> _signals;

    Q_DISABLE_COPY(CVSignalLinker)
};

class CVSignalHandle {
public:
	static void create(QWidget* context) {
		assert(_linker == NULL);
        _linker = new CVSignalLinker(context);
    }

    static CVSignalLinker* get() {
        assert(_linker != NULL);
        return _linker;
    }
private:
    static CVSignalLinker* _linker;
};

} // namespace Helper
} // namespace GUI
} // namespace CV

#endif // CV_GUI_HELPER_CVACTIONSLINKER_H
