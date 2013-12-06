
#include <QtGui>
#include <QProcess>
#include <QProcessEnvironment>

#include "droparea.h"

DropArea::DropArea(QWidget *parent)
    : QLabel(parent)
{
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAlignment(Qt::AlignCenter);
    setAcceptDrops(true);
    setAutoFillBackground(true);
    clear();
}

void DropArea::dragEnterEvent(QDragEnterEvent *event)
{
    setText(tr("<drop content>"));
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
	QString qs = mData->text();
	QList<QUrl>	ql = mData->urls();

    event->acceptProposedAction();
}

//! [dragLeaveEvent() function]
void DropArea::dragLeaveEvent(QDragLeaveEvent *event)
{
    clear();
    event->accept();
}

//! [clear() function]
void DropArea::clear()
{
	setText(tr("<drop content>"));
	setBackgroundRole(QPalette::Dark);

    emit changed();
}

