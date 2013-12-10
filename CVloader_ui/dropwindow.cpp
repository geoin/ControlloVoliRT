/*
    File: dropwindow.cpp
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
#include <QFileDialog>
#include "droparea.h"
#include "dropwindow.h"

DropSiteWindow::DropSiteWindow()
{
    _drop_area = new DropArea;
    connect(_drop_area, SIGNAL(changed(const QString&)), this, SLOT(_assign_dropped(const QString&)));

	item_obj io;

    _prj_tree = new QTreeWidget;
	_prj_tree->setHeaderLabel("RT");

	QTreeWidgetItem* qt = new QTreeWidgetItem;
	io = item_obj("Progetto", "Premere ins per definire la cartella del progetto", "", item_obj::ty_project, true, false);
	qt->setText(0, io.name());
	qt->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	_prj_tree->addTopLevelItem(qt);

	QTreeWidgetItem* q1 = new QTreeWidgetItem;
	io = item_obj("Fotogrammetria", "", "", item_obj::ty_fotogra, false, false);
	q1->setText(0, io.name());
	q1->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));

	QList<QTreeWidgetItem*> qcl;
	QTreeWidgetItem* q2 = new QTreeWidgetItem;
	io = item_obj("Assi di volo progettati", "Shape file", "Trascinare\nil file degli assi di volo progettati", item_obj::ty_assi_p);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Assi di volo", "Shape file", "Trascinare\nil file degli assi di volo", item_obj::ty_assi);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Aree da cartografare", "Shape file", "Trascinare\nil file delle aree da cartografare", item_obj::ty_carto);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Assetti progettati", "txt file", "Trascinare\nil file degli assetti progettati", item_obj::ty_assetti_p);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Assetti effettivi", "txt file", "Trascinare\nil file degli assetti effettivi", item_obj::ty_assetti);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Dati Fotocamera", "xml file", "Trascinare\nil file descrittivo della fotocamera", item_obj::ty_camera);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Modelo numerico del terreno", "asc grid", "Trascinare\nil file del modello numerico del terreno", item_obj::ty_dem);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Dati GPS", "", "Premere ins per aggiungere missioni, del per rimuoverle", item_obj::ty_gps, true, false);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Quadro unione ortofoto", "Shape file", "Trascinare\nil file del quadro d'unione\n", item_obj::ty_quadro);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q1->addChildren(qcl);

	qt->addChild(q1);
	
	q1 = new QTreeWidgetItem;
	q1->setText(0, "Lidar");
	qt->addChild(q1);
	connect(_prj_tree, SIGNAL(itemClicked (QTreeWidgetItem *, int)), _drop_area, SLOT(item_changed(QTreeWidgetItem *, int)));

    clearButton = new QPushButton(tr("Clear"));
    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(clearButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    connect(quitButton, SIGNAL(pressed()), this, SLOT(close()));
    connect(clearButton, SIGNAL(pressed()), _drop_area, SLOT(clear()));

	QSplitter* qs = new QSplitter;
	qs->setOrientation(Qt::Horizontal);
	qs->addWidget(_prj_tree);
	qs->addWidget(_drop_area);
	//QDockWidget* qd = new QDockWidget;
	//qd->setWidget(_prj_tree);
	//addDocWidget(Qt::TopDockWidgetArea, qd);
	//qd = new QDockWidget;
	//qd->setWidget(_drop_area);
	//addDocWidget(Qt::TopDockWidgetArea, qd);

    //QHBoxLayout *hl = new QHBoxLayout;
    //hl->addWidget(_prj_tree);
    //hl->addWidget(_drop_area);

    QVBoxLayout *vl = new QVBoxLayout;
	vl->addWidget(qs);
	vl->addWidget(buttonBox);
    setLayout(vl);

    setWindowTitle(tr("Gestione Progetto"));
    setMinimumSize(350, 500);
}

void DropSiteWindow::_assign_dropped(const QString& path)
{
	QTreeWidgetItem * wi = _prj_tree->currentItem();
	QVariant v = wi->data(0, Qt::UserRole);
	item_obj io = v.value<item_obj>();
	if ( io.acceptdrop() ) {
		io.dropped(path);
		wi->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
		_drop_area->item_changed(wi, 0);
	}
}
void DropSiteWindow::keyPressEvent( QKeyEvent* event ) 
{
	QTreeWidgetItem * wi = _prj_tree->currentItem();
	QVariant v = wi->data(0, Qt::UserRole);
	item_obj io = v.value<item_obj>();
	if ( !io.accepkey() ) {
        event->ignore();
		return;
	}

	switch ( event->key() ) {
    case Qt::Key_Insert:
		if ( io.type() == item_obj::ty_project ) {
		    QFileDialog qf;
			QString dirName;
			dirName = qf.getExistingDirectory(this, tr("Directory"), dirName);
			if ( !dirName.isEmpty() ) {
				QFileInfo qf(dirName);
				QString qs = qf.fileName();
				_prj_tree->setHeaderLabel(qs);
				_assign_dropped(dirName);
			}
            return;
        } else if ( io.type() == item_obj::ty_gps ) {
			item_obj io1("Missione", "Premere Ins per aggiungere una base", "", item_obj::ty_missione, true);
            QTreeWidgetItem * qi = new QTreeWidgetItem;
            qi->setText(0, io1.name());
            qi->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io1));
			qi->setFlags(qi->flags() | Qt::ItemIsEditable);
            wi->addChild(qi);
			_prj_tree->editItem(qi, 0);
        } else if ( io.type() == item_obj::ty_missione ) {
			item_obj io1("base", "rinex", "Trascinare\ni file con i dati delle basi\n", item_obj::ty_base, true);
            QTreeWidgetItem * qi = new QTreeWidgetItem;
            qi->setText(0, io1.name());
            qi->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io1));
			qi->setFlags(qi->flags() | Qt::ItemIsEditable);
            wi->addChild(qi);
			_prj_tree->editItem(qi, 0);
		}
        break;
    case Qt::Key_Delete:
		if ( io.type() == item_obj::ty_missione ) {
			// rimuove una missione
			delete wi;
		} else if ( io.type() == item_obj::ty_base ) {
			delete wi;
		} else if ( !io.dropped().isEmpty() ) {
			io.dropped("");
			wi->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
			_drop_area->item_changed(wi, 0);
		}
        break;
    default:
        event->ignore();
        break;
    }
}
