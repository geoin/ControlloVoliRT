/*
    File: droparea.cpp
    Author:  F.Flamigni
    Date: 2013 December 6
    Comment:

    Disclaimer:
        This file is part of RT_Controllo_Voli.

        Tabula is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        Tabula is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with Tabula.  If not, see <http://www.gnu.org/licenses/>.


        Copyright (C) 2013 Geoin s.r.l.

*/
#include <QtGui>
#include <QProcess>
#include <QProcessEnvironment>
#include <QUrl>

#include "droparea.h"
#include "dropwindow.h"

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

