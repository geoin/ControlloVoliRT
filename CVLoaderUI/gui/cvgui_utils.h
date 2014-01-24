#ifndef CV_GUI_UTILS_H
#define CV_GUI_UTILS_H

#include <QApplication>

namespace CV {
namespace GUI {

class CVScopedCursor {
public:
	CVScopedCursor(Qt::CursorShape s = Qt::WaitCursor) {
		QApplication::setOverrideCursor(s);
	}
	~CVScopedCursor() {
		QApplication::restoreOverrideCursor();
	}

};

}
}

#endif