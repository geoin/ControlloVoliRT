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
#include "Poco/stringtokenizer.h"
#include "Poco/AutoPtr.h"
#include "Poco/Path.h"
#include "Poco/sharedPtr.h"
#include <fstream>
#include <sstream>
#include "ogr_geometry.h"
#include "dem_interpolate/dsm.h"
#include "common/util.h"

#define SRID 32632
#define SIGLA_PRJ "CSTP"
#define REFSCALE "RefScale_2000"
#define GEO_DB_NAME "geo.sqlite"
#define DEM "dem.asc"
#define ASSI_VOLO "lvolo"
#define SIGLA_PRJ "CSTP"

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
	return std::string(REFSCALE) + "." + val;
}
std::string get_strip(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() != 2 )
		return "";
	return tok[0];
}
/***************************************************************************************/
lidar_exec::~lidar_exec() 
{
}
bool lidar_exec::run()
{
	// Read the reference values
	_read_ref_val();

	// initialize docbook xml file
	_init_document();

		Path geodb(_proj_dir, GEO_DB_NAME);
		cnn.create( geodb.toString() ); // Create or open spatialite db
		if ( cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA )
			cnn.initialize_metdata(); // Initialize metadata (if already initialized noop)

		Path avolo(_proj_dir, "assi volo");
		avolo = Path(avolo.toString(), "lvolo");
		int nrows = cnn.load_shapefile(avolo.toString(),
			"lvolop",
                   "CP1252",
                   32632,
                   "geom",
                   true,
                   false,
                   false);

		Path carto(_proj_dir, "aree da cartografare");
		carto = Path(carto.toString(), "new1mas-car");
		nrows = cnn.load_shapefile(carto.toString(),
			"carto",
                   "CP1252",
                   32632,
                   "geom",
                   true,
                   false,
                   false);


	// dagli assi di volo e dai parameti del lidar ricava l'impronta al suolo delle strip
	_read_lidar();
	// read digital terrain model
	_dem_name = Path(_proj_dir, DEM).toString();
	if ( !_read_dem() )
		throw std::runtime_error("Modello numerico non trovato");

	_process_strips();
	_process_block();

	// se volo lidar confronta gli assi progettati con quelli effettivi

	// verifica che le strip ricoprano l'area da cartografare

	// determina il ricoprimento tra strisciate e la loro lunghezza, li confronta con i valori di riferimento

	// se disponibile la velocit� calcola la densit� media dei pt

	// per il volo verifica i punti dell'area di test


	// write the result on the docbook report
	_dbook.write();

	return true;
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
	ref_file.popDirectory();
	ref_file.setFileName("Regione_Toscana_RefVal.xml");
	AutoPtr<XMLConfiguration> pConf;
	try {
		pConf = new XMLConfiguration(ref_file.toString());
		// toll of projection center
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
	std::string lidar_name = Path(_proj_dir, "lidar.xml").toString();
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
	return true;
}
bool lidar_exec::_read_dem()
{
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
	sql2 << "INSERT INTO " << table << " (Z_STRIP_ID, Z_STRIP_CS, Z_MISSION, geom) \
		VALUES (?1, ?2, ?3, ST_GeomFromWKB(:geom, " << SRID << ") )";
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
		OGRGeomPtr pol = (Blob) rs["geom"];
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
		stm[4].fromBlob(rg);
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
	sql1 << "SELECT AsBinary(geom) geom from " << table;
	Statement stm1(cnn);
	stm1.prepare(sql1.str());
	Recordset rs = stm1.recordset();

	std::cout << "Layer:" << table << std::endl;

	OGRGeomPtr blk;
	bool first = true;
	while ( !rs.eof() ) {
		OGRGeomPtr pol = (Blob) rs["geom"];
		if ( first ) {
			blk = pol;
			first = false;
		} else 
			blk = blk->Union(pol);
		rs.next();
	}
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
	OGRGeomPtr blk;
	bool first = true;
	while ( !rs.eof() ) {
		OGRGeomPtr pol = (Blob) rs["geom"];
		if ( first ) {
			blk = pol;
			first = false;
		} else 
			blk = blk->Union(pol);
		rs.next();
	}

	// select data from strips block
	table = std::string("BLOCK") + (_type == Prj_type ? "P" : "V");
	std::stringstream sql2;
	sql2 << "SELECT AsBinary(geom) geom from " << table;
	Statement stm2(cnn);
	stm2.prepare(sql2.str());
	rs = stm2.recordset();
	OGRGeomPtr cart = (Blob) rs["geom"];

	if ( !cart->Intersect(blk) )
		return;
	OGRPolygon* p2 = (OGRPolygon*) ((OGRGeometry*) dif);

	std::string tabled = std::string("DIFF") + (_type == Prj_type ? "P" : "V");
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