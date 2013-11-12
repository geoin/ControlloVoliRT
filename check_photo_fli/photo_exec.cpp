/*
    File: photo_exec.cpp
    Author:  F.Flamigni
    Date: 2013 October 29
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
#include "check_photo.h"
#include "dem_interpolate/dsm.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/stringtokenizer.h"
#include "Poco/AutoPtr.h"
#include "Poco/sharedPtr.h"
#include <fstream>
#include <sstream>
#include <spatialite.h>

#define SRID 32632
#define SIGLA_PRJ "CSTP"

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::SharedPtr;
using Poco::Path;

photo_exec::~photo_exec() 
{
	if ( _df != NULL )
		delete _df;
}
bool photo_exec::run()
{
	try {
		// initialize spatial lite connection
		Poco::Path db_path(_proj_dir, "geo.sqlite");
		cnn.create(db_path.toString());
		cnn.initialize_metdata();

		// Read reference values
		_read_ref_val();

		// read camera data
		if ( !_read_cam() )
			return false;
		// read photo position and attitude
		if ( !_read_vdp() )
			return false;
		// read digital terrain model
		if ( !_read_dem() )
			return false;
		// produce photos feature
		if ( !_process_photos() )
			return false;
		// produce models features
		if ( !_process_models() )
			return false;
	}
    catch(std::exception &e) {
        std::cout << std::string(e.what()) << std::endl;
    }
	return true;
}
void photo_exec::set_vdp_name(const std::string& nome)
{
	_vdp_name = nome;
}
void photo_exec::set_dem_name(const std::string& nome)
{
	_dem_name = nome;
}
void photo_exec::set_cam_name(const std::string& nome)
{
	_cam_name = nome;
}
void photo_exec::set_out_folder(const std::string& nome)
{
	_out_folder = nome;
}
void photo_exec::set_proj_dir(const std::string& nome)
{
	_proj_dir = nome;
}
void photo_exec::set_checkType(Check_Type t)
{
	_type = t;
}

bool photo_exec::_read_ref_val()
{
	Path ref_file(_proj_dir, "*");
	ref_file.popDirectory();
	ref_file.setFileName("Regione_Toscana_RefVal.xml");
	AutoPtr<XMLConfiguration> pConf;
	try {
		pConf = new XMLConfiguration(ref_file.toString());
		//_MAX_PDOP = pConf->getDouble(get_key("MAX_PDOP"));
		//_MIN_SAT = pConf->getInt(get_key("MIN_SAT"));
		//_MAX_DIST = pConf->getInt(get_key("MAX_DIST")) * 1000;
		//_MIN_SAT_ANG = pConf->getDouble(get_key("MIN_SAT_ANG"));
		//_NBASI = pConf->getInt(get_key("NBASI"));
		//_MIN_ANG_SOL = pConf->getDouble(get_key("MIN_ANG_SOL"));
	} catch (...) {
		return false;
	}
	return true;
}
bool photo_exec::_read_cam()
{
	AutoPtr<XMLConfiguration> pConf;
	try {
		pConf = new XMLConfiguration(_cam_name);
		_cam.foc = atof(pConf->getString("FOC").c_str());
		_cam.dimx = atof(pConf->getString("DIMX").c_str());
		_cam.dimy = atof(pConf->getString("DIMY").c_str());
		_cam.dpix = atof(pConf->getString("DPIX").c_str());
		_cam.xp = atof(pConf->getString("XP", "0").c_str());
		_cam.yp = atof(pConf->getString("YP", "0").c_str());
	} catch (...) {
		return false;
	}
	return true;
}
bool photo_exec::_read_vdp()
{
	std::ifstream fvdp(_vdp_name.c_str(), std::ifstream::in);
	if ( !fvdp.is_open() )
		return false;

	char buf[256];
	while ( fvdp.getline(buf, 256) ) {
		Poco::StringTokenizer tok(buf, " \t", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
		if ( tok.count() != 7 )
			continue;
		if ( atof(tok[1].c_str()) == 0. )
			continue;
		VDP vdp(_cam, tok[0]);
		vdp.Init(DPOINT(atof(tok[1].c_str()), atof(tok[2].c_str()), atof(tok[3].c_str())), atof(tok[4].c_str()), atof(tok[5].c_str()), atof(tok[6].c_str()));
		_vdps[tok[0]] = vdp;
	}
	return true;

}
bool photo_exec::_read_dem()
{
	_df = new DSM_Factory;
	if ( !_df->Open(_dem_name, false) )
		return false;
	return true;
}
std::string photo_exec::_get_strip(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() != 2 )
		return "";
	return tok[0];
}
std::string photo_exec::_get_nome(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() != 2 )
		return "";
	return tok[1];
}


//void photo_exec::_get_side(QgsGeometry* fv, double* d1, double* d2) 
//{
//	fv->
//	if ( !fv.IsChiusaP() )
//		return;
//	int np = fv.GetCount();
//	int idx = -1;
//	for (int i = 0; i < np - 1; i++) {
//		int j = i + 1;
//		int k = i == 0 ? np - 2 : i - 1;
//		double alfa = fabs(angdir(fv[j].p[0], fv[j].p[1], fv[i].p[0], fv[i].p[1]) - angdir(fv[i].p[0], fv[i].p[1], fv[k].p[0], fv[k].p[1]));
//		if ( fabs(alfa) > M_PI / 4 ) {
//			idx = i;
//			break;
//		}
//	}
//	FVDATA fvp;
//	if ( idx != 0 ) {
//		int kk = idx;
//		do {
//			fvp.Add(&fv[kk++]);
//			if ( kk >= np -1 ) 
//				kk = 0;
//			if ( kk == idx )
//				fvp.Add(&fv[kk]);
//		} while ( kk != idx );
//	} else
//		fvp = fv;
//
//	double dmax = -1.e30;
//	double dmin = 1.e30;
//	double dl = 0.;
//	for (int i = 1; i < fvp.GetCount(); i++) {
//		double d = fvp[i].dist2D(fvp[i - 1]);
//		if ( dl == 0. )
//			dl += d;
//		else {
//			double alfa = fabs(angdir(fvp[i].p[0], fvp[i].p[1], fvp[i - 1].p[0], fvp[i - 1].p[1]) - angdir(fvp[i - 1].p[0], fvp[i - 1].p[1], fvp[i - 2].p[0], fvp[i - 2].p[1]));
//			if ( fabs(alfa) < M_PI / 4 )
//				dl += d;
//			else {
//				dmin = min(dmin, dl);
//				dmax = max(dmax, dl);
//				dl = d;
//			}
//		}
//	}
//	if ( dl != 0. ) {
//		dmin = min(dmin, dl);
//		dmax = max(dmax, dl);
//	}
//	*d1 = dmin;
//	*d2 = dmax;
//}

bool photo_exec::_process_photos()
{
	std::string table(_type == Prj_type ? "Z_FOTOP" : "Z_FOTOV");
	cnn.remove_layer(table);

	DSM* ds = _df->GetDsm();

	//// create the photo table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"( _FOTO_ID TEXT NOT NULL, " <<
		"Z_FOTO_ID TEXT NOT NULL, " <<
		"Z_FOTO_CS TEXT NOT NULL," <<
		"Z_FOTO_NF INTEGER NOT NULL PRIMARY KEY," <<
		"Z_FOTO_DIMPIX DOUBLE NOT NULL, " <<
		"Z_FOTO_PITCH DOUBLE NOT NULL, " <<
		"Z_FOTO_ROLL DOUBLE NOT NULL)";
	cnn.execute_immediate(sql.str());

	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'POLYGON'," <<
		"'XYZ')";
	cnn.execute_immediate(sql1.str());

	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (Z_FOTO_ID, Z_FOTO_CS, Z_FOTO_NF, Z_FOTO_DIMPIX, Z_FOTO_PITCH, Z_FOTO_ROLL, geom) \
		VALUES (?, ?, ?, ?, ?, ?, ?)";

	CV::Util::Spatialite::Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());

	std::map<std::string, VDP>::iterator it;
	for (it = _vdps.begin(); it != _vdps.end(); it++) {
		VDP& vdp = it->second;
		double dt = 0.;

		DPOINT Pc(vdp.Pc.GetX()	, vdp.Pc.GetY(), vdp.Pc.GetZ());
		std::vector<DPOINT> dpol;
		for ( int i = 0; i < 5; i++) {
			Collimation ci;
			ci.xi = ( i == 0 || i == 3 ) ? 0.f : (i != 4) ? (float) vdp.dimx() : (float) (vdp.dimx() / 2.);
			ci.yi = ( i == 0 || i == 1 ) ? 0.f : (i != 4) ? (float) vdp.dimy() : (float) (vdp.dimy() / 2.);
			DPOINT pd, pt;
			vdp.GetRay(ci, &pd);
			if ( !ds->RayIntersect(Pc, pd, pt) ) {
				if ( !ds->IsInside(pt.z) ) {
					break;
				}
			}
			dt += vdp.Pc.GetZ() - pt.z;
			dpol.push_back(pt);
		}

		gaiaGeomCollPtr geo = gaiaAllocGeomColl();
		geo->Srid = SRID;
		geo->DimensionModel  = GAIA_XY_Z;
		geo->DeclaredType = GAIA_POLYGONZ;
		gaiaPolygonPtr polyg = gaiaAddPolygonToGeomColl(geo, 5, 0);
		gaiaRingPtr ring = polyg->Exterior;

		for (int i = 0; i < 4; i++)
			gaiaSetPointXYZ(ring->Coords, i, dpol[i].x, dpol[i].y, dpol[i].z);
		gaiaSetPointXYZ(ring->Coords, 4, dpol[0].x, dpol[0].y, dpol[0].z);

		//calcola del GSD medio
		dt = vdp.pix() * dt / (5 * vdp.foc());

		//sqlite3_reset(stmt);
		//sqlite3_clear_bindings(stmt);

		unsigned char *blob;
		int blob_size;
		gaiaToSpatiaLiteBlobWkb(geo, &blob, &blob_size);
		gaiaFreeGeomColl (geo);

		// we can now destroy the geometry object
		std::string strip = _get_strip(it->first);
		splite.bind(1, SIGLA_PRJ, w_spatialite::TEXT);
		splite.bind(2, strip.c_str(), w_spatialite::TEXT);
		int id = atoi(_get_nome(it->first).c_str());
		splite.bind(3, &id, w_spatialite::INT);
		splite.bind(4, &dt, w_spatialite::DOUBLE);
		double o = RAD_DEG(vdp.om), f = RAD_DEG(vdp.fi);
		splite.bind(5, &o), w_spatialite::DOUBLE);
		splite.bind(6, &f, w_spatialite::DOUBLE);
		splite.bind(7, blob, blob, blob_size);

		//sqlite3_bind_text(stmt, 1, SIGLA_PRJ, strlen(SIGLA_PRJ), SQLITE_STATIC);
		//sqlite3_bind_text(stmt, 2, strip.c_str(), strip.size(), SQLITE_STATIC);
		//sqlite3_bind_int(stmt, 3, atoi(_get_nome(it->first).c_str()));
		//sqlite3_bind_double(stmt, 4, dt);
		//sqlite3_bind_double(stmt, 5, RAD_DEG(vdp.om));
		//sqlite3_bind_double(stmt, 6, RAD_DEG(vdp.fi));
		//sqlite3_bind_blob (stmt, 7, blob, blob_size, SQLITE_STATIC);

		stm.execute();
		
		//if ( ! splite.step() {
		////int retv = sqlite3_step(stmt);
		////if ( retv != SQLITE_DONE && retv != SQLITE_ROW) {
		//	printf ("sqlite3_step() error: %s\n", splite.get_error_msg());
		//      break;
		//}
		gaiaFree(blob);
		//_vfoto.push_back(fet);
	}
	cnn.commit_transaction();
	//sqlite3_finalize(stmt);

	//splite.commit();
	//ret = sqlite3_exec (db_handle, "COMMIT", NULL, NULL, &err_msg);
	//if (ret != SQLITE_OK) {
	//	fprintf (stderr, "Error: %s\n", err_msg);
	//	sqlite3_free (err_msg);
	//}
	//_process_models();
	//_process_strips();
	return true;
}
// costruisce i modelli a partire da fotogrammi consecutivi della stessa strisciata
bool photo_exec::_process_models()
{
	std::string table(_type == Prj_type ? "Z_MODELP" : "Z_MODELV");
	cnn.remove_layer(table);

	DSM* ds = _df->GetDsm();

	// create the model table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_MODEL_ID TEXT NOT NULL, " <<
		"Z_MODEL_CS TEXT NOT NULL, " <<
		"Z_MODEL_LEFT TEXT NOT NULL, " <<
		"Z_MODEL_RIGHT TEXT NOT NULL, " <<
		"Z_MODEL_L_OVERLAP INTEGER NOT NULL, " <<
		"Z_MODEL_T_OVERLAP INTEGER NOT NULL, " <<
		"Z_MODEL_D_HEADING DOUBLE NOT NULL)";
	cnn.execute_immediate(sql.str());

	// aggiunge la colonna geometrica
	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'POLYGON'," <<
		"'XYZ')";
	cnn.execute_immediate(sql1.str());
	
	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (Z_MODEL_ID, Z_MODEL_CS, Z_MODEL_LEFT, Z_MODEL_RIGHT, Z_MODEL_L_OVERLAP, Z_MODEL_T_OVERLAP, Z_MODEL_D_HEADING, geom) \
		VALUES (?, ?, ?, ?, ?, ?, ?)";

	CV::Util::Spatialite::Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());	

	//QgsFieldMap fields;
	//fields[0] = QgsField("STRIP", QVariant::String);
	//fields[1] = QgsField("NOME_LEFT", QVariant::String);
	//fields[2] = QgsField("NOME_RIGHT", QVariant::String);

	//Poco::Path pth(_out_folder, "Models.shp");
	//QgsVectorFileWriter writer(pth.toString().c_str(), "CP1250", fields, QGis::WKBPolygon, 0, "ESRI Shapefile");

	//std::string str0;
	////SharedPtr<QgsGeometry> g0;
	//QgsGeometry* g0 = NULL;
	//std::string f0, f1;
	//// scandisce tutti i fotogrammi
	//for (size_t i = 0; i < _vfoto.size(); i++) {
	//	QgsFeature& fet = _vfoto[i];
	//	//SharedPtr<QgsGeometry> g1(fet.geometry());
	//	QgsGeometry* g1 = fet.geometry();
	//	const QgsAttributeMap& am = fet.attributeMap();
	//	std::string str = am[0].toByteArray();
	//	std::string f1 = am[1].toByteArray();
	//	if ( str != str0 ) {
	//		// inizia una nuova strip
	//		str0 = str;
	//	} else {
	//		QgsFeature ft;
	//		// fa l'intersezione del modello attuale (g1) col precedente (g0)
	//		ft.setGeometry(g1->intersection(g0));
	//		ft.addAttribute(0, QVariant(str0.c_str()));
	//		ft.addAttribute(1, QVariant(f0.c_str()));
	//		ft.addAttribute(2, QVariant(f1.c_str()));
	//		writer.addFeature(ft);
	//	}
	//	g0 = g1;
	//	f0 = f1;
	//}
	return true;
}
// costruisce le strisciate unendo tutte le foto di una stessa strip
bool photo_exec::_process_strips()
{
	//QgsFieldMap fields;
	//fields[0] = QgsField("STRIP", QVariant::String);
	//fields[1] = QgsField("N_IMGS", QVariant::Int);

	//Poco::Path pth(_out_folder, "Strips.shp");
	//QgsVectorFileWriter writer(pth.toString().c_str(), "CP1250", fields, QGis::WKBPolygon, 0, "ESRI Shapefile");

	//std::string str0;
	//int nimg = 0;

	////SharedPtr<QgsGeometry> g0;
	//QgsGeometry* g0;
	//for (size_t i = 0; i < _vfoto.size(); i++) {
	//	QgsFeature& fet = _vfoto[i];
	//	//SharedPtr<QgsGeometry> g1 = fet.geometry();
	//	QgsGeometry* g1 = fet.geometry();
	//	const QgsAttributeMap& am = fet.attributeMap();
	//	std::string str = am[0].toByteArray();
	//	if ( str != str0 ) {
	//		if ( !str0.empty() ) {
	//			QgsFeature ft;
	//			ft.setGeometry(g0);
	//			ft.addAttribute(0, QVariant(str0.c_str()));
	//			ft.addAttribute(1, QVariant(nimg));
	//			writer.addFeature(ft);
	//		}
	//		str0 = str;
	//		nimg = 0;
	//		g0 = g1;
	//	} else {
	//		g0 = g1->combine(g0);
	//	}
	//	nimg++;
	//}
	//if ( nimg ) {
	//	QgsFeature ft;
	//	ft.setGeometry(g0);
	//	ft.addAttribute(0, QVariant(str0.c_str()));
	//	ft.addAttribute(1, QVariant(nimg));
	//	writer.addFeature(ft);
	//}
	return true;

}

