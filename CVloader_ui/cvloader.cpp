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
#include "CVUtil/cvspatialite.h"
#include <QDir>
#include <QMessageBox>
#include "cvloader.h"
#include <fstream>
#include <sstream>
#include "CVUtil/ogrgeomptr.h"

using namespace CV::Util::Spatialite;
using namespace CV::Util::Geometry;

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
bool loader::load_contorno(const QString& nome)
{
	return _load_layer(nome, CONTORNO_RT);
}
bool loader::load_planned_assetti(const QString& nome)
{
	QFileInfo qdest(_prj_folder, PLANNED_ASSETTI_FILE);
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

bool loader::_read_vdp(const std::string& nome, std::vector<VDP>& vdps)
{
	std::ifstream fvdp(nome.c_str(), std::ifstream::in);
	if ( !fvdp.is_open() )
		return false;
	Camera cam;


	char buf[256];
	while ( fvdp.getline(buf, 256) ) {
		QString qs(buf);
		QStringList qsl = QString(buf).split(QRegExp("\\s+"), QString::SkipEmptyParts);
		if ( qsl.size() != 7 )
			continue;
		if ( qsl[1].toDouble() == 0. ) 
			continue;
		VDP vdp(cam, qsl[0].toStdString());
		vdp.Init(DPOINT(qsl[1].toDouble(), qsl[2].toDouble(), qsl[3].toDouble()), qsl[4].toDouble(), qsl[5].toDouble(), qsl[6].toDouble());
		vdps.push_back(vdp);
	}
	return true;

}
bool loader::load_assetti(const QString& nome)
{
	std::vector<VDP> vdps;
	if ( !_read_vdp(nome.toStdString(), vdps) )
		return false;

	try {
		QFileInfo qf(_prj_folder, GEO_DB_NAME);
		CV::Util::Spatialite::Connection cnn;
		cnn.open( qf.absoluteFilePath().toStdString() ); // Create or open spatialite db
		cnn.remove_layer(ASSETTIV);

		std::stringstream sql;
		sql << "CREATE TABLE " << ASSETTIV << 
			" (foto TEXT NOT NULL PRIMARY KEY, " << //id della stazione
			"Xc FLOAT NOT NULL, " <<
			"Yc FLOAT NOT NULL, " <<
			"Zc FLOAT NOT NULL, " <<
			"omega FLOAT NOT NULL, " <<
			"fi FLOAT NOT NULL, " <<
			"kappa FLOAT NOT NULL)";			// base name
		cnn.execute_immediate(sql.str());
		std::stringstream sql1;
		sql1 << "SELECT AddGeometryColumn('" << ASSETTIV << "'," <<
			"'geom'," <<
			UTM32_SRID << "," <<
			"'POINT'," <<
			"'XY')";
		cnn.execute_immediate(sql1.str());

		std::stringstream sql2;
		sql2 << "INSERT INTO " << ASSETTIV << " (foto, Xc, Yc, Zc, omega, fi, kappa, geom) \
			VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ST_GeomFromWKB(:geom, " << UTM32_SRID << ") )";
		
		CV::Util::Spatialite::Statement stm(cnn);
		cnn.begin_transaction();
		stm.prepare(sql2.str());	

		OGRSpatialReference sr;
		sr.importFromEPSG(UTM32_SRID);
		
		for ( size_t i = 0; i < vdps.size(); i++) {

			OGRGeometryFactory gf;
			OGRGeomPtr gp_ = gf.createGeometry(wkbPoint);
			gp_->setCoordinateDimension(2);
			gp_->assignSpatialReference(&sr);
			OGRPoint* gp = (OGRPoint*) ((OGRGeometry*) gp_);
			gp->setX(vdps[i].Pc.x); gp->setY(vdps[i].Pc.y);

			stm[1] = vdps[i].nome;
			stm[2] = vdps[i].Pc.x;
			stm[3] = vdps[i].Pc.y;
			stm[4] = vdps[i].Pc.z;
			stm[5] = RAD_DEG(vdps[i].om);
			stm[6] = RAD_DEG(-vdps[i].fi);
			stm[7] = RAD_DEG(-vdps[i].ka);
			
			stm[8].fromBlob(gp_); 
			stm.execute();
			stm.reset();
		}
		cnn.commit_transaction();
	} 
    catch(std::exception &e) {
		std::string ss = e.what();
        std::cout << std::string(e.what()) << std::endl;
		return false;
    }


	return true;
}
