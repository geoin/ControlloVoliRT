#ifndef CV_GUI_UTILS_H
#define CV_GUI_UTILS_H

#include <QApplication>
#include <QProgressDialog>
#include <QProgressBar>
#include <QMessageBox>

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

class CVProgressDialog : public QProgressDialog {
public:
    CVProgressDialog(QWidget* p = NULL, Qt::WindowFlags f = 0) : QProgressDialog(p, f), _bar(this) {
		setWindowFlags(((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint) & ~Qt::WindowContextHelpButtonHint);
		setBar(&_bar);
		
		setWindowModality(Qt::WindowModal);
		setCancelButton(NULL);
		setRange(0, 0);
	}

	~CVProgressDialog() {
		reset();
	}

	inline void resizeBarWidth(int w) {
		_bar.setMinimumWidth(w);
	}

private:
	QProgressBar _bar;
};

class CVMessageBox {
public:
	static int message(QWidget* p, QString title, QString info, QString icon, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton def = QMessageBox::Ok) {
		QMessageBox msgBox;
		msgBox.setText(title);
		msgBox.setInformativeText(info);
		msgBox.setIconPixmap(QPixmap(icon));
		msgBox.setStandardButtons(buttons);
		msgBox.setDefaultButton(def);
		return msgBox.exec();
	}
};


}
}

#endif
