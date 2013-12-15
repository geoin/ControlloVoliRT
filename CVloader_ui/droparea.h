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
#ifndef DROPAREA_H
#define DROPAREA_H

#include <QLabel>

QT_BEGIN_NAMESPACE
class QMimeData;
class QTreeWidgetItem;
QT_END_NAMESPACE

class DropArea : public QLabel {
    Q_OBJECT
public:
    DropArea(QWidget *parent = 0);
	void append(const QString& ms);
	void clean(void) { _mes.clear(); }
	void print(void) { setText(_mes); }

public slots:
	void item_changed(QTreeWidgetItem *, int);
signals:
    void changed(const QString& file);
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
private:
	QString _mes;
    QLabel *label;
};

#endif
