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
		if ( !_process_strips() )
			return false;	}
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

	// create the photo table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_FOTO_ID TEXT NOT NULL, " <<		// project short name
		"Z_FOTO_CS TEXT NOT NULL," <<		//  strip name
		"Z_FOTO_NF TEXT NOT NULL PRIMARY KEY," << // photo name
		"Z_FOTO_DIMPIX DOUBLE NOT NULL, " <<	// GSD pixel size
		"Z_FOTO_PITCH DOUBLE NOT NULL, " <<		// pitch
		"Z_FOTO_ROLL DOUBLE NOT NULL)";			// roll
	cnn.execute_immediate(sql.str());

	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'POLYGON'," <<
		"'XYZ')";
	cnn.execute_immediate(sql1.str());

	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (Z_FOTO_ID, Z_FOTO_CS, Z_FOTO_NF, Z_FOTO_DIMPIX, Z_FOTO_PITCH, Z_FOTO_ROLL) \
		VALUES (?1, ?2, ?3, ?4, ?5, ?6)";

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

		// calculate mean GSD
		dt = vdp.pix() * dt / (5 * vdp.foc());

		unsigned char *blob;
		int blob_size;
		gaiaToSpatiaLiteBlobWkb(geo, &blob, &blob_size);
		gaiaFreeGeomColl (geo);

		stm[1] = SIGLA_PRJ;
		stm[2] = _get_strip(it->first);
		stm[3] =it->first;
		stm[4] = dt;
		stm[5] = vdp.om;
		stm[6] = vdp.fi;
		//stm[7] = blob;

		stm.execute();
		stm.reset();
		
		gaiaFree(blob);
		//_vfoto.push_back(fet);
	}
	cnn.commit_transaction();
	return true;
}
// costruisce i modelli a partire da fotogrammi consecutivi della stessa strisciata
bool photo_exec::_process_models()
{
	std::string table(_type == Prj_type ? "Z_MODELP" : "Z_MODELV");
	cnn.remove_layer(table);

	// create the model table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_MODEL_ID TEXT NOT NULL, " <<	// sigla del lavoro
		"Z_MODEL_CS TEXT NOT NULL, " <<		// strisciata
		"Z_MODEL_LEFT TEXT NOT NULL, " <<	// nome foto sinistra
		"Z_MODEL_RIGHT TEXT NOT NULL, " <<	// nome foto destra
		"Z_MODEL_L_OVERLAP INTEGER NOT NULL, " <<	// overlap longitudinale
		"Z_MODEL_T_OVERLAP INTEGER NOT NULL, " <<	// overlap trasversale
		"Z_MODEL_D_HEADING DOUBLE NOT NULL)";		// differenza di heading tra i fotogrammi
	cnn.execute_immediate(sql.str());

	// add the geometry column
	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'POLYGON'," <<
		"'XYZ')";
	cnn.execute_immediate(sql1.str());
	
	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (Z_MODEL_ID, Z_MODEL_CS, Z_MODEL_LEFT, Z_MODEL_RIGHT, Z_MODEL_L_OVERLAP, Z_MODEL_T_OVERLAP, Z_MODEL_D_HEADING) \
		VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7)";
	CV::Util::Spatialite::Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());

	std::stringstream sql3;
	std::string tablef(_type == Prj_type ? "Z_FOTOP" : "Z_FOTOV");
	sql3 << "select distinct Z_FOTO_CS from " << tablef << " order by Z_FOTO_CS";

	CV::Util::Spatialite::Statement stm1(cnn);
	stm1.prepare(sql3.str());
	CV::Util::Spatialite::Recordset rs = stm1.recordset();
	
	while ( !rs.eof() ) { //for every strip
		std::string strip = rs[0].toString();
		std::stringstream sql4;
		sql4 << "select * from " << tablef << " where Z_FOTO_CS=" << strip;
		CV::Util::Spatialite::Statement stm2(cnn);
		stm2.prepare(sql4.str());
		CV::Util::Spatialite::Recordset rs1 = stm2.recordset();
		std::string nomeleft;
		std::string id;
		double lo = 60, to = 95, dh = 1;
		double first = true;
		while ( !rs1.eof() ) {
			if ( first ) {
				first = false;
				id = rs1["Z_FOTO_ID"].toString();
				strip = rs1["Z_FOTO_CS"].toString();
				nomeleft = rs1["Z_FOTO_NF"].toString();
				// acquisisce e memorizza la geom
			} else {
				std::string nomeright = rs1["Z_FOTO_NF"].toString();
				// acquisisce la geom
				// esegue l'intersezione
				// calcola lo e to
				// calcola dh
				stm[1] = id;
				stm[2] = strip;
				stm[3] = nomeleft;
				stm[4] = nomeright;
				stm[5] = lo;
				stm[6] = to;
				stm[7] = dh;
				stm.execute();
				stm.reset();
				nomeleft = nomeright;
			}
            rs1.next();
        }
		rs.next();
	}
	cnn.commit_transaction();
	return true;
}
// costruisce le strisciate unendo tutte le foto di una stessa strip
bool photo_exec::_process_strips()
{
	std::string table(_type == Prj_type ? "Z_STRIPP" : "Z_STRIPV");
	cnn.remove_layer(table);

	// create the strip table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_STRIP_ID TEXT NOT NULL, " <<	// sigla del lavoro
		"Z_STRIP_CS TEXT NOT NULL, " <<		// strisciata
		"Z_STRIP_T_OVERLAP INTEGER)";  // overlap trasversale
	cnn.execute_immediate(sql.str());

	// add the geometry column
	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'POLYGON'," <<
		"'XYZ')";
	cnn.execute_immediate(sql1.str());
	
	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (Z_STRIP_ID, Z_STRIP_CS) \
		VALUES (?1, ?2)";
	CV::Util::Spatialite::Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());

	std::stringstream sql3;
	std::string tablef(_type == Prj_type ? "Z_MODELP" : "Z_MODELV");
	
	sql3 << "select distinct Z_MODEL_CS from " << tablef << " order by Z_MODEL_CS";
	CV::Util::Spatialite::Statement stm1(cnn);
	stm1.prepare(sql3.str());
	CV::Util::Spatialite::Recordset rs = stm1.recordset();
	
	while ( !rs.eof() ) { //for every strip
		std::string strip = rs[0].toString();
		std::stringstream sql4;
		sql4 << "select * from " << tablef << " where Z_MODEL_CS=" << strip;
		CV::Util::Spatialite::Statement stm2(cnn);
		stm2.prepare(sql4.str());
		CV::Util::Spatialite::Recordset rs1 = stm2.recordset();
		std::string id;
		bool first = true;
		while ( !rs1.eof() ) {
			if ( first ) {
				id = rs1["Z_MODEL_ID"].toString();
				strip = rs1["Z_MODEL_CS"].toString();
				first = false;
			} else {
				// unisce tutti i modelli
				;
			}
			rs1.next();
		}
		stm[1] = id;
		stm[2] = strip;
		stm.execute();
		stm.reset();
		
		rs.next();
	}
	cnn.commit_transaction();
	return true;
}

