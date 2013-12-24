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
#include "cvloader.h"

DropWindow::DropWindow()
{
	_ld = new loader;
    _drop_area = new DropArea;
    connect(_drop_area, SIGNAL(changed(const QString&)), this, SLOT(_assign_dropped(const QString&)));

	item_obj io;

    _prj_tree = new QTreeWidget;
	_prj_tree->setHeaderLabel("RT");
	_prj_tree->setMouseTracking(true);

	QTreeWidgetItem* qt = new QTreeWidgetItem;
	io = item_obj("Progetto", "folder", "Trascinare\nla cartella del progetto", item_obj::ty_project, true, true);
	qt->setText(0, io.name());
	qt->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	_prj_tree->addTopLevelItem(qt);

	QTreeWidgetItem* q1 = new QTreeWidgetItem;
	io = item_obj("Fotogrammetria", "", "", item_obj::ty_fotogra, false, false);
	q1->setText(0, io.name());
	q1->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));

	QList<QTreeWidgetItem*> qcl;
	QTreeWidgetItem* q2 = new QTreeWidgetItem;
	io = item_obj("Assi di volo progettati", "Shp", "Trascinare\nil file degli assi di volo progettati", item_obj::ty_assi_p);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Assi di volo", "Shp", "Trascinare\nil file degli assi di volo", item_obj::ty_assi);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Aree da cartografare", "Shp", "Trascinare\nil file delle aree da cartografare", item_obj::ty_carto);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	//q2 = new QTreeWidgetItem;
	//io = item_obj("Assetti progettati", "txt", "Trascinare\nil file degli assetti progettati", item_obj::ty_assetti_p);
	//q2->setText(0, io.name());
	//q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	//qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Assetti effettivi", "txt", "Trascinare\nil file degli assetti effettivi", item_obj::ty_assetti);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Dati Fotocamera", "xml", "Trascinare\nil file descrittivo della fotocamera", item_obj::ty_camera);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Modello numerico del terreno", "asc", "Trascinare\nil file del modello numerico del terreno", item_obj::ty_dem);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Dati GPS", "", "Premere ins per aggiungere missioni, del per rimuoverle", item_obj::ty_gps, true, false);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q2 = new QTreeWidgetItem;
	io = item_obj("Quadro unione ortofoto", "Shp", "Trascinare\nil file del quadro d'unione", item_obj::ty_quadro);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);
	
	q2 = new QTreeWidgetItem;
	io = item_obj("Contorno regione", "Shp", "Trascinare\nil file del contorno della regione", item_obj::ty_contorno);
	q2->setText(0, io.name());
	q2->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
	qcl.push_back(q2);

	q1->addChildren(qcl);

	qt->addChild(q1);
	
	q1 = new QTreeWidgetItem;
	q1->setText(0, "Lidar");
	qt->addChild(q1);
	connect(_prj_tree, SIGNAL(itemClicked (QTreeWidgetItem *, int)), _drop_area, SLOT(item_changed(QTreeWidgetItem *, int)));

    execButton = new QPushButton(tr("Esegui"));
    quitButton = new QPushButton(tr("Esci"));
    //QCheckBox* replaceButton = new QCheckBox("Sostituisci");

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(execButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);
    //buttonBox->addButton(replaceButton, QDialogButtonBox::AcceptRole);

    connect(quitButton, SIGNAL(pressed()), this, SLOT(close()));
    connect(execButton, SIGNAL(pressed()), this, SLOT(exec()));

	QSplitter* qs = new QSplitter;
	qs->setOrientation(Qt::Horizontal);
	qs->addWidget(_prj_tree);
	qs->addWidget(_drop_area);

    QVBoxLayout *vl = new QVBoxLayout;
	vl->addWidget(qs);
	vl->addWidget(buttonBox);
    setLayout(vl);

    setWindowTitle(tr("Gestione Progetto"));
    setMinimumSize(350, 500);
}
DropWindow::~DropWindow()
{
	if ( _ld != NULL )
		delete _ld;
	_ld = NULL;
}
void DropWindow::_assign_dropped(const QString& path)
{
	QTreeWidgetItem * wi = _prj_tree->currentItem();
	QVariant v = wi->data(0, Qt::UserRole);
	item_obj io = v.value<item_obj>();
	if ( io.acceptdrop() ) {
		if ( io.data_type().compare("folder", Qt::CaseInsensitive) == 0 ) {
			QFileInfo qd(path);
			if ( qd.exists() && !qd.isDir() ) {
				QMessageBox::information(NULL, "Errore", "Oggetto non valido per la categoria selezionata");
				return;
			}

		} else if ( !io.data_type().isEmpty() ) {
			QFileInfo qd(path);
			if ( io.data_type().compare(qd.suffix(), Qt::CaseInsensitive) != 0 ) { 
				QMessageBox::information(NULL, "Errore", "Oggetto non valido per la categoria selezionata");
				return;
			}
		}
		io.dropped(path);
		wi->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io));
		_drop_area->item_changed(wi, 0);
	}
}
void DropWindow::keyPressEvent( QKeyEvent* event ) 
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
        if ( io.type() == item_obj::ty_gps ) {
			item_obj io1("Missione", "", "Premere Ins per aggiungere una base oppure trascinare i file rinex del rover", item_obj::ty_missione, true);
            QTreeWidgetItem * qi = new QTreeWidgetItem;
            qi->setText(0, io1.name());
            qi->setData(0, Qt::UserRole, QVariant::fromValue<item_obj>(io1));
			qi->setFlags(qi->flags() | Qt::ItemIsEditable);
            wi->addChild(qi);
			_prj_tree->editItem(qi, 0);
        } else if ( io.type() == item_obj::ty_missione ) {
			item_obj io1("base", "", "Trascinare\ni file con i dati delle basi\n", item_obj::ty_base, true);
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
void DropWindow::_item_manager(const item_obj& io, QTreeWidgetItem* w0)
{
	if ( io.dropped().isEmpty() )
		return;
	QString qs;
	QFileInfo qf(io.dropped());

	switch ( io.type() ) {
		case item_obj::ty_project: // load the project folder
			_ld->prj_folder(io.dropped());
			_ld->create_project();
			qs = QString("Creato GEO DB ") + GEO_DB_NAME + QString(" in \n") + io.dropped() + "\n";
			break;
		case item_obj::ty_fotogra: // no action
			break;
		case item_obj::ty_lidar: // no action
			break;
		case item_obj::ty_assi_p: {// load planned flight  lines
			qs = QString("Caricato ") + qf.baseName() + QString(" come layer ") + PLANNED_FLIGHT_LAYER_NAME + "\n";
			_ld->load_planned_flight_lines(io.dropped());
			break;
		}
		case item_obj::ty_assi: {// load flight lines
			qs = QString("Caricato ") + qf.baseName() + QString(" come layer ") + FLIGHT_LAYER_NAME + "\n";
			_ld->load_flight_lines(io.dropped());
			break;
		}
		case item_obj::ty_carto: {// load areas to collect
			qs = QString("Caricato ") + qf.baseName() + QString(" come layer ") + CARTO_LAYER_NAME + "\n";
			_ld->load_carto(io.dropped());
			break;
		}
		//case item_obj::ty_assetti_p: {// load planned photo parameters
		//	qs = QString("Copiato ") + qf.baseName() + QString(" come ") + PLANNED_ASSETTI_FILE + "\n";
		//	_ld->load_planned_assetti(io.dropped());
		//	break;
		case item_obj::ty_assetti: { // load real photo parameters
			qs = QString("Copiato ") + io.dropped() + QString(" come ") + ASSETTI_FILE + "\n";
			_ld->load_assetti(io.dropped());
			break;
		}
		case item_obj::ty_camera: { // load camera file
			qs = QString("Copiato ") + io.dropped() + QString(" come ") + CAMERA_FILE + "\n";
			_ld->load_camera(io.dropped());
			break;
		}
		case item_obj::ty_dem: { // load digital terrain model
			qs = QString("Copiato ") + qf.baseName() + QString(" come ") + DEM_FILE + "\n";
			_ld->load_dem(io.dropped());
			break;
		}
		case item_obj::ty_quadro: { // load union table for orthos
			qs = QString("Caricato ") + qf.baseName() + QString(" come layer ") + QUADRO_LAYER_NAME + "\n";
			_ld->load_quadro(io.dropped());
			break;
		}
		case item_obj::ty_contorno: { // load region border for orthos
			qs = QString("Caricato ") + qf.baseName() + QString(" come layer ") + CONTORNO_RT + "\n";
			_ld->load_contorno(io.dropped());
			break;
		}
		case item_obj::ty_gps: // no action
			break;
		case item_obj::ty_missione: { // load mission data
			QString mis_name = w0->text(0); // nome della missione
			_ld->load_mission(io.dropped(), mis_name);
			qs = QString("Copiato ") + io.dropped() + QString(" in ") + mis_name + "\n";

			break;
		}
		case item_obj::ty_base: { // load base data refferred to a mission
			QString base_name = w0->text(0); // nome della base
			QString mis_name = w0->parent()->text(0); // nome della missione
			_ld->load_base(io.dropped(), mis_name, base_name);
			qs = QString("Copiati dati base ") + base_name + QString(" in ") + mis_name + "\n";
			break;
		}
	}
	if ( !qs.isEmpty() )
		_drop_area->append(qs);

}
void DropWindow::_child_manager(QTreeWidgetItem * w0)
{
	// gestisce se stesso
	QVariant v = w0->data(0, Qt::UserRole);
	//QString qs = w0->text(0);
	item_obj io = v.value<item_obj>();
	_item_manager(io, w0);

	int nc = w0->childCount();
	// gestisci i chil node
	for ( int i = 0; i < nc; i++) {
		QTreeWidgetItem * wi = w0->child(i);
		_child_manager(wi);
	}
}
void DropWindow::exec()
{
	QTreeWidgetItem * w0 =_prj_tree->topLevelItem(0);
	if ( w0 == NULL )
		return;

	_drop_area->clean();

	// gestisce se stesso
	QVariant v = w0->data(0, Qt::UserRole);
	//QString qs = w0->text(0);
	item_obj io = v.value<item_obj>();
	_item_manager(io, w0);

	int nc = w0->childCount();
	for ( int i = 0; i < nc; i++) {
		QTreeWidgetItem * wi = w0->child(i);
		_child_manager(wi);
	}
	_drop_area->print();
}
