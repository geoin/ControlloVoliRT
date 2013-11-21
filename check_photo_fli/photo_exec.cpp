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
#include "gdal/ogr_geometry.h"

#define SRID 32632
#define SIGLA_PRJ "CSTP"
#define CARTO "CARTO"
#define SHAPE_CHAR_SET "CP1252"
#define REFSCALE "RefScale_2000"

#define Z_FOTO "Z_FOTO"
#define Z_MODEL "Z_MODEL"
#define Z_STRIP "Z_STRIP"
#define Z_BLOCK "Z_BLOCK"

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::SharedPtr;
using Poco::Path;
using namespace CV::Util::Spatialite;
using namespace CV::Util::Geometry;

/**************************************************************/
enum CHECK_TYPE {
	less_ty = 0,
	great_ty = 1,
	abs_less_ty = 2,
	between_ty =3
};
bool print_item(Doc_Item& row, Poco::XML::AttributesImpl& attr, double val, CHECK_TYPE ty, double tol1, double tol2 = 0)
{
	bool rv = true;
	switch ( ty ) {
		case less_ty:
			rv = val < tol1;
			break;
		case great_ty:
			rv = val > tol1;
			break;
		case abs_less_ty:
			rv = fabs(val) < tol1;
			break;
		case between_ty:
			rv = val > tol1 && val < tol2;
			break;
	}
	if ( !rv ) {
		Doc_Item r = row->add_item("entry", attr);
		r->add_instr("dbfo", "bgcolor=\"red\"");
		r->append(val);
	} else
		row->add_item("entry", attr)->append(val);
	return rv;
}
std::string get_key(const std::string& val)
{
	return std::string(REFSCALE) + "." + val;
}
OGRGeomPtr GetGeom(QueryField& rs)
{
	std::vector<unsigned char> blob;
	rs.toBlob(blob);
	OGRGeomPtr og(blob);
	return og;
}
std::string get_strip(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() != 2 )
		return "";
	return tok[0];
}
std::string get_nome(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() != 2 )
		return "";
	return tok[1];
}

/**************************************************************/

photo_exec::~photo_exec() 
{
	if ( _df != NULL )
		delete _df;
}
void photo_exec::_init_document()
{
	Path doc_file(_proj_dir, "*");
	doc_file.setFileName(_type == fli_type ? "check_photoV.xml" : "check_photoP.xml");
	_dbook.set_name(doc_file.toString());	

	_article = _dbook.add_item("article");
	_article->add_item("title")->append(_type == fli_type ? "Collaudo ripresa aerofotogrammetrica" : "Collaudo progetto di ripresa aerofotogrammetrica");
}
bool photo_exec::run()
{
	try {
		// initialize spatial lite connection
		Poco::Path db_path(_proj_dir, "geo.sqlite");
		cnn.create(db_path.toString());
		cnn.initialize_metdata();

		/*int nrows = cnn.load_shapefile("C:/Google_drive/Regione Toscana Tools/Dati_test/Carto/zona2castpescaia",
		   CARTO,
		   SHAPE_CHAR_SET,
		   SRID,
		   "geom",
		   true,
		   false,
		   false);*/
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

		// initialize docbook xml file
		_init_document();

		// produce photos feature
#ifdef ppp
		if ( !_process_photos() )
			return false;
		// produce models features
		if ( !_process_models() )
			return false;
		if ( !_process_strips() )
			return false;	
		if ( !_process_block() )
			return false;
#endif
		_final_report();

		// write the result on the docbook report
		_dbook.write();
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
		_GSD = pConf->getDouble(get_key("GSD"));
		_MAX_GSD = pConf->getDouble(get_key("MAX_GSD"));
		_MODEL_OVERLAP = pConf->getDouble(get_key("MODEL_OVERLAP"));
		_MODEL_OVERLAP_RANGE = pConf->getDouble(get_key("MODEL_OVERLAP_RANGE"));
		_MODEL_OVERLAP_T = pConf->getDouble(get_key("MODEL_OVERLAP_T"));
		_STRIP_OVERLAP = pConf->getDouble(get_key("STRIP_OVERLAP"));
		_STRIP_OVERLAP_RANGE = pConf->getDouble(get_key("STRIP_OVERLAP_RANGE"));
		_MAX_STRIP_LENGTH = pConf->getDouble(get_key("MAX_STRIP_LENGTH"));
		_MAX_HEADING_DIFF = pConf->getDouble(get_key("MAX_HEADING_DIFF"));
		_MAX_ANG = pConf->getDouble(get_key("MAX_ANG"));
	} catch (...) {
		return false;
	}
	return true;
}

bool photo_exec::_get_carto(OGRGeomPtr strips) 
{
	std::string table(CARTO);

	std::stringstream sql;
	sql << "select AsBinary(geom) from " << table;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	
	bool first = true;
	OGRGeomPtr blk;

	while ( !rs.eof() ) { //for every strip
		if ( first ) {
			first = false;
			blk = GetGeom(rs[0]);
		} else {
			OGRGeomPtr pol2 = GetGeom(rs[0]);
			OGRGeomPtr pol1 = blk->Union(pol2);
			blk = pol1;
		}
		rs.next();
	}
	std::vector<OGRGeomPtr> vs;
	OGRwkbGeometryType ty = blk->getGeometryType();
	if ( ty == wkbMultiPolygon ) {
		OGRGeometryCollection* oc = (OGRGeometryCollection*) ((OGRGeometry*) blk);
		int np = oc->getNumGeometries();
		for (int i = 0; i < np; i++ ) {
			OGRGeomPtr pol = oc->getGeometryRef(i);
			OGRGeomPtr inter = pol->Difference(strips);
			OGRPolygon* p1 = (OGRPolygon*) ((OGRGeometry*) pol);
			OGRPolygon* p2 = (OGRPolygon*) ((OGRGeometry*) inter);
			if ( fabs(p1->get_Area() - p2->get_Area()) > 5 ) {
				vs.push_back(inter);
			}
		}
	} else {
		OGRPolygon* p1 = (OGRPolygon*) ((OGRGeometry*) blk);
		OGRGeomPtr inter = blk->Difference(strips);
		OGRPolygon* p2 = (OGRPolygon*) ((OGRGeometry*) inter);
		if ( fabs(p1->get_Area() - p2->get_Area()) < 5 ) {
			vs.push_back(inter);
		}
	}
	_uncovered(vs);
	return vs.empty();
}
void photo_exec::_uncovered(std::vector<OGRGeomPtr>& vs)
{
	std::string table(_type == Prj_type ? "Z_UNCOVERP" : "Z_UNCOVERV");
	cnn.remove_layer(table);
	if ( vs.empty() )
		return;

	// create the photo table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_UNCOVER_ID TEXT NOT NULL)";			// roll
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
	sql2 << "INSERT INTO " << table << " (Z_UNCOVER_ID, geom) \
		VALUES (?1, ST_GeomFromWKB(:geom, " << SRID << ") )";
	
	Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());
	for (size_t i = 0; i < vs.size(); i++) {
		stm[1] = (int) i;
		stm[2].fromBlob(vs[i]);

		stm.execute();
		stm.reset();
	}
	cnn.commit_transaction();
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

void photo_exec::_get_elong(OGRGeomPtr fv0, double ka, double* d1, double* d2)
{
	OGRPoint po;
	if ( fv0->Centroid(&po) != OGRERR_NONE )
		return;
	OGRGeometry* fv = fv0;
	OGRLinearRing* or = ((OGRPolygon*) fv)->getExteriorRing();
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
	std::string table = std::string(Z_FOTO) + (_type == Prj_type ? "P" : "V");
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

	Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());
		
	OGRSpatialReference sr;
	sr.importFromEPSG(SRID);

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

		OGRGeometryFactory gf;
		OGRGeomPtr gp_ = gf.createGeometry(wkbLinearRing);
		OGRLinearRing* gp = (OGRLinearRing*) ((OGRGeometry*) gp_);
		gp->setCoordinateDimension(2);
		gp->assignSpatialReference(&sr);

		for (int i = 0; i < 4; i++)
			gp->addPoint(dpol[i].x, dpol[i].y);
		gp->closeRings();

		OGRGeomPtr pol = gf.createGeometry(wkbPolygon);
		OGRPolygon* p = (OGRPolygon*) ((OGRGeometry*) pol);
		p->setCoordinateDimension(2);
		p->assignSpatialReference(&sr);
		p->addRing(gp);

		// calculate mean GSD
		dt = vdp.pix() * dt / (5 * vdp.foc());

		stm[1] = SIGLA_PRJ;
		stm[2] = get_strip(it->first);
		stm[3] = it->first;
		stm[4] = dt;
		stm[5] = RAD_DEG(vdp.om);
		stm[6] = RAD_DEG(vdp.fi);
		stm[7].fromBlob(pol);

		stm.execute();
		stm.reset();
	}
	cnn.commit_transaction();
	return true;
}
// builds the models from adjacent Photos of the same strip
bool photo_exec::_process_models()
{
	std::string table = std::string(Z_MODEL) + (_type == Prj_type ? "P" : "V");
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
		OGRGeomPtr pol1;
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
				OGRGeomPtr pol2 = GetGeom(rs1[6]);
				// the model is the intersection of two photos
				OGRGeomPtr mod = pol1->Intersection(pol2);
		
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
				_get_elong(pol1, _vdps[nomeleft].ka, &d1f, &d2f);
				_get_elong(mod, _vdps[nomeleft].ka, &d1m, &d2m);
				double lo = 100 * d1m / d1f; // longitudinal overlap
				double to = 100 * d2m / d2f; // trasversal overlap

				stm[1] = id;
				stm[2] = strip;
				stm[3] = nomeleft;
				stm[4] = nomeright;
				stm[5] = (int) lo;
				stm[6] = (int) to;
				stm[7] = dh;
				stm[8].fromBlob(mod);
				stm.execute();
				stm.reset();
				nomeleft = nomeright;
				pol1 = pol2;

			}
            rs1.next(); // recordset on the foto 
        }
		rs.next(); // recordset on the strips
	}
	cnn.commit_transaction();
	return true;
}
// costruisce le strisciate unendo tutte le foto di una stessa strip
bool photo_exec::_process_strips()
{
	std::string table = std::string(Z_STRIP) + (_type == Prj_type ? "P" : "V");
	cnn.remove_layer(table);

	// create the strip table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_STRIP_ID TEXT NOT NULL, " <<	// sigla del lavoro
		"Z_STRIP_CS TEXT NOT NULL, " <<
		"Z_STRIP_FIRST TEXT NOT NULL, " <<
		"Z_STRIP_LAST TEXT NOT NULL, "  // overlap trasversale
		"Z_STRIP_COUNT INTEGER NOT NULL, "
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
	sql2 << "INSERT INTO " << table << " (Z_STRIP_ID, Z_STRIP_CS, Z_STRIP_FIRST, Z_STRIP_LAST, Z_STRIP_COUNT, Z_STRIP_LENGTH, geom) \
		VALUES (?1, ?2, ?3, ?4, ?5, ?6, ST_GeomFromWKB(:geom, " << SRID << ") )";
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
		OGRGeomPtr pol1;
		std::string firstname, lastname;
		bool first = true;
		int count = 1;
		while ( !rs1.eof() ) {
			if ( first ) {
				id = rs1["Z_MODEL_ID"].toString();
				strip = rs1["Z_MODEL_CS"].toString();
				firstname = rs1["Z_MODEL_LEFT"].toString();
				first = false;

				pol1 = GetGeom(rs1[4]);
			} else {
				lastname = rs1["Z_MODEL_RIGHT"].toString();
				// joins all the models
				OGRGeomPtr pol2 = GetGeom(rs1[4]);
				OGRGeomPtr mod = pol1->Union(pol2);
				pol1 = mod;
			}
			count++;
			rs1.next();
		}
		double l = _vdps[firstname].Pc.dist2D(_vdps[lastname].Pc) / 1000.; // strip length in km
		stm[1] = id;
		stm[2] = strip;
		stm[3] = firstname;
		stm[4] = lastname;
		stm[5] = count;
		stm[6] = l;
		stm[7].fromBlob(pol1);
		stm.execute();
		stm.reset();
		
		rs.next();
	}
	cnn.commit_transaction();
	return true;
}
typedef struct mstrp {
	std::string strip;
	std::string first;
	std::string last;
	OGRGeomPtr geo;
} mstrp;
bool photo_exec::_process_block()
{
	std::string table = std::string(Z_BLOCK) + (_type == Prj_type ? "P" : "V");
	cnn.remove_layer(table);

	// create the strip table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_STRIP_ID TEXT NOT NULL, " <<	// sigla del lavoro
		"Z_STRIP1 TEXT NOT NULL, " <<
		"Z_STRIP2 TEXT NOT NULL, " <<
		"Z_STRIP_T_OVERLAP INT NOT NULL)";  // overlap trasversale
	cnn.execute_immediate(sql.str());

	std::stringstream sql2;
	std::string tablef(_type == Prj_type ? "Z_STRIPP" : "Z_STRIPV");
	
	sql2 << "select Z_STRIP_CS, Z_STRIP_FIRST, Z_STRIP_LAST, AsBinary(geom) from " << tablef << " order by Z_STRIP_CS";
	CV::Util::Spatialite::Statement stm1(cnn);
	stm1.prepare(sql2.str());
	CV::Util::Spatialite::Recordset rs = stm1.recordset();
	
	std::vector<mstrp> vs;
	bool first = true;
	OGRGeomPtr blk;

	while ( !rs.eof() ) { //for every strip
		mstrp s;
		s.strip = rs[0].toString();
		s.first = rs[1].toString();
		s.last = rs[2].toString();
		if ( first ) {
			first = false;
			blk = GetGeom(rs[3]);
			s.geo = blk;
		} else {
			OGRGeomPtr pol2 = GetGeom(rs[3]);
			s.geo = pol2;
			OGRGeomPtr pol1 = blk->Union(pol2);
			blk = pol1;
		}
		vs.push_back(s);
		rs.next();
	}

	_get_carto(blk);

	std::stringstream sql3;
	sql3 << "INSERT INTO " << table << " (Z_STRIP_ID, Z_STRIP1, Z_STRIP2, Z_STRIP_T_OVERLAP) VALUES (?1, ?2, ?3, ?4)";
	CV::Util::Spatialite::Statement stm2(cnn);
	cnn.begin_transaction();
	stm2.prepare(sql3.str());

	int k = 1;
	for (size_t i = 0; i < vs.size(); i++) {
		VDP& vdp1 = _vdps[vs[i].first];
		VDP& vdp2 = _vdps[vs[i].last];
		double k1 = vdp2.Pc.angdir(vdp1.Pc);
		VecOri v1(vdp2.Pc - vdp1.Pc);
		for (size_t j = i + 1; j < vs.size(); j++) {
			VDP& vdp3 = _vdps[vs[j].first];
			VDP& vdp4 = _vdps[vs[j].last];
			VecOri v2(vdp4.Pc - vdp3.Pc);
			double ct = RAD_DEG(acos((v1 % v2) / (v1.module() * v2.module())));
			if ( fabs(ct) < 10 || fabs(ct) > 170 ) { // 10 deg difference in the heading means they are parallel
				OGRGeomPtr g1 = vs[i].geo;
				OGRGeomPtr g2 = vs[j].geo;
				if ( g1->Intersect(g2) ) {
					double d1, d2, d3, d4;
					_get_elong(g1, k1, &d1, &d2);
					OGRGeomPtr inter = g1->Intersection(g2);
					_get_elong(inter, k1, &d3, &d4);
					double dt = (int) 100 * (d3 / d1);
					stm2[1] = (int) k++;
					stm2[2] = vs[i].strip;
					stm2[3] = vs[j].strip;
					stm2[4] = dt;
					stm2.execute();
					stm2.reset();
				}
			}
		}
	}
	cnn.commit_transaction();
	return true;
}
bool photo_exec::_prj_report()
{
	// se si tratta di controllo del volo va verificata la congruenza col progetto
	// 1 numero e lunghezza delle strisciate
	// 2 per ogni strisciata numero di fotogrammi
	return false;
}
bool photo_exec::_foto_report()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica Parametri immagini");

	double v1 = _GSD * (1 - _MAX_GSD / 100);
	double v2 = _GSD * (1 + _MAX_GSD / 100);

	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
	ss << "GSD compreso tra " << v1 << " e " << v2;
	itl->add_item("listitem")->append(ss.str());
	std::stringstream ss1;
	ss1 << "angoli di pitch e roll minori di " << _MAX_ANG;
	itl->add_item("listitem")->append(ss1.str());

	// verifica della dimensione del pixel e dei valori di picth e roll
	std::stringstream sql;
	std::string table = std::string(Z_FOTO) + (_type == Prj_type ? "P" : "V");
	sql << "SELECT Z_FOTO_NF, Z_FOTO_DIMPIX, Z_FOTO_PITCH, Z_FOTO_ROLL FROM " << table << " WHERE Z_FOTO_DIMPIX not between " << v1 << " and " << v2 <<
		" OR Z_FOTO_PITCH>" << _MAX_ANG << " OR Z_FOTO_ROLL>" << _MAX_ANG;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	if ( rs.fields_count() == 0 ) {
		sec->add_item("para")->append("In tutte le immagini i parametri verificati rientrano nei range previsti");
		return true;
	}
	sec->add_item("para")->append("Nelle seguenti immagini i parametri verificati non rientrano nei range previsti");
	
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("Immagini con parametri fuori range");

	Poco::XML::AttributesImpl attr;
	attr.addAttribute("", "", "cols", "", "4");
	tab = tab->add_item("tgroup", attr);

	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");

	attr.clear();
	attr.addAttribute("", "", "align", "", "center");
	row->add_item("entry", attr)->append("Foto");
	row->add_item("entry", attr)->append("GSD");
	row->add_item("entry", attr)->append("Pitch");
	row->add_item("entry", attr)->append("Roll");

	Doc_Item tbody = tab->add_item("tbody");

	Poco::XML::AttributesImpl attrr;
	attrr.addAttribute("", "", "align", "", "right");
	while ( !rs.eof() ) {
		row = tbody->add_item("row");

		row->add_item("entry", attr)->append(rs[0].toString());
		
		print_item(row, attrr, rs[1].toDouble(), between_ty, v1, v2);
		print_item(row, attrr, rs[2].toDouble(), abs_less_ty, _MAX_ANG);
		print_item(row, attrr, rs[3].toDouble(), abs_less_ty, _MAX_ANG);
		rs.next();
	}
	return false;
}
bool photo_exec::_model_report()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica parametri stereo modelli");

	double v1 = _MODEL_OVERLAP * (1 - _MODEL_OVERLAP_RANGE / 100);
	double v2 = _MODEL_OVERLAP * (1 + _MODEL_OVERLAP_RANGE / 100);

	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
	ss << "Ricoprimento longitudinale compreso tra " << v1 << "% e " << v2 << "%";
	itl->add_item("listitem")->append(ss.str());
	std::stringstream ss1;
	ss1 << "Ricoprimento trasversale maggiore di " << _MODEL_OVERLAP_T << "%";
	itl->add_item("listitem")->append(ss1.str());
	std::stringstream ss2;
	ss2 << "Differenza di heading tra i fotogrammi minori di " << _MAX_HEADING_DIFF;
	itl->add_item("listitem")->append(ss2.str());

	// verifica ricoprimento tra fotogrammi
	std::string table = std::string(Z_MODEL) + (_type == Prj_type ? "P" : "V");
	std::stringstream sql;
	sql << "SELECT Z_MODEL_LEFT, Z_MODEL_RIGHT, Z_MODEL_L_OVERLAP, Z_MODEL_T_OVERLAP, Z_MODEL_D_HEADING FROM " << table << " WHERE Z_MODEL_L_OVERLAP not between " << v1 << " and " << v2 <<
		" OR Z_MODEL_T_OVERLAP <" << _MODEL_OVERLAP_T << " OR Z_MODEL_D_HEADING >" << _MAX_HEADING_DIFF;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	if ( rs.fields_count() == 0 ) {
		sec->add_item("para")->append("In tutti i modelli i parametri verificati rientrano nei range previsti");
		return true;
	}
	sec->add_item("para")->append("Nei seguenti modelli i parametri verificati non rientrano nei range previsti");
	
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("modelli con parametri fuori range");

	Poco::XML::AttributesImpl attr;
	attr.addAttribute("", "", "cols", "", "5");
	tab = tab->add_item("tgroup", attr);

	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");

	attr.clear();
	attr.addAttribute("", "", "align", "", "center");
	row->add_item("entry", attr)->append("Foto Sx");
	row->add_item("entry", attr)->append("Foto Dx");
	row->add_item("entry", attr)->append("Ric. long.");
	row->add_item("entry", attr)->append("Ric. trasv.");
	row->add_item("entry", attr)->append("Dif. head.");

	Doc_Item tbody = tab->add_item("tbody");

	//Doc_Item row = tbody->add_item("row");
	Poco::XML::AttributesImpl attrr;
	attrr.addAttribute("", "", "align", "", "right");
	while ( !rs.eof() ) {
		row = tbody->add_item("row");

		row->add_item("entry", attr)->append(rs[0].toString());
		row->add_item("entry", attr)->append(rs[1].toString());
		
		print_item(row, attrr, rs[2].toDouble(), between_ty, v1, v2);
		print_item(row, attrr, rs[3].toDouble(), great_ty, _MODEL_OVERLAP_T);
		print_item(row, attrr, rs[4].toDouble(), abs_less_ty, _MAX_HEADING_DIFF);
		rs.next();
	}
	return false;
}
bool photo_exec::_strip_report()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica parametri strisciate");

	double v1 = _STRIP_OVERLAP * (1 - _STRIP_OVERLAP_RANGE / 100);
	double v2 = _STRIP_OVERLAP * (1 + _STRIP_OVERLAP_RANGE / 100);

	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
	ss << "Ricoprimento Trasversale compreso tra " << v1 << "% e " << v2 << "%";
	itl->add_item("listitem")->append(ss.str());
	std::stringstream ss1;
	ss1 << "Massima lunghezza strisciate minore di " << _MAX_STRIP_LENGTH;
	itl->add_item("listitem")->append(ss1.str());

	// Strip verification
	std::string table = std::string(Z_STRIP) + (_type == Prj_type ? "P" : "V");
	std::string table2 = std::string(Z_BLOCK) + (_type == Prj_type ? "P" : "V");
	std::stringstream sql;
	sql << "SELECT Z_STRIP_CS, Z_STRIP_LENGTH, Z_STRIP_T_OVERLAP, Z_STRIP2 FROM " << table << " a inner JOIN " << 
		table2 << " b on b.Z_STRIP1 = a.Z_STRIP_CS WHERE Z_STRIP_LENGTH>" << _MAX_STRIP_LENGTH << " OR Z_STRIP_T_OVERLAP<" << v1 << " OR Z_STRIP_T_OVERLAP>" << v2;

	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	if ( rs.fields_count() == 0 ) {
		sec->add_item("para")->append("In tutte le strisciate i parametri verificati rientrano nei range previsti");
		return true;
	}
	sec->add_item("para")->append("Nelle seguenti strisciate i parametri verificati non rientrano nei range previsti");
	
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("Strisciate con parametri fuori range");

	Poco::XML::AttributesImpl attr;
	attr.addAttribute("", "", "cols", "", "4");
	tab = tab->add_item("tgroup", attr);

	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");

	attr.clear();
	attr.addAttribute("", "", "align", "", "center");
	row->add_item("entry", attr)->append("Strip.");
	row->add_item("entry", attr)->append("Lung.");
	row->add_item("entry", attr)->append("Ric. trasv.");
	row->add_item("entry", attr)->append("Strip adiac.");

	Doc_Item tbody = tab->add_item("tbody");

	Poco::XML::AttributesImpl attrr;
	attrr.addAttribute("", "", "align", "", "right");
	while ( !rs.eof() ) {
		row = tbody->add_item("row");

		row->add_item("entry", attr)->append(rs[0].toString());
		
		print_item(row, attrr, rs[1].toDouble(), less_ty, _MAX_STRIP_LENGTH);
		print_item(row, attrr, rs[2].toDouble(), between_ty, v1, v2);

		row->add_item("entry", attr)->append(rs[3].toString());
		rs.next();
	}
	return false;
}
void photo_exec::_final_report()
{
	// test to be done only for the real flight
	if ( _type == fli_type ) {
		_prj_report();
	}

	// common tests

	// coverage of cartographic areas
	std::stringstream sql;
	std::string table = std::string(Z_BLOCK) + (_type == Prj_type ? "P" : "V");
	sql << "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='" <<table << "'";
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();

	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica Copertura aree da cartografare");

	int cv = rs[0].toInt();
	if ( cv == 0 ) {
		sec->add_item("para")->append("Tutte le aree da cartografare sono state ricoperte da modelli stereoscopici");
	} else {
		sec->add_item("para")->append("Esistono delle aree da cartografare non completamente ricoperte da modelli stereoscopici");
	}
	_foto_report();
	_model_report();
	_strip_report();
}
