/*
    File: lidar_exec.cpp
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
#include "check_lidar.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/StringTokenizer.h"
#include "Poco/AutoPtr.h"
#include "Poco/Path.h"
#include "Poco/SharedPtr.h"
#include <fstream>
#include <sstream>
#include "gdal/ogr_geometry.h"
#include "dem_interpolate/dsm.h"
#include "common/util.h"

#include <iostream>

#define SRID 32632
#define SIGLA_PRJ "CSTP"
#define REFSCALE "RefScale_2000"
#define GEO_DB_NAME "geo.sqlite"
#define SIGLA_PRJ "CSTP"
#define LIDAR "Lidar"

#define OUT_DOCV "check_lidarV.xml"
#define OUT_DOCP "check_lidarP.xml"

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::SharedPtr;
using Poco::Path;
using namespace CV::Util::Spatialite;
using namespace CV::Util::Geometry;
/**************************************************************************/
typedef std::vector<unsigned char> Blob;


std::string get_key(const std::string& val)
{
	return std::string(LIDAR) + "." + val;
}

/***************************************************************************************/
lidar_exec::~lidar_exec() 
{
}
bool lidar_exec::run()
{
	try {
		if ( _proj_dir.empty() ) {
			throw std::runtime_error("cartella di lavoro non impostata");
		}

		Path geodb(_proj_dir, GEO_DB_NAME);
		cnn.open( geodb.toString() ); // Create or open spatialite db

		// Read the reference values
		std::string norefscale;
		if ( !GetProjData(cnn, _note, norefscale) ) {
			throw std::runtime_error("dati progetto incompleti");
		}

		_read_ref_val();

		// initialize docbook xml file
		std::string title =_type == fli_type ? "Collaudo rilievo lidar" : "Collaudo progetto di rilievo lidar";
		Path doc_file(_proj_dir, "*");
		doc_file.setFileName(_type == fli_type ? OUT_DOCV : OUT_DOCP);

		init_document(_dbook, doc_file.toString(), title, _note);
		char* dtd_ = getenv("DOCBOOKRT");
		std::string dtd;
		if ( dtd_ != NULL )
			dtd = std::string("file:") + dtd_;
		_dbook.set_dtd(dtd);
		_article = _dbook.get_item("article");

		// dagli assi di volo e dai parameti del lidar ricava l'impronta al suolo delle strip
		_read_lidar();
		// read digital terrain model
		//_dem_name = Path(_proj_dir, DEM).toString();
		if ( !_read_dem() )
			throw std::runtime_error("Modello numerico non trovato");

		_process_strips();
		_process_block();

		// se volo lidar confronta gli assi progettati con quelli effettivi

		// verifica che le strip ricoprano l'area da cartografare

		// determina il ricoprimento tra strisciate e la loro lunghezza, li confronta con i valori di riferimento

		// se disponibile la velocità calcola la densità media dei pt

		// per il volo verifica i punti dell'area di test


		std::cout << "Produzione del report finale: " << _dbook.name() << std::endl;
		_final_report();

		// write the result on the docbook report
		_dbook.write();
		std::cout << "Procedura terminata:" << std::endl;

		return true;
	} catch(const std::exception& ex) {
		std::string msg = ex.what();
		return false; //TODO
	}
}

void lidar_exec::_final_report() {

}

void lidar_exec::set_proj_dir(const std::string& nome)
{
	_proj_dir = nome;
}
void lidar_exec::set_checkType(Check_Type t)
{
	_type = t;
}
bool lidar_exec::_read_ref_val()
{
    Path ref_file(_proj_dir, "*");
    ref_file.setFileName("refval.xml");
	AutoPtr<XMLConfiguration> pConf;
	try {
		pConf = new XMLConfiguration(ref_file.toString());
		STRIP_OVERLAP = pConf->getInt(get_key("STRIP_OVERLAP"));
		STRIP_OVERLAP_RANGE = pConf->getInt(get_key("STRIP_OVERLAP_RANGE"));
		MAX_STRIP_LENGTH = pConf->getInt(get_key("MAX_STRIP_LENGTH"));
		//_T_CP = pConf->getDouble(get_key("T_CP"));

	} catch (...) {
		return false;
	}
	return true;
}
void lidar_exec::_init_document()
{
	Path doc_file(_proj_dir, "*");
	doc_file.setFileName("check_ta.xml");
	_dbook.set_name(doc_file.toString());
	
	_article = _dbook.add_item("article");
	_article->add_item("title")->append("Collaudo triangolazione aerea");
}
Doc_Item lidar_exec::_initpg1()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica punti appoggio");

	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
	//ss << "Tolleranza planimetrica " << _TP_PA << " m";
	//itl->add_item("listitem")->append(ss.str());
	//std::stringstream ss1;
	//ss1 << "Tolleranza altimetrica " << _TA_PA << " m";
	//itl->add_item("listitem")->append(ss1.str());
	
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("scarti tra valori nominali e valori misurati");
	
	Poco::XML::AttributesImpl attrs;
	attrs.addAttribute("", "", "cols", "", "6");
	tab = tab->add_item("tgroup", attrs);
	
	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");

	attrs.clear();
	attrs.addAttribute("", "", "align", "", "center");
	row->add_item("entry", attrs)->append("Codice");
	row->add_item("entry", attrs)->append("Foto Sx");
	row->add_item("entry", attrs)->append("Foto Dx");
	row->add_item("entry", attrs)->append("sc X");
	row->add_item("entry", attrs)->append("sc Y");
	row->add_item("entry", attrs)->append("Sc Z");

	Doc_Item tbody = tab->add_item("tbody");
	return tbody;
}


bool lidar_exec::_read_lidar()
{
    /*std::string lidar_name = Path(_proj_dir, "lidar.xml").toString();
	AutoPtr<XMLConfiguration> pConf;
	try {
		pConf = new XMLConfiguration(lidar_name);
		_lidar.fov = atof(pConf->getString("FOV").c_str());
		_lidar.ifov = atof(pConf->getString("IFOV").c_str());
		_lidar.freq = atof(pConf->getString("FREQ").c_str());
		_lidar.scan = atof(pConf->getString("SCAN_RATE").c_str());
	} catch (...) {
		return false;
	}
    return true;*/

    try {
        CV::Util::Spatialite::Statement stm(cnn);

        std::stringstream query;
        query << "select FOV, IFOV, FREQ, SCAN_RATE from SENSOR where PLANNING = ?1";
        stm.prepare(query.str());
		stm[1] = 1;
        CV::Util::Spatialite::Recordset set = stm.recordset();
        if (set.eof()) {
            return false;
        }
        _lidar.fov = set[0].toDouble();
        _lidar.ifov = set[1].toDouble();
        _lidar.freq = set[2].toDouble();
        _lidar.scan = set[3].toDouble();
     } catch (CV::Util::Spatialite::spatialite_error& err) {
        (void*)&err;
        return false;
     }

    return true;
}

bool lidar_exec::_read_dem() {
    CV::Util::Spatialite::Statement stm(cnn);

    std::stringstream query;
    query << "select URI from DEM";
    stm.prepare(query.str());
    CV::Util::Spatialite::Recordset set = stm.recordset();
    if (set.eof()) {
        return false;
    }
    _dem_name = Path(_proj_dir, set[0].toString()).toString();

	_df = new DSM_Factory;
	if ( !_df->Open(_dem_name, false) )
		return false;
	return true;
}
void lidar_exec::_process_strips()
{
	// create tthe table for the strip footprint
	std::string table = std::string("STRIP") + (_type == Prj_type ? "P" : "V");
	cnn.remove_layer(table);
	std::cout << "Layer:" << table << std::endl;

	// create the model table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_STRIP_ID TEXT NOT NULL, " <<	// sigla del lavoro
		"Z_STRIP_CS TEXT NOT NULL, " <<		// strisciata
		"Z_STRIP_YAW FLOAT NOT NULL, " <<		// angolo
		"Z_MISSION TEXT NOT NULL)";	// overlap longitudinale
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
	sql2 << "INSERT INTO " << table << " (Z_STRIP_ID, Z_STRIP_CS, Z_MISSION, Z_STRIP_YAW, geom) \
		VALUES (?1, ?2, ?3, ?4, ST_GeomFromWKB(:geom, " << SRID << ") )";
	Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());

	// select data from flight lines
	table = std::string(ASSI_VOLO) + (_type == Prj_type ? "P" : "V");
	std::stringstream sql3;
	sql3 << "SELECT A_VOL_QT, A_VOL_CS, mission, AsBinary(geom) geom from " << table;
	Statement stm1(cnn);
	stm1.prepare(sql3.str());
	Recordset rs = stm1.recordset();

	std::cout << "Layer:" << table << std::endl;

	DSM* ds = _df->GetDsm();

	double teta2 = DEG_RAD(_lidar.fov / 2.);

	while ( !rs.eof() ) {
        Blob blob =  rs["geom"].toBlob();
        OGRGeomPtr pol = blob;
		OGRPoint* p0 = NULL;
		OGRPoint* p1 = NULL;

		OGRGeometry* og = (OGRGeometry*) pol;
		OGRLineString* ls = (OGRLineString*)og;
		int n = ls->getNumPoints();
		if ( n != 2 ) // each line must have only two points
			throw std::runtime_error("asse di volo non valido");

		double z = rs[0];
		DPOINT pt0(ls->getX(0), ls->getY(0), z);
		DPOINT pt1(ls->getX(1), ls->getY(1), z);
		double k = pt1.angdir(pt0);
		std::string strip = rs[1];
		std::string mission = rs[2];
		MatOri m(0, 0, -k);

		OGRGeometryFactory gf;
		OGRGeomPtr gp_ = gf.createGeometry(wkbLinearRing);

		OGRLinearRing* gp = (OGRLinearRing*) ((OGRGeometry*) gp_);
		gp->setCoordinateDimension(2);

		OGRSpatialReference sr;
		sr.importFromEPSG(SRID);
		gp->assignSpatialReference(&sr);

		for ( int i = 0; i < 4; i++) {
			DPOINT pa = ( i < 2 ) ? pt0 : pt1;
			double x = ( i == 0 || i == 3 ) ? -tan(teta2) : tan(teta2);
			DPOINT pd(x, 0, -1);
			pd = m * pd;
			//pd = pd - pa;

			DPOINT pt;
			if ( !ds->RayIntersect(pa, pd, pt) ) {
				if ( !ds->IsInside(pt.z) ) {
					std::stringstream ss;
					ss << "la strisciata " << strip << " della missione " << mission << " cade al di fuori del dem";
					throw std::runtime_error(ss.str());
				}
			}
			gp->addPoint(pt.x, pt.y);
		}
		gp->closeRings();

		OGRGeomPtr rg = gf.createGeometry(wkbPolygon);
		OGRPolygon* p = (OGRPolygon*) ((OGRGeometry*) rg);
		p->setCoordinateDimension(2);
		p->assignSpatialReference(&sr);
		p->addRing(gp);

		stm[1] = SIGLA_PRJ;
		stm[2] = strip;
		stm[3] = mission;
		stm[4] = RAD_DEG(-k);
		stm[5].fromBlob(rg);
		stm.execute();
		stm.reset();

		rs.next();
	}
	cnn.commit_transaction();
}

void lidar_exec::_process_block()
{
	// select data from flight lines
	std::string table = std::string("STRIP") + (_type == Prj_type ? "P" : "V");
	std::stringstream sql1;
	sql1 << "SELECT AsBinary(geom) geom, Z_STRIP_YAW, Z_STRIP_CS from " << table;
	Statement stm1(cnn);
	stm1.prepare(sql1.str());
	Recordset rs = stm1.recordset();

	std::cout << "Layer:" << table << std::endl;

	OGRGeomPtr blk;
	bool first = true;
	std::map<std::string, StripRec> rec;
	while ( !rs.eof() ) {
		StripRec r;
        Blob blob = rs["geom"].toBlob();
        r.geom = blob;
		r.yaw = rs["Z_STRIP_YAW"].toDouble();
		r.name = rs["Z_STRIP_CS"].toString();
		rec.insert(std::pair<std::string, StripRec>(r.name, r));
		if ( first ) {
			blk = r.geom;
			first = false;
		} else {
			blk = blk->Union(r.geom);
		}
		rs.next();
	}

	_get_overlaps(rec);

	std::string tableb = std::string("BLOCK") + (_type == Prj_type ? "P" : "V");
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

	_get_dif();
}

void lidar_exec::_get_overlaps(const std::map<std::string, StripRec>& rec) {
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

	sql.str("");

	sql << "INSERT INTO " << table << " (Z_STRIP_ID, Z_STRIP1, Z_STRIP2, Z_STRIP_T_OVERLAP) VALUES (?1, ?2, ?3, ?4)";
	CV::Util::Spatialite::Statement stm(cnn);
	cnn.begin_transaction();
	try {
		stm.prepare(sql.str());

		std::map<std::string, StripRec>::const_iterator it = rec.begin();
		std::map<std::string, StripRec>::const_iterator end = rec.end();
		
		int k = 0;
		for (; it != end; it++) {
			const StripRec& source = (*it).second;
			std::map<std::string, StripRec>::const_iterator next = it;
			for (next++; next != end; next++) {
				const StripRec& target = (*next).second;
				double diff = source.yaw - target.yaw;

				diff = fabs(diff > 180 ? 360 - diff : diff);

				if (diff < 10 || diff > 170) {
					OGRGeomPtr sourceGeom = source.geom;
					OGRGeomPtr targetGeom = target.geom;
					if (sourceGeom->Intersect(targetGeom)) {
						OGRGeomPtr intersection = sourceGeom->Intersection(targetGeom);
						if (intersection->getGeometryType() == wkbPolygon) {
							double srcMajorAxis, srcMinorAxis;
							get_elong(sourceGeom, DEG_RAD(source.yaw), &srcMajorAxis, &srcMinorAxis);
							
							double targetMajorAxis, targetMinorAxis;
							get_elong(intersection, DEG_RAD(target.yaw), &targetMajorAxis, &targetMinorAxis);

							double dt = (int) 100 * (targetMajorAxis / srcMajorAxis);
							stm[1] = ++k;
							stm[2] = source.name;
							stm[3] = target.name;
							stm[4] = dt;
							stm.execute();
							stm.reset();
						}
					}
				}
			}
		}
	} catch (const std::exception& ex) {
		cnn.rollback_transaction();
	}
	cnn.commit_transaction();
}

void lidar_exec::_get_dif()
{
	// select data from carto areas
	std::string table = std::string("carto");
	std::stringstream sql1;
	sql1 << "SELECT AsBinary(geom) geom from " << table;
	Statement stm1(cnn);
	stm1.prepare(sql1.str());
	Recordset rs = stm1.recordset();

	std::cout << "Layer:" << table << std::endl;

	// buid a unique feature
    OGRGeomPtr carto;
	bool first = true;
	while ( !rs.eof() ) {
        Blob blob = rs["geom"].toBlob();
        OGRGeomPtr pol = blob;
		if ( first ) {
            carto = pol;
			first = false;
		} else 
            carto = carto->Union(pol);
		rs.next();
	}

	// select data from strips block
	table = std::string("BLOCK") + (_type == Prj_type ? "P" : "V");
	std::stringstream sql2;
	sql2 << "SELECT AsBinary(geom) geom from " << table;
	Statement stm2(cnn);
	stm2.prepare(sql2.str());
	rs = stm2.recordset();
    Blob blob = rs["geom"].toBlob();
    OGRGeomPtr block = blob;

    if ( !block->Intersect(carto) ) {
		return;
    }

    OGRGeomPtr dif = carto->Difference(block);
    if (dif->IsEmpty()) { //tutte le aree sono coperte
        return;
    }
	stm2.reset();

    //OGRPolygon* diffPolygon = (OGRPolygon*) ((OGRGeometry*) dif);
    std::string tabled = std::string("UNCOVERED_AREA") + (_type == Prj_type ? "P" : "V");
	cnn.remove_layer(tabled);
	std::cout << "Layer:" << tabled << std::endl;

	std::stringstream sqla;
	sqla << "CREATE TABLE " << tabled << 
		"(DIFF_ID TEXT NOT NULL)";	// sigla del lavoro
	cnn.execute_immediate(sqla.str());
	// add the geometry column
	std::stringstream sqlb;
	sqlb << "SELECT AddGeometryColumn('" << tabled << "'," <<
		"'geom'," <<
		SRID << "," <<
		"'" << get_typestring(dif) << "'," <<
		"'XY')";
	cnn.execute_immediate(sqlb.str());
	
	std::stringstream sqlc;
	sqlc << "INSERT INTO " << tabled << " (DIFF_ID, geom) VALUES (?1, ST_GeomFromWKB(:geom, " << SRID << ") )";
	Statement stm0(cnn);
	cnn.begin_transaction();
	stm0.prepare(sqlc.str());
	stm0[1] = SIGLA_PRJ;
	stm0[2].fromBlob(dif);
	stm0.execute();
	stm0.reset();
	cnn.commit_transaction();
}
