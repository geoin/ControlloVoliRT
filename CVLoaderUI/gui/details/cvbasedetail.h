#ifndef CV_GUI_DETAILS_CVBASEDETAIL_H
#define CV_GUI_DETAILS_CVBASEDETAIL_H

#include <QLabel>
#include <QWidget>
#include <QMenu>
#include <QFileInfo>
#include <QScopedPointer>
#include <QFileDialog>

#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {
namespace Details {

class CVBaseDetail : public QWidget {
    Q_OBJECT
public:
	explicit CVBaseDetail(QWidget* p, Core::CVObject*);

	inline void title(const QString& title) { _title->setText(title); }
	inline QString title() const { return _title->text(); }

	inline void description(const QString& descr) { _descr->setText(descr); }
	inline QString description() const { return _descr->text(); }

	inline void body(QLayout* l) { _body->setLayout(l); }
	inline QMenu* detailMenu() const { return _menu; }

	void createRow(QWidget* parent, const QString&, QLabel*& lab, QLabel*& info);

protected:
	inline Core::CVObject* controller() const { return _controller; }

public slots: //declared as slots only here, not in derived
	virtual void clearAll() = 0;
	virtual void searchFile() = 0;
	virtual void importAll(QStringList&) = 0;

private:
	Core::CVObject* _controller;

	QLabel* _title, * _descr;
	QWidget* _body;
	QMenu* _menu;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVBASEDETAIL_H
