#ifndef CV_GUI_DETAILS_CVBASEDETAIL_H
#define CV_GUI_DETAILS_CVBASEDETAIL_H

#include <QLabel>
#include <QWidget>
#include <QMenu>
#include <QFileInfo>
#include <QScopedPointer>
#include <QComboBox>
#include <QFileDialog>
#include <QDateTime>

#include "core/cvjournal.h"
#include "core/cvcore_utils.h"

#include "gui/cvshapeviewer.h"

class QLineEdit;

namespace CV {
namespace GUI {
namespace Details {

class CVBaseDetail : public QWidget {
    Q_OBJECT
public:
	explicit CVBaseDetail(QWidget* p, Core::CVObject*);
	virtual ~CVBaseDetail() {}

	inline void title(const QString& title) { _title->setText(title); }
	inline QString title() const { return _title->text(); }
	
	inline void info() { 
		Core::CVJournalEntry::Entry e = Core::CVJournal::last(controller()->controlType(), controller()->type());
		if (e != NULL) {
			_path->setText("Ultimo inserimento: " + e->uri);
			_date->setText("In data: " + e->date.toString());
		} else {
			_path->setText("");
			_date->setText("");
		}
	}

	inline void description(const QString& descr) { _descr->setText(descr); }
	inline QString description() const { return _descr->text(); }

	inline void body(QLayout* l) { _body->setLayout(l); }
	inline QWidget* body() { return _body; }
	inline QMenu* detailMenu() const { return _menu; }

	void createRow(QWidget* parent, const QString&, QLabel*& lab, QLabel*& info);
	void createRow(QWidget* parent, const QString&, QLabel*& lab, QComboBox*& info);
	
    QLineEdit* lineEdit(QWidget* p, const QPalette&); // TODO: I can find a palette from p, remove

	void disableLineEdit(QWidget* p, QLineEdit*);
	void enableLineEdit(QLineEdit*);

protected:
	inline Core::CVObject* controller() const { return _controller; }

public slots: //declared as slots only here, not in derived
	virtual void clearAll() = 0;
	virtual void searchFile() = 0;
	virtual void importAll(QStringList&) {}
	virtual void importAll(const QStringList&) {}

private:
	Core::CVObject* _controller;

	QLabel* _title, * _descr, *_path, *_date;
	QWidget* _body;
	QMenu* _menu;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVBASEDETAIL_H
