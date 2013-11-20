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
#include "gdal/ogr_geometry.h"

#define SRID 32632
#define SIGLA_PRJ "CSTP"
#define CARTO "CARTO"
#define SHAPE_CHAR_SET "CP1252"

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

		int nrows = cnn.load_shapefile("C:/Google_drive/Regione Toscana Tools/Dati_test/Carto/zona2castpescaia",
		   CARTO,
		   SHAPE_CHAR_SET,
		   SRID,
		   "geom",
		   true,
		   false,
		   false);
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
			return false;	
		if ( !_process_block() )
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
OGRGeometry* photo_exec::GetGeom(CV::Util::Spatialite::QueryField& rs)
{
	std::vector<unsigned char> blob;
	rs.toBlob(blob);
	OGRGeometryFactory gf;
	OGRGeometry* pol;
	int ret = gf.createFromWkb( (unsigned char *)&blob[0], NULL, &pol);
	return pol;
}
void photo_exec::SetGeom(CV::Util::Spatialite::BindField& bf, const OGRGeometry* og)
{
	static std::vector<unsigned char> buffin;
	int size_in = og->WkbSize();
	buffin.resize(size_in);
	og->exportToWkb(wkbNDR, &buffin[0]);
	bf.fromBlob(buffin);
}

OGRGeometry* photo_exec::_get_carto() 
{
	std::string table(CARTO);

	std::stringstream sql;
	sql << "select AsBinary(geom) from " << table;
	CV::Util::Spatialite::Statement stm(cnn);
	stm.prepare(sql.str());
	CV::Util::Spatialite::Recordset rs = stm.recordset();
	
	bool first = true;
	OGRGeometry* blk;

	while ( !rs.eof() ) { //for every strip
		if ( first ) {
			first = false;
			blk = GetGeom(rs[0]);
		} else {
			OGRGeometry* pol2 = GetGeom(rs[0]);
			OGRGeometry* pol1 = blk->Union(pol2);
			OGRGeometryFactory::destroyGeometry(blk);
			OGRGeometryFactory::destroyGeometry(pol2);
			blk = pol1;
		}
		rs.next();
	}
	return blk;
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

void photo_exec::_get_elong(OGRPolygon* fv, double ka, double* d1, double* d2) 
{
	OGRPoint po;
	if ( fv->Centroid(&po) != OGRERR_NONE )
		return;
	OGRLinearRing* or = (OGRLinearRing*) fv->getExteriorRing();
	double xm = 1.e20, ym = 1.e20;
	double xM = -1.e20, yM = -1.e20;

	for (int i = 0; i < or->getNumPoints(); i++) {
		double x = or->getX(i) - po.getX();
		double y = or->getY(i) - po.getY();
		double x1 = x * cos(ka) + y * sin(ka);
		double y1 = -x * sin(ka) + y * cos(ka);
		xm = min(xm, x1);
		ym = min(ym, y1);
		xM = max(xM, x1);
		yM = max(yM, y1);
	}
	double l1 = fabs(xM - xm);
	double l2 = fabs(yM - ym);
	if ( l1 > l2 ) {
		*d1 = l2;
		*d2 = l1;
	} else {
		*d1 = l1;
		*d2 = l2;
	}
}

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

	// add the geom column
	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'POLYGON'," <<
		"'XY')";
	cnn.execute_immediate(sql1.str());

	// create the insertion query
	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (Z_FOTO_ID, Z_FOTO_CS, Z_FOTO_NF, Z_FOTO_DIMPIX, Z_FOTO_PITCH, Z_FOTO_ROLL, geom) \
		VALUES (?1, ?2, ?3, ?4, ?5, ?6, ST_GeomFromWKB(:geom, " << SRID << ") )";

	CV::Util::Spatialite::Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());

	std::map<std::string, VDP>::iterator it;
	for (it = _vdps.begin(); it != _vdps.end(); it++) {
		VDP& vdp = it->second;
		double dt = 0.;

		DPOINT Pc(vdp.Pc);
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
			dt += vdp.Pc.z - pt.z;
			dpol.push_back(pt);
		}

		OGRSpatialReference sr;
		sr.importFromEPSG(SRID);

	    OGRGeometryFactory gf;
		OGRLinearRing* gp = (OGRLinearRing*) OGRGeometryFactory::createGeometry(wkbLinearRing);
		gp->setCoordinateDimension(2);
		gp->assignSpatialReference(&sr);

		for (int i = 0; i < 4; i++)
			gp->addPoint(dpol[i].x, dpol[i].y);
		gp->closeRings();

		OGRPolygon* pol  = (OGRPolygon*) gf.createGeometry(wkbPolygon);
		pol->setCoordinateDimension(2);
		pol->assignSpatialReference(&sr);
		pol->addRing(gp);

		// calculate mean GSD
		dt = vdp.pix() * dt / (5 * vdp.foc());

		stm[1] = SIGLA_PRJ;
		stm[2] = _get_strip(it->first);
		stm[3] = it->first;
		stm[4] = dt;
		stm[5] = RAD_DEG(vdp.om);
		stm[6] = RAD_DEG(vdp.fi);
		SetGeom(stm[7], pol);
		//stm[7].fromBlob(buffin);

		stm.execute();
		stm.reset();
		
		OGRGeometryFactory::destroyGeometry(gp);
		OGRGeometryFactory::destroyGeometry(pol);
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
		"'XY')";
	cnn.execute_immediate(sql1.str());
	
	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (Z_MODEL_ID, Z_MODEL_CS, Z_MODEL_LEFT, Z_MODEL_RIGHT, Z_MODEL_L_OVERLAP, Z_MODEL_T_OVERLAP, Z_MODEL_D_HEADING, geom) \
		VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ST_GeomFromWKB(:geom, " << SRID << ") )";
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
		sql4 << "select Z_FOTO_ID, Z_FOTO_CS, Z_FOTO_NF, Z_FOTO_DIMPIX, Z_FOTO_PITCH, Z_FOTO_ROLL, AsBinary(geom) from " << tablef << " where Z_FOTO_CS=" << strip;
		CV::Util::Spatialite::Statement stm2(cnn);
		stm2.prepare(sql4.str());
		CV::Util::Spatialite::Recordset rs1 = stm2.recordset();
		std::string nomeleft;
		std::string id;
		OGRGeometry* pol1;
		double lo = 60, to = 95, dh = 1;
		double first = true;
		while ( !rs1.eof() ) {
			if ( first ) {
				first = false;
				id = rs1["Z_FOTO_ID"].toString();
				strip = rs1["Z_FOTO_CS"].toString();
				nomeleft = rs1["Z_FOTO_NF"].toString();

				// Get the first photo geometry
				pol1 = GetGeom(rs1[6]);
			} else {
				std::string nomeright = rs1["Z_FOTO_NF"].toString();

				// get the next photo geometry
				OGRGeometry* pol2 = GetGeom(rs1[6]);
				// the model is the intersection of two photos
				OGRGeometry* mod = pol1->Intersection(pol2);
		
				double dh = _vdps[nomeleft].ka - _vdps[nomeright].ka;
				if ( dh > M_PI ) {
					dh -= 2 * M_PI;
				} else if ( dh < -M_PI ) {
					dh += 2 * M_PI;
				}
				//dh = RAD_DEG(dh);
				dh = (int)(RAD_DEG(dh) * 1000.) / 1000.;
				//dh /= 1000.;

				double d1f, d2f, d1m, d2m;
				_get_elong((OGRPolygon*) pol1, _vdps[nomeleft].ka, &d1f, &d2f);
				_get_elong((OGRPolygon*) mod, _vdps[nomeleft].ka, &d1m, &d2m);
				lo = 100 * d1m / d1f; // longitudinal overlap
				to = 100 * d2m / d2f; // trasversal overlap

				// calcola lo e to
				// calcola dh
				stm[1] = id;
				stm[2] = strip;
				stm[3] = nomeleft;
				stm[4] = nomeright;
				stm[5] = (int) lo;
				stm[6] = (int) to;
				stm[7] = dh;
				SetGeom(stm[8], mod);
				stm.execute();
				stm.reset();
				nomeleft = nomeright;
				OGRGeometryFactory::destroyGeometry(pol1);
				OGRGeometryFactory::destroyGeometry(mod);
				pol1 = pol2;

			}
            rs1.next();
        }
		OGRGeometryFactory::destroyGeometry(pol1);
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
		"Z_STRIP_CS TEXT NOT NULL, " <<
		"Z_STRIP_FIRST TEXT NOT NULL, " <<
		"Z_STRIP_LAST TEXT NOT NULL, "  // overlap trasversale
		"Z_STRIP_LENGTH DOUBLE NOT NULL)";  // strip length
	cnn.execute_immediate(sql.str());

	// add the geometry column
	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'POLYGON'," <<
		"'XY')";
	cnn.execute_immediate(sql1.str());
	
	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (Z_STRIP_ID, Z_STRIP_CS, Z_STRIP_FIRST, Z_STRIP_LAST, Z_STRIP_LENGTH, geom) \
		VALUES (?1, ?2, ?3, ?4, ?5, ST_GeomFromWKB(:geom, " << SRID << ") )";
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

		sql4 << "select Z_MODEL_ID, Z_MODEL_CS, Z_MODEL_LEFT, Z_MODEL_RIGHT, AsBinary(geom) from " << tablef << " where Z_MODEL_CS=" << strip;
		CV::Util::Spatialite::Statement stm2(cnn);
		stm2.prepare(sql4.str());
		CV::Util::Spatialite::Recordset rs1 = stm2.recordset();
		std::string id;
		OGRGeometry* pol1;
		std::string firstname, lastname;
		bool first = true;
		while ( !rs1.eof() ) {
			if ( first ) {
				id = rs1["Z_MODEL_ID"].toString();
				strip = rs1["Z_MODEL_CS"].toString();
				firstname = rs1["Z_MODEL_LEFT"].toString();
				first = false;

				pol1 = GetGeom(rs1[4]);
			} else {
				lastname = rs1["Z_MODEL_RIGHT"].toString();
				// unisce tutti i modelli
				OGRGeometry* pol2 = GetGeom(rs1[4]);
				OGRGeometry* mod = pol1->Union(pol2);
				OGRGeometryFactory::destroyGeometry(pol1);
				OGRGeometryFactory::destroyGeometry(pol2);
				pol1 = mod;
			}
			rs1.next();
		}
		double l = _vdps[firstname].Pc.dist2D(_vdps[lastname].Pc) / 1000.; // strip length in km
		stm[1] = id;
		stm[2] = strip;
		stm[3] = firstname;
		stm[4] = lastname;
		stm[5] = l;
		SetGeom(stm[6], pol1);
		stm.execute();
		stm.reset();
		
		rs.next();
	}
	cnn.commit_transaction();
	return true;
}
bool photo_exec::_process_block()
{
	std::string table(_type == Prj_type ? "Z_BLOCKP" : "Z_BLOCKV");
	cnn.remove_layer(table);

	// create the strip table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_STRIP_ID TEXT NOT NULL, " <<	// sigla del lavoro
		"Z_STRIP1 TEXT NOT NULL, " <<
		"Z_STRIP2 TEXT NOT NULL, " <<
		"Z_STRIP_T_OVERLAP INT NOT NULL)";  // overlap trasversale
	cnn.execute_immediate(sql.str());

	std::stringstream sql3;
	std::string tablef(_type == Prj_type ? "Z_STRIPP" : "Z_STRIPV");
	
	sql3 << "select Z_STRIP_CS, Z_STRIP_FIRST, Z_STRIP_LAST, AsBinary(geom) from " << tablef << " order by Z_STRIP_CS";
	CV::Util::Spatialite::Statement stm1(cnn);
	stm1.prepare(sql3.str());
	CV::Util::Spatialite::Recordset rs = stm1.recordset();
	
	typedef struct strp {
		std::string strip;
		std::string first;
		std::string last;
	};
	std::vector<strp> vs;
	bool first = true;
	OGRGeometry* blk;

	while ( !rs.eof() ) { //for every strip
		std::string s1 = rs[0].toString();
		strp s;
		s.strip = rs[0].toString();
		s.first = rs[1].toString();
		s.last = rs[2].toString();
		vs.push_back(s);
		if ( first ) {
			first = false;
			std::vector<unsigned char> blob;
			rs[3].toBlob(blob);
			OGRGeometryFactory gf;
		    int ret = gf.createFromWkb( (unsigned char *)&blob[0], NULL, &blk);
		} else {
			std::vector<unsigned char> blob;
			rs[3].toBlob(blob);
			OGRGeometryFactory gf;
			OGRGeometry* pol2;
		    int ret = gf.createFromWkb( (unsigned char *)&blob[0], NULL, &pol2);
			OGRGeometry* pol1 = blk->Union(pol2);
			OGRGeometryFactory::destroyGeometry(blk);
			OGRGeometryFactory::destroyGeometry(pol2);
			blk = pol1;
		}
		rs.next();
	}

	OGRGeometry* carto = _get_carto();
	OGRPolygon* inter = (OGRPolygon*) carto->Intersection(blk);
	double a1 = carto->get_Area();
	double a2 = inter->get_Area();

	for (size_t i = 0; i < vs.size(); i++) {
		VDP& vdp1 = _vdps[vs[i].first];
		VDP& vdp2 = _vdps[vs[i].last];
		double k1 = vdp2.Pc.angdir(vdp1.Pc);
		for (size_t j = i + 1; i < vs.size(); i++) {
			VDP& vdp3 = _vdps[vs[j].first];
			VDP& vdp4 = _vdps[vs[j].last];
			double k2 = vdp4.Pc.angdir(vdp3.Pc);
			double dk = fabs(RAD_DEG(k2 - k1));
			int  a = 1;
			//if ( dk < 5 
		}
	}
	return true;
}
