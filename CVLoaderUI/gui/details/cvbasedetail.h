#ifndef CV_GUI_DETAILS_CVBASEDETAIL_H
#define CV_GUI_DETAILS_CVBASEDETAIL_H

#include <QLabel>
#include <QWidget>
#include <QMenu>
#include <QFileInfo>
#include <QScopedPointer>

namespace CV {
namespace GUI {
namespace Details {

class CVBaseDetail : public QWidget {
    Q_OBJECT
public:
    explicit CVBaseDetail(QWidget *parent = 0);

	inline void title(const QString& title) { _title->setText(title); }
	inline QString title() const { return _title->text(); }

	inline void description(const QString& descr) { _descr->setText(descr); }
	inline QString description() const { return _descr->text(); }

	inline void body(QLayout* l) { _body->setLayout(l); }
	inline QMenu* detailMenu() const { return _menu; }

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
    QScopedPointer<QFileInfo> _file;
	QString _uri;

	QLabel* _title, * _descr;
	QWidget* _body;
	QMenu* _menu;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVBASEDETAIL_H
