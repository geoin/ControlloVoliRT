/*
    File: cvloader.cpp
    Author:  A.Comparini
    Date: 2013 November 03
    Comment:
        Load files and data for cartography aerial missions to be processed for suitable precision
        and reports creation

    Disclaimer:
        This file is part of CV a framework for verifying aerial missions for cartography.

        CV is free software: you can redistribute it and/or modify
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

#include <iostream>
#include <sqlite3.h>
#include "spatialite.h"
#include "cvloader.h"
#include "CVUtil/cvspatialite.h"
#include <QDir>
#include <QMessageBox>

#define PLANNED_FLIGHT_LAYER_NAME "AVOLOP"
#define FLIGHT_LAYER_NAME "AVOLOV"
#define QUADRO_LAYER_NAME "Quadro_RT"
#define CARTO_LAYER_NAME "carto"
#define CAMERA_FILE "camera.xml"
#define DEM_FILE "dem.asc"
#define ASSETTI_FILE "assettiv.txt"
#define PLANNED_ASSETTI_FILE "assettip.txt"
#define SHAPE_CHAR_SET "CP1252"
#define UTM32_SRID  32632
#define GEOM_COL_NAME "geom"
#define MAX_MSG_LEN 250
#define GEO_DB_NAME "geo.sqlite"

bool _check_file(const QString& nome, bool verbose = true)
{
	QFileInfo qf0(nome);
	if ( !qf0.exists() ) {
		if ( verbose ) {
			QString mes = "Il file " + nome + " non esiste";
			QMessageBox::information(NULL, "Attenzione", mes);
		}
		return false;
	}
	return true;
}
bool _create_dir(const QString& nome, bool verbose = true)
{
	QDir qd(nome);
	bool ret = true;
	if ( !qd.exists() ) {
		bool ret = qd.mkdir(nome);
		if ( verbose && !ret ) {
			QString mes = "Impossibile creare la cartella " + nome;
			QMessageBox::information(NULL, "Attenzione", mes);
		}
	}
	return ret;
}
bool _is_rinex(const QString& ext)
{
	QString e1 = ext.toLower();
	if ( e1.compare("z", Qt::CaseInsensitive) == 0 ) // file z hatanaka
		return true;
	if ( e1.compare("zip", Qt::CaseInsensitive) == 0 ) // regular zip
		return true;
	if ( e1.size() == 3 ) { // file rinex già decompressi
		if ( e1.at(2) == 'o' || e1.at(2) == 'n' )
			return true;
	}
	return false;
}

bool loader::_copy_file(const QString& source, const QString& dest) 
{
	if ( !_check_file(source) )
		return false;

	QFile qdest(dest);
	if ( qdest.exists() ) {
		if ( !_replace )
			return 0;
		if ( !qdest.remove() ) {
			QString mes = "Il file " + dest + " non può essere sovrascritto";
			QMessageBox::information(NULL, "Attenzione", mes);
			return false;
		}
	}

	QFile qsource(source);
	if ( !qsource.copy(dest) ) {
		QString mes = "Il file " + dest + " non può essere creato";
		QMessageBox::information(NULL, "Attenzione", mes);
		return false;

	}

	return true;
}
bool loader::_load_layer(const QString& file_name, const QString& layer_name)
{
	if ( !_check_file(file_name) )
		return false;

    int nrows;
    try {
		QFileInfo qf(_prj_folder, GEO_DB_NAME);
		CV::Util::Spatialite::Connection cnn;
		cnn.open( qf.absoluteFilePath().toStdString() ); // Create or open spatialite db

		//QMessageBox::information(NULL, "Errore", "1");
		//if ( cnn.layer_exists(layer_name.toStdString()) && !_replace )
		//	return false;

		QString dir = QFileInfo(file_name).path();
		QString nome = QFileInfo(file_name).baseName();
		QString shp = QFileInfo(dir, nome).absoluteFilePath();
		//QMessageBox::information(NULL, "Errore", shp.toStdString().c_str());

		nrows = cnn.load_shapefile(shp.toStdString(),
			layer_name.toStdString(),
                           SHAPE_CHAR_SET,
                           UTM32_SRID,
                           GEOM_COL_NAME,
                           true,
                           false,
                           false);
    }
    catch(std::exception const& e) {
		QMessageBox::information(NULL, "Errore", e.what());
    }
    return true;
}

bool loader::load_planned_flight_lines(const QString& nome) 
{
	return _load_layer(nome, PLANNED_FLIGHT_LAYER_NAME);
}
bool loader::load_flight_lines(const QString& nome)
{
	return _load_layer(nome, FLIGHT_LAYER_NAME);
}
bool loader::load_carto(const QString& nome)
{
	return _load_layer(nome, CARTO_LAYER_NAME);
}
bool loader::load_quadro(const QString& nome)
{
	return _load_layer(nome, QUADRO_LAYER_NAME);
}

bool loader::load_planned_assetti(const QString& nome)
{
	QFileInfo qdest(_prj_folder, PLANNED_ASSETTI_FILE);
	return _copy_file(nome, qdest.filePath());
}
bool loader::load_assetti(const QString& nome)
{
	QFileInfo qdest(_prj_folder, ASSETTI_FILE);
	return _copy_file(nome, qdest.filePath());
}
bool loader::load_camera(const QString& nome)
{
	QFileInfo qdest(_prj_folder, CAMERA_FILE);
	return _copy_file(nome, qdest.filePath());
}
bool loader::load_dem(const QString& nome)
{
	QFileInfo qdest(_prj_folder, DEM_FILE);
	return _copy_file(nome, qdest.filePath());
}
bool loader::load_mission(const QString& nome, const QString& item)
{
	QFileInfo qfo(_prj_folder, "missioni");
	QString mis_dir = QFileInfo(_prj_folder, "missioni").filePath();
	if ( !_create_dir(mis_dir) )
		return false;
	mis_dir = QFileInfo(mis_dir, item).filePath();
	if ( !_create_dir(mis_dir) )
		return false;

	QStringList qsl = nome.split(";");
	for ( int i = 0; i < qsl.size(); i++) {
		QString qs1 = qsl[i];
		QFileInfo qf1(qsl[i]);
		QString ext = qf1.suffix();
		if ( !_is_rinex(ext) )
			continue;
		QFileInfo qf2(mis_dir, qf1.fileName());
		_copy_file(qf1.filePath(), qf2.filePath()) ? 1 : 0;
	}
	return true;
}
bool loader::load_base(const QString& nome, const QString& mis_name, const QString& base_name)
{
	QString mis_dir = QFileInfo(_prj_folder, "missioni").filePath();
	if ( !_create_dir(mis_dir) )
		return false;
	mis_dir = QFileInfo(mis_dir, mis_name).filePath();
	if ( !_create_dir(mis_dir) )
		return false;
	mis_dir = QFileInfo(mis_dir, base_name).filePath();
	if ( !_create_dir(mis_dir) )
		return false;

	QStringList qsl = nome.split(";");
	for ( int i = 0; i < qsl.size(); i++) {
		QString qs1 = qsl[i];
		QFileInfo qf1(qsl[i]);
		QString ext = qf1.suffix();
		if ( !_is_rinex(ext) )
			continue;
		QFileInfo qf2(mis_dir, qf1.fileName());
		_copy_file(qf1.filePath(), qf2.filePath()) ? 1 : 0;
	}

	return true;
}
bool loader::create_project()
{
	if ( _prj_folder.isEmpty() ) {
		return false;
		QMessageBox::information(NULL, "Attenzione", "Progetto non impostato");
	}
    try {
		QDir qd(_prj_folder);
		if ( !qd.exists() ) {
			qd.mkdir(_prj_folder);
		}
		QFileInfo qf(_prj_folder, GEO_DB_NAME);
		if ( !qf.exists() ) {
			CV::Util::Spatialite::Connection cnn;
			cnn.create( qf.absoluteFilePath().toStdString() ); // Create or open spatialite db
			if ( cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA )
				cnn.initialize_metdata(); // Initialize metadata (if already initialized noop)
		}
	} catch (std::exception const& e) {
		QMessageBox::information(NULL, "Errore", e.what());
	}
	return true;
}