/*
    File: ortho_exec.cpp
    Author:  F.Flamigni
    Date: 2013 November 22
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
#include "check_ortho.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/stringtokenizer.h"
#include "Poco/AutoPtr.h"
#include "Poco/sharedPtr.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/string.h"
#include <fstream>
#include <sstream>
#include "gdal/ogr_geometry.h"
#include "tiff_util.h"

#define SRID 32632
#define SIGLA_PRJ "CSTP"
#define SHAPE_CHAR_SET "CP1252"
#define REFSCALE "RefScale_2000"
#define QUADRO_RT "Quadro_RT"
#define DB_NAME "geo.sqlite"
#define OUT_DOC "check_ortho.xml"
#define REF_FILE "Regione_Toscana_RefVal.xml"

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::SharedPtr;
using Poco::Path;
using Poco::File;
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
typedef std::vector<unsigned char> Blob;
/**************************************************************/

/*****************************************************/

ortho_exec::~ortho_exec() 
{
}
void ortho_exec::_init_document()
{
	Path doc_file(_proj_dir, "*");
	doc_file.setFileName(OUT_DOC);
	_dbook.set_name(doc_file.toString());	

	_article = _dbook.add_item("article");
	_article->add_item("title")->append("Collaudo ortho immagini");
}

bool ortho_exec::run()
{
	try {
		// initialize spatial lite connection
		Poco::Path db_path(_proj_dir, DB_NAME);
		cnn.create(db_path.toString());
		cnn.initialize_metdata();

		// loads the refverence union table
		int nrows = cnn.load_shapefile("C:/Google_drive/Regione Toscana Tools/Dati_test/Quadro/qu-ofc-cast_pescaia-scarlino-etrf89",
		   QUADRO_RT,
		   SHAPE_CHAR_SET,
		   SRID,
		   "geom",
		   true,
		   false,
		   false);

		// Read reference values
		_read_ref_val();

		if ( !_process_imgs() )
			return false;

		// initialize docbook xml file
		_init_document();

		// write the result on the docbook report
		_dbook.write();
	}
    catch(std::exception &e) {
        std::cout << std::string(e.what()) << std::endl;
    }
	return true;
}
void ortho_exec::set_proj_dir(const std::string& nome)
{
	_proj_dir = nome;
}
void ortho_exec::set_img_dir(const std::string& nome)
{
	_img_dir = nome;
}

bool ortho_exec::_read_ref_val()
{
	Path ref_file(_proj_dir, "*");
	ref_file.popDirectory();
	ref_file.setFileName(REF_FILE);
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

bool ortho_exec::_process_imgs()
{
	// select all the tfw files in the folder
	Poco::Path tfw(_img_dir);
	File dircnt(tfw);
	std::vector<std::string> df;
	dircnt.list(df);
	std::vector<std::string> files;
	for (size_t i = 0; i < df.size(); i++) {
		Poco::Path fn(_img_dir, df[i]);
		std::string ext = Poco::toLower(fn.getExtension());
		if ( ext == "tfw" )
			files.push_back(fn.toString());
	}
	std::vector<TFW> vtfw;
	for (size_t i = 0; i < files.size(); i++) {
		TFW tf(files[i]);
		Poco::Path fn(files[i]);
		fn.setExtension("tif");
		vtfw.push_back(tf);
	}

	std::string table("Z_QUADRO");
	cnn.remove_layer(table);

	// create the union table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"( FOGLIO TEXT NOT NULL PRIMARY KEY, " <<
		"P_SIZE TEXT NOT NULL)";
	cnn.execute_immediate(sql.str());
	
	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'POLYGON'," <<
		"'XY')";
	cnn.execute_immediate(sql1.str());	

	// create the insertion query
	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (FOGLIO, P_SIZE, geom) \
		VALUES (?1, ?2, ST_GeomFromWKB(:geom, " << SRID << ") )";

	Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());
		
	OGRSpatialReference sr;
	sr.importFromEPSG(SRID);

	std::vector<DPOINT> dpol;
	for ( size_t i = 0; i < vtfw.size(); i++) {
		OGRGeometryFactory gf;
		OGRGeomPtr gp_ = gf.createGeometry(wkbLinearRing);
		OGRLinearRing* gp = (OGRLinearRing*) ((OGRGeometry*) gp_);
		//gp->setCoordinateDimension(2);
		//gp->assignSpatialReference(&sr);
		for (int j = 0; j < 4; j++) {
			DPOINT pi( j == 0 || j == 3 ? 0 : vtfw[i].dimx(), j == 0 || j == 1 ? vtfw[i].dimy() :  0, 1);
			DPOINT pt = vtfw[i].img_ter(pi);
			gp->addPoint(pt.x, pt.y);
		}

		gp->closeRings();
		OGRGeomPtr pol = gf.createGeometry(wkbPolygon);
		OGRPolygon* p = (OGRPolygon*) ((OGRGeometry*) pol);
		p->setCoordinateDimension(2);
		p->assignSpatialReference(&sr);
		p->addRing(gp);

		stm[1] = vtfw[i].nome();
		stm[2] = vtfw[i].pix_size();
		stm[3].fromBlob(pol);

		stm.execute();
		stm.reset();
	}
	cnn.commit_transaction();
	return true;
}
bool ortho_exec::_process_photos()
{
	//std::string table("Z_QUADRO");
	//cnn.remove_layer(table);

	////// create the photo table
	//std::stringstream sql;
	//sql << "CREATE TABLE " << table << 
	//	"( _FOTO_ID TEXT NOT NULL, " <<
	//	"Z_FOTO_ID TEXT NOT NULL, " <<
	//	"Z_FOTO_CS TEXT NOT NULL," <<
	//	"Z_FOTO_NF INTEGER NOT NULL PRIMARY KEY," <<
	//	"Z_FOTO_DIMPIX DOUBLE NOT NULL, " <<
	//	"Z_FOTO_PITCH DOUBLE NOT NULL, " <<
	//	"Z_FOTO_ROLL DOUBLE NOT NULL)";
	//cnn.execute_immediate(sql.str());

	//std::stringstream sql1;
	//sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
	//	"'geom'," <<
	//	SRID << "," <<
	//	"'POLYGON'," <<
	//	"'XYZ')";
	//cnn.execute_immediate(sql1.str());

	//std::stringstream sql2;
	//sql2 << "INSERT INTO " << table << " (Z_FOTO_ID, Z_FOTO_CS, Z_FOTO_NF, Z_FOTO_DIMPIX, Z_FOTO_PITCH, Z_FOTO_ROLL, geom) \
	//	VALUES (?, ?, ?, ?, ?, ?, ?)";

	//CV::Util::Spatialite::Statement stm(cnn);
	//cnn.begin_transaction();
	//stm.prepare(sql2.str());

	//std::map<std::string, VDP>::iterator it;
	//for (it = _vdps.begin(); it != _vdps.end(); it++) {
	//	VDP& vdp = it->second;
	//	double dt = 0.;

	//	DPOINT Pc(vdp.Pc.GetX()	, vdp.Pc.GetY(), vdp.Pc.GetZ());
	//	std::vector<DPOINT> dpol;
	//	for ( int i = 0; i < 5; i++) {
	//		Collimation ci;
	//		ci.xi = ( i == 0 || i == 3 ) ? 0.f : (i != 4) ? (float) vdp.dimx() : (float) (vdp.dimx() / 2.);
	//		ci.yi = ( i == 0 || i == 1 ) ? 0.f : (i != 4) ? (float) vdp.dimy() : (float) (vdp.dimy() / 2.);
	//		DPOINT pd, pt;
	//		vdp.GetRay(ci, &pd);
	//		if ( !ds->RayIntersect(Pc, pd, pt) ) {
	//			if ( !ds->IsInside(pt.z) ) {
	//				break;
	//			}
	//		}
	//		dt += vdp.Pc.GetZ() - pt.z;
	//		dpol.push_back(pt);
	//	}

	//	gaiaGeomCollPtr geo = gaiaAllocGeomColl();
	//	geo->Srid = SRID;
	//	geo->DimensionModel  = GAIA_XY_Z;
	//	geo->DeclaredType = GAIA_POLYGONZ;
	//	gaiaPolygonPtr polyg = gaiaAddPolygonToGeomColl(geo, 5, 0);
	//	gaiaRingPtr ring = polyg->Exterior;

	//	for (int i = 0; i < 4; i++)
	//		gaiaSetPointXYZ(ring->Coords, i, dpol[i].x, dpol[i].y, dpol[i].z);
	//	gaiaSetPointXYZ(ring->Coords, 4, dpol[0].x, dpol[0].y, dpol[0].z);

	//	//calcola del GSD medio
	//	dt = vdp.pix() * dt / (5 * vdp.foc());

	//	//sqlite3_reset(stmt);
	//	//sqlite3_clear_bindings(stmt);

	//	unsigned char *blob;
	//	int blob_size;
	//	gaiaToSpatiaLiteBlobWkb(geo, &blob, &blob_size);
	//	gaiaFreeGeomColl (geo);

	//	// we can now destroy the geometry object
	//	std::string strip = _get_strip(it->first);
	//	splite.bind(1, SIGLA_PRJ, w_spatialite::TEXT);
	//	splite.bind(2, strip.c_str(), w_spatialite::TEXT);
	//	int id = atoi(_get_nome(it->first).c_str());
	//	splite.bind(3, &id, w_spatialite::INT);
	//	splite.bind(4, &dt, w_spatialite::DOUBLE);
	//	double o = RAD_DEG(vdp.om), f = RAD_DEG(vdp.fi);
	//	splite.bind(5, &o), w_spatialite::DOUBLE);
	//	splite.bind(6, &f, w_spatialite::DOUBLE);
	//	splite.bind(7, blob, blob, blob_size);

	//	//sqlite3_bind_text(stmt, 1, SIGLA_PRJ, strlen(SIGLA_PRJ), SQLITE_STATIC);
	//	//sqlite3_bind_text(stmt, 2, strip.c_str(), strip.size(), SQLITE_STATIC);
	//	//sqlite3_bind_int(stmt, 3, atoi(_get_nome(it->first).c_str()));
	//	//sqlite3_bind_double(stmt, 4, dt);
	//	//sqlite3_bind_double(stmt, 5, RAD_DEG(vdp.om));
	//	//sqlite3_bind_double(stmt, 6, RAD_DEG(vdp.fi));
	//	//sqlite3_bind_blob (stmt, 7, blob, blob_size, SQLITE_STATIC);

	//	stm.execute();
	//	
	//	//if ( ! splite.step() {
	//	////int retv = sqlite3_step(stmt);
	//	////if ( retv != SQLITE_DONE && retv != SQLITE_ROW) {
	//	//	printf ("sqlite3_step() error: %s\n", splite.get_error_msg());
	//	//      break;
	//	//}
	//	gaiaFree(blob);
	//	//_vfoto.push_back(fet);
	//}
	//cnn.commit_transaction();
	////sqlite3_finalize(stmt);

	////splite.commit();
	////ret = sqlite3_exec (db_handle, "COMMIT", NULL, NULL, &err_msg);
	////if (ret != SQLITE_OK) {
	////	fprintf (stderr, "Error: %s\n", err_msg);
	////	sqlite3_free (err_msg);
	////}
	////_process_models();
	////_process_strips();
	return true;
}


