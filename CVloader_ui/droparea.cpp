
#include <QtGui>
#include <QProcess>
#include <QProcessEnvironment>
#include <QUrl>

#include "droparea.h"
#include "dropsitewindow.h"

DropArea::DropArea(QWidget *parent)
    : QLabel(parent)
{
    setMinimumSize(300, 200);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAlignment(Qt::AlignCenter);
    setAcceptDrops(true);
    setAutoFillBackground(true);
	setMargin(10);
	setWordWrap(true);
    clear();
}

void DropArea::dragEnterEvent(QDragEnterEvent *event)
{
    setBackgroundRole(QPalette::Highlight);

    event->acceptProposedAction();
}

void DropArea::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void DropArea::dropEvent(QDropEvent *event)
{
    setBackgroundRole(QPalette::Dark);
 
	const QMimeData* mData = event->mimeData();
	QList<QUrl>	ql = mData->urls();
	QString pth;
	for (int i = 0; i < ql.size(); i++) {
		QUrl q = ql[i];
		if ( i ) 
			pth +=";";
		pth += q.path();
	}
	emit changed(pth);

    event->acceptProposedAction();
}
void DropArea::item_changed(QTreeWidgetItem * qtw, int col)
{
	QVariant v = qtw->data(0, Qt::UserRole);
	item_obj o = v.value<item_obj>();
	QString qs;
	if ( !o.dropped().isEmpty() ) {
		setStyleSheet("* { background-color: rgb(0, 250, 50); }");
	    //setBackgroundRole(QPalette::Link);
		QStringList ql = o.dropped().split(";");
		for (int i = 0; i < ql.size(); i++) {
			QFileInfo qf(ql[i]);
			if ( !i ) {
				qs += qf.path();
			}
			qs += "\n";
			qs += qf.fileName();
		}
	} else {
		setStyleSheet("* { background-color: rgb(150, 150, 150); }");
		setBackgroundRole(QPalette::Dark);
		if ( !o.msg().isEmpty() )
			qs = o.msg();
		if ( !o.data_type().isEmpty() )
			qs += "\n(" + o.data_type() + ")";
	}
	setText(qs);
}

void DropArea::dragLeaveEvent(QDragLeaveEvent *event)
{
    clear();
    event->accept();
}

void DropArea::clear()
{
}

