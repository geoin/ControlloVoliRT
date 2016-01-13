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
#include "Poco/StringTokenizer.h"
#include "Poco/AutoPtr.h"
#include "Poco/SharedPtr.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "common/util.h"
#include "photo_util/sun.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeParser.h"

#define SIGLA_PRJ "CSTP"
#define CARTO "CARTO"
#define REF_FILE "refval.xml"
#define FOTOGRAMMETRIA "Fotogrammetria"

#define CAMERA "camera.xml"
#define ASSETTI "assetti"
#define DEM "dem"
#define OUT_DOCV "check_photoV.xml"
#define OUT_DOCP "check_photoP.xml"

#include "cv_version.h"

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::SharedPtr;
using Poco::Path;
using namespace CV::Util::Spatialite;
using namespace CV::Util::Geometry;

photo_exec::~photo_exec()
{
    if (_df != NULL) {
        delete _df;
    }
}

std::string photo_exec::_get_key(const std::string& val)
{
	return std::string(FOTOGRAMMETRIA) + "." + _refscale + "." + val;
}

bool photo_exec::run()
{
	CV::Version::print();

    if (_proj_dir.empty()) {
        throw std::runtime_error("cartella di lavoro non impostata");
    }

	try {
		// initialize spatial lite connection
		Poco::Path db_path(_proj_dir, DB_NAME);
        cnn.open(db_path.toString());

        if ( !GetProjData(cnn, _note, _refscale) ) {
			throw std::runtime_error("dati progetto incompleti");
        }

        if ( _refscale.empty() ) {
			throw std::runtime_error("Scala di lavoro non impostata");
        }

		//_cam_name = Path(_proj_dir, CAMERA).toString();
		/*std::string assetti = std::string(ASSETTI) + (_type == Prj_type ? "P" : "V") + ".txt";
		std::string assettip = std::string(ASSETTI) + "P" + ".txt";
		_vdp_name = Path(_proj_dir, assetti).toString();
		_vdp_name_proj = Path(_proj_dir, assettip).toString();
		_dem_name = Path(_proj_dir, DEM).toString();*/

		// map areas
		std::cout << "Layer:" << CARTO << std::endl;
		
		// Read reference values
		_read_ref_val();

		// read planned photo position and attitude
		if ( _type == fli_type ) {

			// read photo position and attitude
			if ( !_read_vdp(_vdps) )
				throw std::runtime_error("File assetti non trovato");

			std::string assi = std::string(ASSI_VOLO) + "V";
			std::cout << "Layer:" << assi << std::endl;
            if ( !_calc_vdp(_vdps_plan) ) {
				std::cout << "Tema degli assi di volo progettati non trovato" << std::endl;
            }
		} else {
            if ( !_calc_vdp(_vdps) ) {
				throw std::runtime_error("Tema degli assi di volo progettati non trovato");
            }
		}
		
		// read digital terrain model
        if ( !_read_dem() ) {
			throw std::runtime_error("Modello numerico non trovato");
        }

		// initialize docbook xml file
		std::string title =_type == fli_type ? "Collaudo ripresa aerofotogrammetrica" : "Collaudo progetto di ripresa aerofotogrammetrica";
		Path doc_file(_proj_dir, "*");
		doc_file.setFileName(_type == fli_type ? OUT_DOCV : OUT_DOCP);

		init_document(_dbook, doc_file.toString(), title, _note);
		char* dtd_ = getenv("DOCBOOKRT");
		std::string dtd;
		if ( dtd_ != NULL )
			dtd = std::string("file:") + dtd_;
		_dbook.set_dtd(dtd);
		_article = _dbook.get_item("article");

		_get_carto(_carto);
		// produce photos feature
		_process_photos();
		_process_models();
		_process_strips();
		_process_block();

		std::cout << "Produzione del report finale: " << _dbook.name() << std::endl;
		_final_report();

		// write the result on the docbook report
		_dbook.write();
		std::cout << "Procedura terminata:" << std::endl;
	}
    catch(std::exception &e) {
        std::cout << std::string(e.what()) << std::endl;
    }
	return true;
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
	//ref_file.popDirectory();
	ref_file.setFileName(REF_FILE);
	AutoPtr<XMLConfiguration> pConf;
	try {
		pConf = new XMLConfiguration(ref_file.toString());
		_GSD = pConf->getDouble(_get_key("GSD"));
		_MAX_GSD = pConf->getDouble(_get_key("MAX_GSD"));
		_MODEL_OVERLAP = pConf->getDouble(_get_key("MODEL_OVERLAP"));
		_MODEL_OVERLAP_RANGE = pConf->getDouble(_get_key("MODEL_OVERLAP_RANGE"));
		_MODEL_OVERLAP_T = pConf->getDouble(_get_key("MODEL_OVERLAP_T"));
		_STRIP_OVERLAP = pConf->getDouble(_get_key("STRIP_OVERLAP"));
		_STRIP_OVERLAP_RANGE = pConf->getDouble(_get_key("STRIP_OVERLAP_RANGE"));
		_MAX_STRIP_LENGTH = pConf->getDouble(_get_key("MAX_STRIP_LENGTH"));
		_MAX_HEADING_DIFF = pConf->getDouble(_get_key("MAX_HEADING_DIFF"));
		_MAX_ANG = pConf->getDouble(_get_key("MAX_ANG"));

		_MAX_PDOP = pConf->getDouble(_get_key("MAX_PDOP"));
		_MIN_SAT = pConf->getInt(_get_key("MIN_SAT"));
		_MAX_DIST = pConf->getInt(_get_key("MAX_DIST")) * 1000;
		_MIN_SAT_ANG = pConf->getDouble(_get_key("MIN_SAT_ANG"));
		_NBASI = pConf->getInt(_get_key("NBASI"));
		_MIN_ANG_SOL = pConf->getDouble(_get_key("MIN_ANG_SOL"));

		_MAX_GPS_GAP = pConf->getDouble(_get_key("MAX_GPS_GAP"), 3);
    } catch (const std::exception& ex) {
        throw std::runtime_error(std::string("Errore nela lettura dei valori di riferimento: ") + ex.what());
	}
	return true;
}

bool photo_exec::_get_carto(OGRGeomPtr& carto)
{
	std::string table(CARTO);

	std::stringstream sql;
	sql << "select AsBinary(geom) from " << table;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	
	bool first = true;
	//OGRGeomPtr carto;

	long count = 0;
	while ( !rs.eof() ) { //for every strip
		++count;
        Blob blob = rs[0];
		if ( first ) {
            first = false;
            carto = blob;
		} else {
            Blob b = rs[0];
            OGRGeomPtr pol = blob;
			carto = carto->Union(pol);
		}
		rs.next();
	}
	if ( !carto->IsValid() )
		throw std::runtime_error("Geometria delle aree da cartografare no valide");

	return true;
}
bool photo_exec::_uncovered(OGRGeomPtr& vs)
{
	std::string table = std::string(Z_UNCOVER) + (_type == Prj_type ? "P" : "V");
	cnn.remove_layer(table);
	if ( vs->IsEmpty() ) 
		return true;

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
		"'" << get_typestring(vs) << "'," <<
		"'XY')";
	cnn.execute_immediate(sql1.str());
	std::cout << "Layer:" << table << std::endl;

	// create the insertion query
	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (Z_UNCOVER_ID, geom) \
		VALUES (?1, ST_GeomFromWKB(:geom, " << SRID << ") )";
	
	Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());
	stm[1] = (int) 1;
	stm[2].fromBlob(vs);
	stm.execute();
	stm.reset();
	cnn.commit_transaction();
	return false;
}
//bool photo_exec::_read_cam()
//{
//	std::stringstream sql;
//	sql << "SELECT * from " << Z_CAMERA; // << " where planning=1";
//	Statement stm(cnn);
//	stm.prepare(sql.str());
//	Recordset rs = stm.recordset();
//	while ( !rs.eof() ) {
//		Camera cam;
//		cam.foc = rs["FOC"];
//		cam.dimx = rs["DIMX"];
//		cam.dimy = rs["DIMY"];
//		cam.dpix = rs["DPIX"];
//		cam.xp = rs["XP"];
//		cam.yp = rs["YP"];
//		cam.serial = rs["SERIAL_NUMBER"];
//		cam.id = rs["ID"];
//		int plan = rs["PLANNING"];
//		cam.planning = plan == 1;
//		rs.next();
//		_cams[cam.id] = cam;
//		if ( cam.planning )
//			_cam_plan = cam;
//	}
//	return true;
//
//	//AutoPtr<XMLConfiguration> pConf;
//	//try {
//	//	pConf = new XMLConfiguration(_cam_name);
//	//	_cam.foc = atof(pConf->getString("FOC").c_str());
//	//	_cam.dimx = atof(pConf->getString("DIMX").c_str());
//	//	_cam.dimy = atof(pConf->getString("DIMY").c_str());
//	//	_cam.dpix = atof(pConf->getString("DPIX").c_str());
//	//	_cam.xp = atof(pConf->getString("XP", "0").c_str());
//	//	_cam.yp = atof(pConf->getString("YP", "0").c_str());
//	//} catch (...) {
//	//	return false;
//	//}
//	//return true;
//}

//bool photo_exec::_strip_cam()
//{
//	// get the camera associated to each mission
//	std::string table = "MISSION";
//	std::stringstream sql;
//	sql << "SELECT ID_CAMERA, NAME from " << table;
//	Statement stm(cnn);
//	stm.prepare(sql.str());
//	Recordset rs = stm.recordset();
//
//	std::map<std::string, std::string> map_mission_cam;
//
//	while ( !rs.eof() ) {
//		map_mission_cam[ rs["NAME"] ] = rs["ID_CAMERA"]; // mission name camera id
//		rs.next();
//	}
//	// get the mission associated to each strip
//	table = std::string(ASSI_VOLO) + "V";
//	std::stringstream sql1;
//	sql1 << "SELECT A_VOL_CS, MISSION from " << table;
//	stm = Statement(cnn);
//	stm.prepare(sql1.str());
//	rs = stm.recordset();
//	while ( !rs.eof() ) {
//		std::string strip = rs["A_VOL_CS"]; // strip name - mission name
//		std::string mission = rs["MISSION"];
//		std::string cam_id = map_mission_cam[mission]; // camera id for mission
//		if ( _cams.find(cam_id) != _cams.end() )
//			_map_strip_cam[strip] = _cams[cam_id];
//		else
//			_map_strip_cam[strip] = _cam_plan;
//		rs.next();
//	}
//	return true;
//}

void photo_exec::_assi_from_vdp(std::map<std::string, VDP>& vdps)
{
	std::string table = ASSI_VOLO + std::string("V");
	cnn.remove_layer(table);

	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(A_VOL_ENTE TEXT NOT NULL, " <<		
		"A_VOL_DT TEXT, " <<
		"A_VOL_RID TEXT, " <<
		"A_VOL_CS TEXT NOT NULL PRIMARY KEY," <<
		"A_VOL_DR TEXT," << 
		"A_VOL_QT DOUBLE NOT NULL, " <<
		"A_VOL_CCOD TEXT, " <<
		"A_VOL_DSTP DOUBLE, " <<
		"A_VOL_NFI NUMBER NOT NULL, " <<
		"A_VOL_NFF NUMBER NOT NULL)";
	cnn.execute_immediate(sql.str());

	// add the geom column
	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'LINESTRING'," <<
		"'XY')";
	cnn.execute_immediate(sql1.str());

	// create the insertion query
	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (A_VOL_ENTE, A_VOL_DT, A_VOL_RID, A_VOL_CS, A_VOL_DR, A_VOL_QT, A_VOL_CCOD, A_VOL_DSTP, A_VOL_NFI, A_VOL_NFF, geom) \
		VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ST_GeomFromWKB(:geom, " << SRID << ") )";

	Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());
		
	OGRSpatialReference sr;
	sr.importFromEPSG(SRID);

	std::map<std::string, VDP>::iterator it;
	std::string strip0, fi, ff;

	bool first = true;
	DPOINT p0, p1;
	for ( it = vdps.begin(); it != vdps.end(); it++) {
		std::string strip = get_strip(it->first);
		if ( strip != strip0 ) {
			if ( !first ) {
				OGRGeometryFactory gf;
				OGRGeomPtr gp_ = gf.createGeometry(wkbLineString);
				OGRLineString* gp = (OGRLineString*) ((OGRGeometry*) gp_);
				gp->setCoordinateDimension(2);
				gp->assignSpatialReference(&sr);
				gp->addPoint(p0.x, p0.y);
				gp->addPoint(p1.x, p1.y);

				stm[1] = "Regione Toscana";
				stm[2] = "";
				stm[3] = "";
				stm[4] = strip0;
				stm[5] = ""; // data
				stm[6] = (p1.z + p0.z) / 2;
				stm[7] = ""; // fotocamera
				stm[8] = 100; // focale
				stm[9] = fi; // numero prima foto
				stm[10] = ff; // numero ultima foto
				stm[11].fromBlob(gp_);
				stm.execute();
				stm.reset();
			} else {
				first = false;
			}
			strip0 = strip;
			p0 = it->second.Pc;
			fi = get_nome(it->first);
		}
		p1 = it->second.Pc;
		ff = get_nome(it->first);
	}

	// record the last strip
	OGRGeometryFactory gf;
	OGRGeomPtr gp_ = gf.createGeometry(wkbLineString);
	OGRLineString* gp = (OGRLineString*) ((OGRGeometry*) gp_);
	gp->setCoordinateDimension(2);
	gp->assignSpatialReference(&sr);
	gp->addPoint(p0.x, p0.y);
	gp->addPoint(p1.x, p1.y);

	stm[1] = "Regione Toscana";
	stm[2] = "";
	stm[3] = "";
	stm[4] = strip0;
	stm[5] = ""; // data
	stm[6] = (p1.z + p0.z) / 2;
	stm[7] = ""; // fotocamera
	stm[8] = 100; // focale
	stm[9] = fi; // numero prima foto
	stm[10] = ff; // numero ultima foto
	stm[11].fromBlob(gp_);
	stm.execute();
	stm.reset();

	cnn.commit_transaction();
}
// read attitude data for photo flight
bool photo_exec::_read_vdp(std::map<std::string, VDP>& vdps)
{
	std::string table = std::string(ASSETTI);

	std::stringstream sql;
	sql << "SELECT * from " << table;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	while ( !rs.eof() ) {
		std::string strip = rs["STRIP"];
		Camera cam;
		VDP vdp(cam, rs["ID"]);
		vdp.Init(DPOINT(rs["PX"], rs["PY"], rs["PZ"]), (double) rs["OMEGA"], (double) rs["PHI"], (double) rs["KAPPA"]);
		vdps[vdp.nome] = vdp;
		rs.next();
	}

    if (vdps.empty()) {
        return false;
    }
	// create the axis theme from attitude data
	_assi_from_vdp(vdps);
	// update axis with gps data
	_update_assi_volo();
	// read camera data
	read_cams(cnn, _map_strip_cam);

	// update inner orientation data
	std::map<std::string, VDP>::iterator it;
	for ( it = vdps.begin(); it !=vdps.end(); it++) {
		std::string strip = get_strip(it->first);
		Camera cam;
		if ( _map_strip_cam.find(strip) != _map_strip_cam.end() )
			cam = _map_strip_cam[strip];
		else
			cam = _map_strip_cam["progetto"]; //_cam_plan;
		it->second.InitIor(cam);
	}
	return true;
}

// build attitude data from planned flight lines 
bool photo_exec::_calc_vdp(std::map<std::string, VDP>& vdps)
{
	// read from the flight line table
	std::string table = std::string(ASSI_VOLO) + "P";
	std::stringstream sql;
	sql << "SELECT A_VOL_QT, A_VOL_CS, A_VOL_NFI, A_VOL_NFF, AsBinary(geom) geom from " << table;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();

	std::cout << "Layer:" << table << std::endl;

	Camera cam_plan;
	read_planned_cam(cnn, cam_plan);

	while ( !rs.eof() ) {
        Blob blob = rs["geom"];
        OGRGeomPtr pol = blob;

		OGRGeometry* og = (OGRGeometry*) pol;
		OGRLineString* ls = (OGRLineString*)og;
		int n = ls->getNumPoints();
		if ( n != 2 ) // each line must have only two points
			throw std::runtime_error("asse di volo non valido");

		double z = rs[0];
		DPOINT pt0(ls->getX(0), ls->getY(0), z);
		DPOINT pt1(ls->getX(1), ls->getY(1), z);

		std::string strip = rs[1];
		int first = rs[2]; // number of first photo in the strip
		int last = rs[3]; // number of last photo in the strip

		double alfa = pt0.angdir2(pt1);

		double len = pt1.dist2D(pt0); // strip length
		double step = len / (last - first); // distance between photos

		for (int i = first; i <= last; i++) {
			int k = i;
			char nomef[256];
			sprintf(nomef, "%s_%04d", strip.c_str(), k);
			VDP vdp(cam_plan, nomef); // cam is the camera used for planning the flight
			DPOINT pt(pt0.x + (i - first) * step * cos(alfa), pt0.y + (i - first) * step * sin(alfa), z);
			vdp.Init(pt, 0, 0, Conv<Angle_t::DEG>::FromRad(alfa));
			vdps[vdp.nome] = vdp;
		}
		rs.next();
	}

	return true;

}
bool photo_exec::_read_dem()
{
	std::string table = DEM;
	std::stringstream sql;
	sql << "SELECT URI from " << table;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	if ( !rs.eof() ) {
		Path dem_path = Path(_proj_dir, rs[0]);
		_dem_name = dem_path.toString();

		_df = new DSM_Factory;
		if ( _df == NULL )
			return false;
		if ( !_df->Open(_dem_name, false) )
			return false;
		return true;
	}
	return false;
}
void GetMbr(const OGRGeometry* fv, const OGRPoint& po, double ka, MBR& mbr)
{
    const OGRPolygon* polygon = static_cast<const OGRPolygon*>(fv);
    const OGRLinearRing* linearRing = polygon->getExteriorRing();
    if (!linearRing) {
        return;
    }

    for (int i = 0; i < linearRing->getNumPoints(); i++) {
        double x = linearRing->getX(i) - po.getX();
        double y = linearRing->getY(i) - po.getY();
		double x1 = x * cos(ka) + y * sin(ka);
		double y1 = -x * sin(ka) + y * cos(ka);
		mbr.Update(x1, y1);
    }
}
void photo_exec::_get_elong(OGRGeomPtr fv0, double ka, double* d1, double* d2)
{
	OGRPoint po;
	if ( fv0->Centroid(&po) != OGRERR_NONE )
		return;
	MBR mbr;

    const OGRGeometry* fv = fv0;
	if ( fv0->getGeometryType() == wkbMultiPolygon ) {
        const OGRGeometryCollection* geom = (OGRGeometryCollection*) fv;
        int n = geom->getNumGeometries();
        for (int i = 0; i < n; i++) {
            MBR b;
            GetMbr(geom->getGeometryRef(i), po, ka, b);
            mbr.Extend(b);
        }
	} else {
		//OGRLinearRing* or = ((OGRPolygon*) fv)->getExteriorRing();
        MBR b;
        GetMbr(fv, po, ka, b);
        mbr.Extend(b);
	}
	//OGRLinearRing* or = ((OGRPolygon*) fv)->getExteriorRing();
	//double xm = 1.e20, ym = 1.e20;
	//double xM = -1.e20, yM = -1.e20;

	//for (int i = 0; i < or->getNumPoints(); i++) {
	//	double x = or->getX(i) - po.getX();
	//	double y = or->getY(i) - po.getY();
	//	double x1 = x * cos(ka) + y * sin(ka);
	//	double y1 = -x * sin(ka) + y * cos(ka);
	//	xm = std::min(xm, x1);
	//	ym = std::min(ym, y1);
	//	xM = std::max(xM, x1);
	//	yM = std::max(yM, y1);
	//}
	double l1 = mbr.GetDx();
	double l2 = mbr.GetDy();
	//double l1 = fabs(xM - xm);
	//double l2 = fabs(yM - ym);
	if ( l1 > l2 ) {
		*d1 = l2;
		*d2 = l1;
	} else {
		*d1 = l1;
		*d2 = l2;
	}
}

void photo_exec::_process_gsd(std::vector<GSD>& vgsd)
{
	std::string table = std::string("_ZGSD") + (_type == Prj_type ? "P" : "V");
	cnn.remove_layer(table);

	std::cout << "Layer:" << table << std::endl;

	// create the photo table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_FOTO_ID TEXT NOT NULL, " <<		// photo id
        "Z_STRIP_ID TEXT NOT NULL, " <<		// strip id
		"Z_GSD DOUBLE NOT NULL)";		//  gsd value
	cnn.execute_immediate(sql.str());
	// add the geom column
	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'POINT'," <<
		"'XY')";
	cnn.execute_immediate(sql1.str());

	// create the insertion query
	std::stringstream sql2;
    sql2 << "INSERT INTO " << table << " (Z_FOTO_ID, Z_STRIP_ID, Z_GSD, geom) \
        VALUES (?1, ?2, ?3, ST_GeomFromWKB(:geom, " << SRID << ") )";

	Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());
	OGRSpatialReference sr;
	sr.importFromEPSG(SRIDGEO);
	for (size_t i = 0; i < vgsd.size(); i++) {
		OGRGeometryFactory gf;
		OGRGeomPtr gp_ = gf.createGeometry(wkbPoint);
		gp_->setCoordinateDimension(2);
		gp_->assignSpatialReference(&sr);
		OGRPoint* gp = (OGRPoint*) ((OGRGeometry*) gp_);
		*gp = OGRPoint(vgsd[i].pt.x, vgsd[i].pt.y);
		stm[1] = vgsd[i].foto;
        stm[2] = get_strip(vgsd[i].foto);
        stm[3] = vgsd[i].dpix;
        stm[4].fromBlob(gp_);
		stm.execute();
        stm.reset();
	}
	cnn.commit_transaction();
}

void photo_exec::_process_photos()
{
	std::cout << "Elaborazione dei fotogrammi" << std::endl;

	std::string table = std::string(Z_FOTO) + (_type == Prj_type ? "P" : "V");
    cnn.remove_layer(table);

	std::cout << "Layer:" << table << std::endl;

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

	std::vector<GSD> vgsd;
		
	OGRSpatialReference sr;
	sr.importFromEPSG(SRID);

	std::map<std::string, VDP>::iterator it;
	for (it = _vdps.begin(); it != _vdps.end(); it++) {
		VDP& vdp = it->second;
		double dt = 0.;

		DPOINT Pc(vdp.Pc);
		std::vector<DPOINT> dpol;
		bool failed = false;
		for ( int i = 0; i < 5; i++) {
			Collimation ci;
			ci.xi = ( i == 0 || i == 3 ) ? 0.f : (i != 4) ? (float) vdp.dimx() : (float) (vdp.dimx() / 2.);
			ci.yi = ( i == 0 || i == 1 ) ? 0.f : (i != 4) ? (float) vdp.dimy() : (float) (vdp.dimy() / 2.);
			DPOINT pd, pt;
			vdp.GetRay(ci, &pd);
			if ( !ds->RayIntersect(Pc, pd, pt) ) {
				if ( !ds->IsValid(pt.z) ) {
					if (!failed) {
						std::cout << "Il fotogramma " << it->first << " cade al di fuori del dem" << std::endl;
						failed = true;
					}
					continue;
				}
			}
			double dp = vdp.pix() * (vdp.Pc.z - pt.z) / vdp.foc();
			vgsd.push_back(GSD(pt, it->first, dp));

			dt += vdp.Pc.z - pt.z;
			dpol.push_back(pt);
		}
		if (failed) {
			continue;
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
		stm[5] = Conv<Angle_t::DEG>::FromRad(vdp.om);
		stm[6] = Conv<Angle_t::DEG>::FromRad(vdp.fi);
		stm[7].fromBlob(pol);

		stm.execute();
		stm.reset();
	}
	cnn.commit_transaction();
	_process_gsd(vgsd);
}


// builds the models from adjacent Photos of the same strip
void photo_exec::_process_models()
{
	std::cout << "Elaborazione dei modelli" << std::endl;

	std::string table = std::string(Z_MODEL) + (_type == Prj_type ? "P" : "V");
	cnn.remove_layer(table);
	std::cout << "Layer:" << table << std::endl;

	// create the model table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_MODEL_ID TEXT NOT NULL, " <<	// sigla del lavoro
		"Z_MODEL_CS TEXT NOT NULL, " <<		// strisciata
		"Z_MODEL_LEFT TEXT NOT NULL, " <<	// nome foto sinistra
		"Z_MODEL_RIGHT TEXT NOT NULL, " <<	// nome foto destra
		"Z_MODEL_L_OVERLAP INTEGER NOT NULL, " <<	// overlap longitudinale
		"Z_MODEL_T_OVERLAP INTEGER NOT NULL, " <<	// overlap trasversale
        "Z_MODEL_D_HEADING DOUBLE NOT NULL, " <<		// differenza di heading tra i fotogrammi
        "Z_MODEL_USED NUMERIC NOT NULL)";		// model used
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
    sql2 << "INSERT INTO " << table << " (Z_MODEL_ID, Z_MODEL_CS, Z_MODEL_LEFT, Z_MODEL_RIGHT, Z_MODEL_L_OVERLAP, Z_MODEL_T_OVERLAP, Z_MODEL_D_HEADING, Z_MODEL_USED, geom) \
        VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ST_GeomFromWKB(:geom, " << SRID << ") )";
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
		std::string strip = rs[0];
		std::stringstream sql4;
        sql4 << "select Z_FOTO_ID, Z_FOTO_CS, Z_FOTO_NF, Z_FOTO_DIMPIX, Z_FOTO_PITCH, Z_FOTO_ROLL, AsBinary(geom) from " << tablef << " where Z_FOTO_CS=?1";// << strip;
		CV::Util::Spatialite::Statement stm2(cnn);
		stm2.prepare(sql4.str());
        stm2[1] = strip;
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
                Blob blob = rs1[6].toBlob();
                pol1 = blob;
			} else {
				std::string nomeright = rs1["Z_FOTO_NF"];

				// get the next photo geometry
                Blob blob = rs1[6];
                OGRGeomPtr pol2 = blob;
                int res = 1;
				// the model is the intersection of two photos
				OGRGeomPtr mod = pol1->Intersection(pol2);
				if (!mod->IsValid() || mod->IsEmpty() || !mod->Intersect(_carto) ) {
					std::string mod = nomeleft + " - " + nomeright;
                    _useless_models.push_back(mod);
                    res = 0;
				}
		
				double dh = _vdps[nomeleft].ka - _vdps[nomeright].ka;
				if ( dh > M_PI ) {
					dh -= 2 * M_PI;
				} else if ( dh < -M_PI ) {
					dh += 2 * M_PI;
				}
				//dh = Conv<Angle_t::DEG>::FromRad(dh);
				dh = (int)(Conv<Angle_t::DEG>::FromRad(dh) * 1000.) / 1000.;
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
                stm[8] = res;
                stm[9].fromBlob(mod);
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
}
// costruisce le strisciate unendo tutte le foto di una stessa strip
void photo_exec::_process_strips()
{
	std::cout << "Elaborazione delle strisciate" << std::endl;
	std::string table = std::string(Z_STRIP) + (_type == Prj_type ? "P" : "V");
	cnn.remove_layer(table);
	std::cout << "Layer:" << table << std::endl;

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
		"'MULTIPOLYGON'," <<
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
		std::string strip = rs[0];
		std::stringstream sql4;

        sql4 << "select Z_MODEL_ID, Z_MODEL_CS, Z_MODEL_LEFT, Z_MODEL_RIGHT, AsBinary(geom) from " << tablef << " where Z_MODEL_CS=?1";// << strip;
		CV::Util::Spatialite::Statement stm2(cnn);
		stm2.prepare(sql4.str());
        stm2[1] = strip;
		CV::Util::Spatialite::Recordset rs1 = stm2.recordset();
		std::string id;
		OGRGeomPtr pol;
		std::string firstname, lastname;
		bool first = true;
		int count = 1;
		while ( !rs1.eof() ) {
			if ( first ) {
                id = rs1["Z_MODEL_ID"].toString();
                strip = rs1["Z_MODEL_CS"].toString();
                firstname = rs1["Z_MODEL_LEFT"].toString();
                first = false;
                Blob blob = rs1[4].toBlob();
                pol = blob;
			} else {
                lastname = rs1["Z_MODEL_RIGHT"].toString();
				// joins all the models
                Blob blob = rs1[4].toBlob();
                if (blob.size()) {
                    OGRGeomPtr pol2 = blob;
                    pol = pol->Union(pol2);
                }
			}
			count++;
			rs1.next();
		}
		if ( pol->getGeometryType() == wkbPolygon ) {
			OGRSpatialReference sr;
			sr.importFromEPSG(SRID);

			OGRGeometryFactory gf;
			OGRGeomPtr gp_ = gf.createGeometry(wkbMultiPolygon);
			OGRMultiPolygon* gp = (OGRMultiPolygon*) ((OGRGeometry*) gp_);
			gp->setCoordinateDimension(2);
			gp->assignSpatialReference(&sr);
			gp->addGeometry(pol);
			stm[7].fromBlob(gp_);
		} else
			stm[7].fromBlob(pol);

		double l = _vdps[firstname].Pc.dist2D(_vdps[lastname].Pc) / 1000.; // strip length in km
		stm[1] = id;
		stm[2] = strip;
		stm[3] = firstname;
		stm[4] = lastname;
		stm[5] = count;
		stm[6] = l;
		//stm[7].fromBlob(pol);
		stm.execute();
		stm.reset();
		
		rs.next();
	}
	cnn.commit_transaction();
}
typedef struct mstrp {
	std::string strip;
	std::string first;
	std::string last;
	OGRGeomPtr geo;
	bool used;
} mstrp;
void photo_exec::_process_block()
{
	std::cout << "Elaborazione del blocco" << std::endl;

	std::stringstream sql2;
	std::string tablef(_type == Prj_type ? "Z_STRIPP" : "Z_STRIPV");
	
	sql2 << "select Z_STRIP_CS, Z_STRIP_FIRST, Z_STRIP_LAST, AsBinary(geom) from " << tablef << " order by Z_STRIP_CS";
	Statement stm1(cnn);
	stm1.prepare(sql2.str());
	Recordset rs = stm1.recordset();
	
	std::vector<mstrp> vs;

	OGRGeomPtr blk;
	bool first = true;
    while ( !rs.eof() ) { //for every strip
		mstrp s;
        s.strip = rs[0].toString();
        s.first = rs[1].toString();
        s.last = rs[2].toString();
        Blob blob = rs[3].toBlob();
        s.geo = blob;
		if ( first ) {
			blk = s.geo;
			first = false;
		} else {
			blk = blk->Union(s.geo);
		}
		vs.push_back(s);
		rs.next();
    }
	OGRGeomPtr blk1 = blk;

	std::string tableb = std::string(Z_BLOCK) + (_type == Prj_type ? "P" : "V");
    cnn.remove_layer(tableb);
    std::cout << "Layer:" << tableb << std::endl;

	std::stringstream sqla;
	sqla << "CREATE TABLE " << tableb << 
        "(Z_BLOCK_ID TEXT NOT NULL)";	// sigla del lavoro
    cnn.execute_immediate(sqla.str());
	// add the geometry column
	std::stringstream sqlb;
	sqlb << "SELECT AddGeometryColumn('" << tableb << "'," <<
		"'geom'," <<
		SRID << "," <<
        "'" << get_typestring(blk) << "'," <<
        "'XY')";
    cnn.execute_immediate(sqlb.str());
	std::stringstream sqlc;
	sqlc << "INSERT INTO " << tableb << " (Z_BLOCK_ID, geom) VALUES (?1, ST_GeomFromWKB(:geom, " << SRID << ") )";
	Statement stm0(cnn);
	cnn.begin_transaction();
	stm0.prepare(sqlc.str());
	stm0[1] = SIGLA_PRJ;
	stm0[2].fromBlob(blk);
	stm0.execute();
    stm0.reset();
    cnn.commit_transaction();

	OGRGeomPtr dif = _carto->Difference(blk1);
	_uncovered(dif);
	//_get_carto(blk1);

	std::string table = std::string(Z_STR_OVL) + (_type == Prj_type ? "P" : "V");
	cnn.remove_layer(table);

	std::cout << "Calcolo sovrapposizione tra strisciate" << std::endl;

	// create the strip overlap table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_STRIP_ID TEXT NOT NULL, " <<	// sigla del lavoro
		"Z_STRIP1 TEXT NOT NULL, " <<
		"Z_STRIP2 TEXT NOT NULL, " <<
		"Z_STRIP_T_OVERLAP INT NOT NULL)";  // overlap trasversale
	cnn.execute_immediate(sql.str());

	std::stringstream sql3;
	sql3 << "INSERT INTO " << table << " (Z_STRIP_ID, Z_STRIP1, Z_STRIP2, Z_STRIP_T_OVERLAP) VALUES (?1, ?2, ?3, ?4)";
	CV::Util::Spatialite::Statement stm2(cnn);
	cnn.begin_transaction();
	stm2.prepare(sql3.str());

	int k = 1;
	for (size_t i = 0; i < vs.size(); i++) {
		VDP& vdp1 = _vdps[vs[i].first];
		VDP& vdp2 = _vdps[vs[i].last];
		double k1 = vdp2.Pc.angdir2(vdp1.Pc);
		VecOri v1(vdp2.Pc - vdp1.Pc);
		for (size_t j = i + 1; j < vs.size(); j++) {
			VDP& vdp3 = _vdps[vs[j].first];
			VDP& vdp4 = _vdps[vs[j].last];
			VecOri v2(vdp4.Pc - vdp3.Pc);
			double ct = Conv<Angle_t::DEG>::FromRad(acos((v1 % v2) / (v1.module() * v2.module())));
			if ( fabs(ct) < 10 || fabs(ct) > 170 ) { // 10 deg difference in the heading means they are parallel
				OGRGeomPtr g1 = vs[i].geo;
				OGRGeomPtr g2 = vs[j].geo;
				if ( g1->Intersect(g2) ) {
					double d1, d2, d3, d4;
                    _get_elong(g1, k1, &d1, &d2);
					OGRGeomPtr inter = g1->Intersection(g2);
					//if ( inter->getGeometryType() == wkbPolygon ) {
                        _get_elong(inter, k1, &d3, &d4);
						double dt = (int) 100 * (d3 / d1);
						stm2[1] = (int) k++;
						stm2[2] = vs[i].strip;
						stm2[3] = vs[j].strip;
						stm2[4] = dt;
						stm2.execute();
						stm2.reset();
					//}
				}
			}
		}
	}
	cnn.commit_transaction();
}

Poco::Timestamp from_string(const std::string time)
{
	Poco::StringTokenizer tok(time, ":", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	Poco::Timestamp tm;
	if ( tok.count() < 3 )
		return tm;
	double sec = atof(tok[2].c_str());
	double msec = 1000 * ( sec - (int) sec);
	Poco::DateTime dm(2010, 1, 1, atoi(tok[0].c_str()), atoi(tok[1].c_str()), (int) sec, (int) msec);
	return dm.timestamp();
}

void photo_exec::process_end_point_axis_info(const Blob& pt, end_point_axis_info& epai)
{
    OGRGeomPtr gp = pt;
	OGRPoint* pti = (OGRPoint*) ((OGRGeometry*) gp);

	std::stringstream sql;

	sql << "SELECT *, rowid, AsBinary(ST_Transform(geom, " << SRID << ") ) as geo, AsBinary(geom) as geoWGS FROM " << GPS_TABLE_NAME << 
		" WHERE rowid in( select rowid from SpatialIndex where f_table_name='gps' and  search_frame=MakeCircle(ST_X(ST_Transform(ST_GeomFromWKB(?1, " << SRID << ")," << SRIDGEO << ")), ST_Y(ST_Transform(ST_GeomFromWKB(?1, " << SRID << ")," << SRIDGEO << ")), 0.01))";

	Statement stm(cnn);
	stm.prepare(sql.str());

	stm[1].fromBlob(pt);
	Recordset rs = stm.recordset();
	while ( !rs.eof() ) {
		feature f;
		Blob b = rs["geo"].toBlob(); // geometria utm
        OGRGeomPtr g1 = b;
		OGRPoint* pti1 = (OGRPoint*) ((OGRGeometry*) g1);
		
		f.dist = g1->Distance(gp);
        f.mission = rs["MISSION"].toString();
		bool insert = true;
		if ( epai.find(f.mission) != epai.end() ) {
			if ( epai[f.mission].dist < f.dist )
				insert = false;
		}
		if ( insert ) {
			f.time = rs["TIME"].toString();
			f.date = rs["DATE"].toString();
			f.nsat = rs["NSAT"].toInt();
			f.nbasi = rs["NBASI"].toInt();
			f.pdop = rs["PDOP"].toDouble();
			f.pt = g1;

			Blob bwgs = rs["geoWGS"].toBlob();
			OGRGeomPtr gwgs = bwgs;
			f.ptwgs = gwgs;

			epai[f.mission] = f;
		}
		rs.next();
	}
}
void photo_exec::update_strips(std::vector<feature>& ft)
{
	std::string table = ASSI_VOLO + std::string("V");

	std::stringstream sql1;
    sql1 << "UPDATE " << table << " SET MISSION=?1, DATE=?2, TIME_S=?3, TIME_E=?4, NSAT=?5, PDOP=?6, NBASI=?7, SUN_HL=?8, GPS_GAP=?9 where " << STRIP_NAME  << "=?10";
	Statement stm(cnn);
	stm.prepare(sql1.str());
	cnn.begin_transaction();

	// per ogni strip determina i parametri gps con cui è stata acquisita
	for ( size_t i = 0; i < ft.size(); i++) {
		const std::string & val = ft[i].strip;
		std::string t1 = ft[i].time;
		std::string t2 = ft[i].time2;
		if ( t1 > t2 )
			std::swap(t1, t2);

		std::stringstream sql;
		sql << "SELECT MISSION, DATE, TIME, NSAT, PDOP, NBASI from " << GPS_TABLE_NAME << " where TIME >= '" << t1 << "' and TIME <= '" << t2 << "' and MISSION= '" <<
			ft[i].mission << "' ORDER BY TIME";

		Statement stm1(cnn);
		stm1.prepare(sql.str());
		Recordset rs = stm1.recordset();
		bool first = true;
		
		Poco::Timestamp tm0, tm1;
		double dt0 = 0.;
		int nsat, nbasi;
		double pdop;
		while ( !rs.eof() ) {
			if ( first ) {
				tm0 = from_string(rs["TIME"]);
				// determina l'altezza media del sole sull'orizzonte
				OGRPoint* pt = (OGRPoint*) ((OGRGeometry*) ft[i].ptwgs);
				Sun sun(pt->getY(), pt->getX());
				int td;
				std::stringstream ss2;
				ss2 << ft[i].date << " " << t1;
				Poco::DateTime dt = Poco::DateTimeParser::parse(ss2.str(), td);
				sun.calc(dt.year(), dt.month(), dt.day(), dt.hour());
				double h = sun.altit();

				stm[1] = (std::string const &) rs["MISSION"].toString(); // mission
				stm[2] = rs["DATE"].toString(); // date
				stm[3] = t1; // istante di inizio acquisizione
				stm[4] = t2; // istante di fine acquisizione

				nsat = rs["NSAT"].toInt();
				pdop = rs["PDOP"].toDouble();
				nbasi = rs["NBASI"].toInt();

				stm[8] = h;	// sun elevation
				stm[10] = val;
				first = false;
			} else {
				nsat = std::min(nsat, rs["NSAT"].toInt());
				pdop = std::max(pdop, rs["PDOP"].toDouble());
				nbasi = std::min(nbasi, rs["NBASI"].toInt());
				tm1 = from_string(rs["TIME"]);
				double dt = (double) (tm1 - tm0) / 1000000;
				tm0 = tm1;
				if ( dt > dt0 )
					dt0 = dt;
			}
			rs.next();
		}
		if ( first )
			return;
		stm[5] = nsat; // minimal number of satellite
		stm[6] = pdop; // max pdop
		stm[7] = nbasi; // number of bases
		stm[9] = (int) dt0;
		stm.execute();
		stm.reset();
	}
	cnn.commit_transaction();
}
bool photo_exec::select_mission(end_point_axis_info& ep1, end_point_axis_info& ep2, feature& f, double len)
{
	bool ret = false;
	end_point_axis_info::const_iterator it1 = ep1.begin();
	end_point_axis_info::const_iterator it2 = ep2.begin();
	double d0 = 1e30;
	double v0 = 10000;
	for ( it1 = ep1.begin(); it1 != ep1.end(); it1++) {
		for ( it2 = ep2.begin(); it2 != ep2.end(); it2++) {
			if ( it1->first == it2->first ) {
				// same mission
				double d1 = it1->second.dist;
				double d2 = it2->second.dist;
				double d = sqrt(d1 * d1 + d2 * d2);

				Poco::DateTime dt1, dt2;
				int tm = 0;
				Poco::DateTimeParser::parse("%H:%M:%s", it1->second.time, dt1, tm);
				Poco::DateTimeParser::parse("%H:%M:%s", it2->second.time, dt2, tm);
				Poco::Timespan tspan = dt2 > dt1 ? dt2 - dt1 : dt1 - dt2;
				double dtime = tspan.totalMilliseconds() / 1000.;
				double v = fabs(3.6 * len / dtime - 250); 
				//if ( fabs(d - d0) < 10 ) {
				if ( v < v0 ) {
					f = it1->second;
					f.time2 = it2->second.time;
					d0 = d;
					v0 = v;
					ret = true;
				}
				break;
			}
		}
	}
	return ret;
}
void photo_exec::_update_assi_volo()
{
	std::stringstream sqla;
	sqla << "SELECT CreateSpatialIndex('" << GPS_TABLE_NAME << "', 'geom')";
	//sqla << "SELECT DisableSpatialIndex('" << GPS_TABLE_NAME << "', 'geom')";
	cnn.execute_immediate(sqla.str());

	std::cout << "Associazione della traccia GPS con gli assi di volo" << std::endl;

	std::string table = ASSI_VOLO + std::string("V");

	// add the columns for the gps data
	add_column(cnn, table, "DATE TEXT");
	add_column(cnn, table, "TIME_S TEXT");
	add_column(cnn, table, "TIME_E TEXT");
	add_column(cnn, table, "MISSION TEXT");
	add_column(cnn, table, "SUN_HL DOUBLE");
	add_column(cnn, table, "NBASI INTEGER");
	add_column(cnn, table, "NSAT INTEGER");
	add_column(cnn, table, "PDOP DOUBLE");
	add_column(cnn, table, "GPS_GAP INTEGER");

	std::stringstream sqli;
	sqli << "SELECT " << STRIP_NAME << ", ST_Length(geom), AsBinary(StartPoint(geom)), AsBinary(EndPoint(geom)) FROM "  << table;
	Statement stm1(cnn);
	stm1.prepare(sqli.str());
	Recordset rs1 = stm1.recordset();

	// for every strip get the GPS time of the first point
	std::vector<feature> ft;
	while ( !rs1.eof() ) {
		end_point_axis_info ep1;
		end_point_axis_info ep2;
		double len = rs1[1].toDouble();
		process_end_point_axis_info(rs1[2].toBlob(), ep1);
		process_end_point_axis_info(rs1[3].toBlob(), ep2);

		std::string s = rs1[0].toString();
		if ( ep1.size() >  1 || ep2.size() > 1 )
			int a = 1;

		feature f;
		if ( select_mission(ep1, ep2, f, len) ) {
			f.strip = rs1[0].toString();
			ft.push_back(f);
		}
		rs1.next();
	}
	update_strips(ft);


	//// query to associate to the first and last point of each flight line the nearest point of the gps track
	//std::stringstream sql;
	//sql << "SELECT a." << STRIP_NAME << " as strip, b.*, AsBinary(b.geom) as geo, min(st_Distance(st_PointN(ST_Transform(a.geom," << SRIDGEO << "), ?1), b.geom)) FROM " <<
 //       table << " a, GPS b group by strip";

	//Statement stm(cnn);
	//stm.prepare(sql.str());

	//stm[1] = 1;
	//Recordset rs = stm.recordset();

	//// for every strip get the GPS time of the first point
	//std::vector<feature> ft1;
	//while ( !rs.eof() ) {
	//	feature f;
 //       f.strip = rs["strip"].toString();
 //       f.mission = rs["MISSION"].toString();
 //       f.time = rs["TIME"].toString();
 //       f.date = rs["DATE"].toString();
 //       f.nsat = rs["NSAT"].toInt();
 //       f.nbasi = rs["NBASI"].toInt();
 //       f.pdop = rs["PDOP"].toDouble();
 //       Blob blob = rs["geo"].toBlob();
 //       f.pt = blob;
	//	ft1.push_back(f);
	//	rs.next();
	//}
	//stm.reset();
	//stm[1] = 2;
	//rs = stm.recordset();

	//// for every strip get the GPS time of the last point
	//std::vector<feature> ft2;
	//while ( !rs.eof() ) {
	//	feature f;
 //       f.strip = rs["strip"].toString();
 //       f.mission = rs["MISSION"].toString();
 //       f.time = rs["TIME"].toString();
 //       f.date = rs["DATE"].toString();
 //       f.nsat = rs["NSAT"].toInt();
 //       f.nbasi = rs["NBASI"].toInt();
 //       f.pdop = rs["PDOP"].toDouble();
 //       Blob blob = rs["geo"].toBlob();
 //       f.pt = blob;
	//	ft2.push_back(f);
	//	rs.next();
	//}

	//std::stringstream sql1;
 //   sql1 << "UPDATE " << table << " SET MISSION=?1, DATE=?2, TIME_S=?3, TIME_E=?4, NSAT=?5, PDOP=?6, NBASI=?7, SUN_HL=?8, GPS_GAP=?9 where " << STRIP_NAME  << "=?10";
	//Statement stm1(cnn);
	//stm1.prepare(sql1.str());
	//cnn.begin_transaction();

	//// per ogni strip determina i parametri gps con cui è stata acquisita
	//for ( size_t i = 0; i < ft1.size(); i++) {
	//	const std::string & val = ft1[i].strip;
	//	std::string t1 = ft1[i].time;
	//	for ( size_t j = 0; j < ft2.size(); j++) {
	//		if ( ft2[j].strip == ft1[i].strip ) {
	//			if ( ft2[j].mission != ft1[i].mission ) {
	//				std::cout << "strip " << ft1[i].strip << " non correttamente abbinata alla traccia GPS" << std::endl;
	//				continue;
	//			}
	//			std::string t2 = ft2[j].time;
	//			if ( t1 > t2 )
	//				std::swap(t1, t2);

	//			std::stringstream sql;
	//			sql << "SELECT MISSION, DATE, TIME, NSAT, PDOP, NBASI from " << GPS_TABLE_NAME << " where TIME >= '" << t1 << "' and TIME <= '" << t2 << "' and MISSION= '" <<
	//				ft1[i].mission << "' ORDER BY TIME";
	//			stm.prepare(sql.str());
	//			rs = stm.recordset();
	//			bool first = true;
	//			
	//			Poco::Timestamp tm0, tm1;
	//			double dt0 = 0.;
	//			int nsat, nbasi;
	//			double pdop;
	//			while ( !rs.eof() ) {
	//				if ( first ) {
	//					tm0 = from_string(rs["TIME"]);
	//					// determina l'altezza media del sole sull'orizzonte
	//					OGRPoint* pt = (OGRPoint*) ((OGRGeometry*) ft1[i].pt);
	//					Sun sun(pt->getY(), pt->getX());
	//					int td;
	//					std::stringstream ss2;
	//					ss2 << ft1[i].date << " " << t1;
	//					Poco::DateTime dt = Poco::DateTimeParser::parse(ss2.str(), td);
	//					sun.calc(dt.year(), dt.month(), dt.day(), dt.hour());
	//					double h = sun.altit();

	//					stm1[1] = (std::string const &) rs["MISSION"]; // mission
	//					stm1[2] = rs["DATE"].toString(); // date
	//					stm1[3] = t1; // istante di inizio acquisizione
	//					stm1[4] = t2; // istante di fine acquisizione

	//					nsat = rs["NSAT"].toInt();
	//					pdop = rs["PDOP"].toDouble();
	//					nbasi = rs["NBASI"].toInt();

	//					//stm1[5] = rs["NSAT"].toInt(); // minimal number of satellite
	//					//stm1[6] = rs["PDOP"].toDouble(); // max pdop
	//					//stm1[7] = rs["NBASI"].toInt(); // number of bases
	//					stm1[8] = h;	// sun elevation
	//					stm1[10] = val;
	//					//stm1.execute();
	//					//stm1.reset();
	//					first = false;
	//					//continue;
	//				} else {
	//					nsat = std::min(nsat, rs["NSAT"].toInt());
	//					pdop = std::max(pdop, rs["PDOP"].toDouble());
	//					nbasi = std::min(nbasi, rs["NBASI"].toInt());
	//					tm1 = from_string(rs["TIME"]);
	//					double dt = (double) (tm1 - tm0) / 1000000;
	//					tm0 = tm1;
	//					if ( dt > dt0 )
	//						dt0 = dt;
	//				}
	//				rs.next();
	//			}
	//			if ( first )
	//				return;
	//			stm1[5] = nsat; // minimal number of satellite
	//			stm1[6] = pdop; // max pdop
	//			stm1[7] = nbasi; // number of bases
	//			stm1[9] = (int) dt0;
	//			stm1.execute();
	//			stm1.reset();
	//		}
	//	}
	//}
	//cnn.commit_transaction();

}
