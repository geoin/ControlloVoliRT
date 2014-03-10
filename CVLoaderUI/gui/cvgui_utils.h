#ifndef CV_GUI_UTILS_H
#define CV_GUI_UTILS_H

#include <QApplication>
#include <QProgressDialog>
#include <QProgressBar>
#include <QMessageBox>
#include <QSpacerItem>
#include <QGridLayout>

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
		
		setWindowModality(Qt::ApplicationModal);
		setCancelButton(NULL);
		setRange(0, 0);
        adjustSize();
	}

	~CVProgressDialog() {
		reset();
	}

	inline void resizeBarWidth(int w) {
		_bar.setMinimumWidth(w);
        adjustSize();
	}

private:
	QProgressBar _bar;
};

class CVMessageBox {
public:
	static int message(QWidget* p, QString title, QString info, int minWidth = 260, QString icon = QString(), QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton def = QMessageBox::Ok) {
        QMessageBox msgBox(p);
		msgBox.setText(title);
		msgBox.setInformativeText(info);
		msgBox.setIconPixmap(QPixmap(icon));
		msgBox.setStandardButtons(buttons);
		msgBox.setDefaultButton(def);

		//WORKAROUND (CHECK LATER): for some reason I can resize the box without adding some dummy stuff to messagebox layout
		QSpacerItem* horizontalSpacer = new QSpacerItem(minWidth, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
		QGridLayout* layout = static_cast<QGridLayout*>(msgBox.layout());
		layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
		return msgBox.exec();
	}
};


}
}

#endif
