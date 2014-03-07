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

using namespace CV;
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

		// dagli assi di volo e dai parameti del lidar ricava l'impronta al suolo delle strip
		
		if ( _type == FLY_TYPE ) {
			_read_lidar_from_mission();
		} else {
			_read_lidar();
		}
		// read digital terrain model
		//_dem_name = Path(_proj_dir, DEM).toString();
		if ( !_read_dem() )
			throw std::runtime_error("Modello numerico non trovato");

		_process_strips();
		_process_block();
		
		if ( _type == FLY_TYPE ) {
			_compare_axis();
			_update_assi_volo();
		}

		// se volo lidar confronta gli assi progettati con quelli effettivi

		// verifica che le strip ricoprano l'area da cartografare

		// determina il ricoprimento tra strisciate e la loro lunghezza, li confronta con i valori di riferimento

		// se disponibile la velocità calcola la densità media dei pt

		// per il volo verifica i punti dell'area di test

		// initialize docbook xml file
		std::string title =_type == FLY_TYPE ? "Collaudo rilievo lidar" : "Collaudo progetto di rilievo lidar";
		Path doc_file(_proj_dir, "*");
		doc_file.setFileName(_type == FLY_TYPE ? OUT_DOCV : OUT_DOCP);

		init_document(_dbook, doc_file.toString(), title, _note);
		char* dtd_ = getenv("DOCBOOKRT");
		std::string dtd;
		if ( dtd_ != NULL )
			dtd = std::string("file:") + dtd_;
		_dbook.set_dtd(dtd);
		_article = _dbook.get_item("article");

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

void lidar_exec::_compare_axis() {
	std::stringstream sql;
    sql << "SELECT A_VOL_CS, mission, A_VOL_QT, AsBinary(GEOM) as GEOM FROM AVOLOP";
    Statement stm(cnn);
    stm.prepare(sql.str());
    Recordset rs = stm.recordset();

	std::vector<CV::Lidar::Axis::Ptr> _projectAxis;
	while (!rs.eof()) {
		Blob b = rs["GEOM"].toBlob();
		OGRGeomPtr g = b;
		CV::Lidar::Axis::Ptr axis(new CV::Lidar::Axis(g, rs["A_VOL_QT"].toDouble()));
		axis->stripName(rs["A_VOL_CS"].toString());
		axis->missionName(rs["mission"].toString());
		_projectAxis.push_back(axis);
		rs.next();
	}

	if (_projectAxis.size() != _strips.size()) {
		std::cout << "Error in project axis number" << std::endl;
	}

	std::map<std::string, CV::Lidar::Strip::Ptr>::const_iterator it = _strips.begin();
	std::map<std::string, CV::Lidar::Strip::Ptr>::const_iterator end = _strips.end();
	for (; it != end; it++) {
		CV::Lidar::Axis::Ptr stripAxis = (*it).second->axis();
		std::vector<CV::Lidar::Axis::Ptr>::const_iterator tIt = _projectAxis.begin();
		std::vector<CV::Lidar::Axis::Ptr>::const_iterator tEnd = _projectAxis.end();
		for (; tIt != tEnd; ++tIt) {
			CV::Lidar::Axis::Ptr tAxis = (*tIt);
			if (stripAxis->first().dist2D(tAxis->first()) < 10 && stripAxis->last().dist2D(tAxis->last()) < 10) {
				std::cout << stripAxis->stripName() << " equal " << tAxis->stripName() << std::endl; 
			}
		}
	}
}

void lidar_exec::_final_report() {
    if ( _type == FLY_TYPE ) {
        //fly specific data
    }

    //controlo del ricoprimento delle aree da rilevare
    std::stringstream sql;
    std::string table = std::string(Z_UNCOVER) + (_type == PRJ_TYPE ? "P" : "V");
    sql << "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='" << table << "'";
    Statement stm(cnn);
    stm.prepare(sql.str());
    Recordset rs = stm.recordset();

    Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Verifica copertura aree da rilevare");

    int cv = rs[0];
    if ( cv == 0 ) {
        sec->add_item("para")->append("Tutte le aree da rilevare sono state ricoperte da modelli ...");
    } else {
        sec->add_item("para")->append("Esistono delle aree da rilevare non completamente ricoperte da modelli ...");
    }
    stm.reset();

    // verifica ricoprimento tra strisciate
    // verifica lunghezza strisciate
    _strip_report();
}

void lidar_exec::_strip_report() {
    Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Verifica parametri strisciate");

    double minVal = STRIP_OVERLAP * (1 - STRIP_OVERLAP_RANGE / 100.0);
    double maxVal = STRIP_OVERLAP * (1 + STRIP_OVERLAP_RANGE / 100.0);

    sec->add_item("para")->append("Valori di riferimento:");
    Doc_Item itl = sec->add_item("itemizedlist");
    std::stringstream ss;
    ss << "Ricoprimento Trasversale compreso tra " << minVal << "% e " << maxVal << "%";
    itl->add_item("listitem")->add_item("para")->append(ss.str());
    std::stringstream ss1;
    ss1 << "Massima lunghezza strisciate minore di " << MAX_STRIP_LENGTH << " km";
    itl->add_item("listitem")->add_item("para")->append(ss1.str());

    std::string table = std::string(Z_STRIP) + (_type == PRJ_TYPE ? "P" : "V");
    std::string table2 = std::string(Z_STR_OVL) + (_type == PRJ_TYPE ? "P" : "V");
    std::stringstream sql;
    sql << "SELECT Z_STRIP_CS, Z_STRIP_LENGTH, Z_STRIP_T_OVERLAP, Z_STRIP2 FROM " << table << " a inner JOIN " <<
        table2 << " b on b.Z_STRIP1 = a.Z_STRIP_CS WHERE Z_STRIP_LENGTH>" << MAX_STRIP_LENGTH << " OR Z_STRIP_T_OVERLAP<" << minVal << " OR Z_STRIP_T_OVERLAP>" << maxVal;

    Statement stm(cnn);
    stm.prepare(sql.str());
    Recordset rs = stm.recordset();
    if ( rs.fields_count() == 0 ) {
        sec->add_item("para")->append("In tutte le strisciate i parametri verificati rientrano nei range previsti");
        return;
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

        print_item(row, attrr, rs[1], less_ty, MAX_STRIP_LENGTH);
        print_item(row, attrr, rs[2], between_ty, minVal, maxVal);

        row->add_item("entry", attr)->append(rs[3].toString());
        rs.next();
    }
    return;
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
        _lidar.fov(set[0].toDouble());
        _lidar.ifov(set[1].toDouble());
        _lidar.freq(set[2].toDouble());
        _lidar.scan(set[3].toDouble());
     } catch (CV::Util::Spatialite::spatialite_error& err) {
        (void*)&err;
        return false;
     }

    return true;
}

bool lidar_exec::_read_lidar_from_mission() {
	try {
        CV::Util::Spatialite::Statement stm(cnn);

        std::stringstream query;
		query << "select MISSION.NAME as MISSION, SENSOR.* from MISSION, SENSOR where MISSION.ID_SENSOR == SENSOR.ID";
		stm.prepare(query.str());

        CV::Util::Spatialite::Recordset set = stm.recordset();
		while (!set.eof()) {
			Lidar::Sensor::Ptr sensor(new Lidar::Sensor);
			sensor->fov(set["FOV"].toDouble());
			sensor->ifov(set["IFOV"].toDouble());
			sensor->freq(set["FREQ"].toDouble());
			sensor->scan(set["SCAN_RATE"].toDouble());
			_lidarsList.insert(std::pair<std::string, Lidar::Sensor::Ptr>(set["MISSION"].toString(), sensor));
			set.next();
		}

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

void lidar_exec::_createStripTable() {
	std::string table = std::string(Z_STRIP) + (_type == PRJ_TYPE ? "P" : "V");
	cnn.remove_layer(table);
	std::cout << "Layer:" << table << std::endl;

	// create the model table
	std::stringstream sql;
	sql << "CREATE TABLE " << table << 
		"(Z_STRIP_ID TEXT NOT NULL, " <<	// sigla del lavoro
		"Z_STRIP_CS TEXT NOT NULL, " <<		// strisciata
		"Z_STRIP_YAW FLOAT NOT NULL, " <<		// angolo
		"Z_MISSION TEXT NOT NULL, " <<	
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
}

void lidar_exec::_process_strips()
{
	_createStripTable();

	// create the table for the strip footprint
    std::string table = std::string(Z_STRIP) + (_type == PRJ_TYPE ? "P" : "V");

	std::stringstream sql2;
	sql2 << "INSERT INTO " << table << " (Z_STRIP_ID, Z_STRIP_CS, Z_MISSION, Z_STRIP_YAW, Z_STRIP_LENGTH, geom) \
		VALUES (?1, ?2, ?3, ?4, ?5, ST_GeomFromWKB(:geom, " << SRID << ") )";
	Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());

	// select data from flight lines
	table = std::string(ASSI_VOLO) + (_type == PRJ_TYPE ? "P" : "V");
	std::stringstream sql3;
	sql3 << "SELECT A_VOL_QT, A_VOL_CS, mission, AsBinary(geom) geom from " << table;
	Statement stm1(cnn);
	stm1.prepare(sql3.str());
	Recordset rs = stm1.recordset();

	std::cout << "Layer:" << table << std::endl;

	DSM* ds = _df->GetDsm();


	while ( !rs.eof() ) {
        Blob blob =  rs["geom"].toBlob();
        OGRGeomPtr pol = blob;
		double z = rs[0];
		std::string strip = rs[1];
		std::string mission = rs[2];
		double gProj = _type == FLY_TYPE ? _lidarsList[mission]->halfGroundWidth() : _lidar.halfGroundWidth();

		Lidar::Axis::Ptr axis(new Lidar::Axis(blob, z));
		axis->stripName(strip);
		axis->missionName(mission);

		if (!axis->isValid()) { 
			throw std::runtime_error("asse di volo non valido");
		}

		Lidar::Strip::Ptr stripPtr(new Lidar::Strip);
		stripPtr->fromAxis(axis, ds, gProj);

        double dist = axis->length() / 1000;
		stm[1] = SIGLA_PRJ;
		stm[2] = strip;
		stm[3] = mission;
		stm[4] = RAD_DEG(axis->angle());
        stm[5] = dist;
		stm[6].fromBlob(stripPtr->geom());
		stm.execute();

		_strips.insert(std::pair<std::string, Lidar::Strip::Ptr>(stripPtr->name(), stripPtr));

		stm.reset();
		rs.next();
	}
	cnn.commit_transaction();
}

Poco::Timestamp from_string(const std::string& date, const std::string& time, const std::string& format = "%Y/%m/%d %H:%M:%s")
{
	/*Poco::StringTokenizer tok(time, ":", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	Poco::Timestamp tm;
	if ( tok.count() < 3 )
		return tm;
	double sec = atof(tok[2].c_str());
	double msec = 1000 * ( sec - (int) sec);
	Poco::DateTime dm(2010, 1, 1, atoi(tok[0].c_str()), atoi(tok[1].c_str()), (int) sec, (int) msec);

	Poco::DateTime date;
	int d;
	bool ret = Poco::DateTimeParser::tryParse("%H:%M:%s", time, date, d);

	return dm.timestamp();*/

	std::stringstream stream;
	stream << date << " " << time;  
	int d;
	Poco::DateTime outDate;
	Poco::DateTimeParser::tryParse(format, stream.str(), outDate, d);
	return outDate.timestamp();
}

void lidar_exec::_update_assi_volo()
{
	std::cout << "Associazione della traccia GPS con gli assi di volo" << std::endl;

	std::string table = ASSI_VOLO + std::string("V");

	// add the columns for the gps data
	add_column(cnn, table, "DATE TEXT");
	add_column(cnn, table, "TIME_S TEXT");
	add_column(cnn, table, "TIME_E TEXT");
	add_column(cnn, table, "MISSION TEXT");
	add_column(cnn, table, "NBASI INTEGER");
	add_column(cnn, table, "NSAT INTEGER");
	add_column(cnn, table, "PDOP DOUBLE");
	add_column(cnn, table, "GPS_GAP INTEGER");

	// query to associate to the first and last point of each flight line the nearest point of the gps track
	std::stringstream sql;
	sql << "SELECT a." << STRIP_NAME << " as strip, b.*, AsBinary(b.geom) as geo, min(st_Distance(st_PointN(ST_Transform(a.geom," << SRIDGEO << "), ?1), b.geom)) FROM " <<
        table << " a, " << GPS_TABLE_NAME << " b group by strip";

	Statement stm(cnn);
	stm.prepare(sql.str());

	stm[1] = 1;
	Recordset rs = stm.recordset();

	// for every strip get the GPS time of the first point
	std::vector<GPS::Sample::Ptr> ft1;
	while ( !rs.eof() ) {
		GPS::Sample::Ptr f(new GPS::Sample);
        f->strip(rs["strip"].toString());
        f->mission(rs["MISSION"].toString());
        f->dateTime(rs["DATE"].toString(), rs["TIME"].toString());
		f->gpsData(rs["NSAT"].toInt(), rs["NBASI"].toInt(), rs["PDOP"].toDouble());
        Blob blob = rs["geo"].toBlob();
		f->point(blob);
		ft1.push_back(f);

		_strips["1"/*f->mission()*/]->axis()->addFirstSample(f);
		rs.next();
	}
	stm.reset();
	stm[1] = 2;
	rs = stm.recordset();

	// for every strip get the GPS time of the last point
	std::vector<GPS::Sample::Ptr> ft2;
	while ( !rs.eof() ) {
		GPS::Sample::Ptr f(new GPS::Sample);
        f->strip(rs["strip"].toString());
        f->mission(rs["MISSION"].toString());
        f->dateTime(rs["DATE"].toString(), rs["TIME"].toString());
		f->gpsData(rs["NSAT"].toInt(), rs["NBASI"].toInt(), rs["PDOP"].toDouble());
        Blob blob = rs["geo"].toBlob();
		f->point(blob);
		ft2.push_back(f);

		_strips["1"/*f->mission()*/]->axis()->addLastSample(f);
		rs.next();
	}

	_strips["1"/*f->mission()*/]->axis()->averageSpeed();

	std::stringstream sql1;
	sql1 << "UPDATE " << table << " SET MISSION=?1, DATE=?2, TIME_S=?3, TIME_E=?4, NSAT=?5, PDOP=?6, NBASI=?7, GPS_GAP=?8 where " << STRIP_NAME  << "=?9";
	Statement stm1(cnn);
	stm1.prepare(sql1.str());
	cnn.begin_transaction();

	// per ogni strip determina i parametri gps con cui è stata acquisita
	for ( size_t i = 0; i < ft1.size(); i++) {
		const std::string& val = ft1[i]->strip();
		std::string dateStr;

		std::string t1 = ft1[i]->time();
		for ( size_t j = 0; j < ft2.size(); j++) {
			if ( ft2[j]->strip() == ft1[i]->strip() ) {
				std::string t2 = ft2[j]->time();
				if (t1 > t2) {
					std::swap(t1, t2);
				}

				std::stringstream sql;
				sql << "SELECT MISSION, DATE, TIME, NSAT, PDOP, NBASI from " << GPS_TABLE_NAME << " where TIME >= '" << t1 << "' and TIME <= '" << t2 << "' ORDER BY TIME";
				stm.prepare(sql.str());
				rs = stm.recordset();
				bool first = true;
				
				Poco::Timestamp tm0, tm1;
				double dt0 = 0.;
				int nsat, nbasi;
				double pdop;
				while (!rs.eof()) {
					if (first) {
						dateStr = rs["DATE"].toString();

						stm1[1] = (std::string const &) rs["MISSION"]; // mission
						stm1[2] = dateStr; // date
						stm1[3] = t1; // istante di inizio acquisizione
						stm1[4] = t2; // istante di fine acquisizione

						nsat = rs["NSAT"].toInt();
						pdop = rs["PDOP"].toDouble();
						nbasi = rs["NBASI"].toInt();
						stm1[9] = val;
						first = false;
					} else {
						nsat = std::min(nsat, rs["NSAT"].toInt());
						pdop = std::max(pdop, rs["PDOP"].toDouble());
						nbasi = std::min(nbasi, rs["NBASI"].toInt());
						tm1 = from_string(dateStr, rs["TIME"]);
						double dt = (double) (tm1 - tm0) / 1000000;
						tm0 = tm1;
						if (dt > dt0) {
							dt0 = dt;
						}
					}
					rs.next();
				}
				if (first) {
					return;
				}
				stm1[5] = nsat; // minimal number of satellite
				stm1[6] = pdop; // max pdop
				stm1[7] = nbasi; // number of bases
				stm1[8] = (int) dt0;
				stm1.execute();
				stm1.reset();
			}
		}
	}
	cnn.commit_transaction();
}

void lidar_exec::_process_block()
{
	// select data from flight lines
    /*std::string table = std::string(Z_STRIP) + (_type == PRJ_TYPE ? "P" : "V");
	std::stringstream sql1;
	sql1 << "SELECT AsBinary(geom) geom, Z_STRIP_YAW, Z_STRIP_CS from " << table;
	Statement stm1(cnn);
	stm1.prepare(sql1.str());
	Recordset rs = stm1.recordset();

	std::cout << "Layer:" << table << std::endl;

	OGRGeomPtr blk;
	bool first = true;
	while ( !rs.eof() ) {
        Blob blob = rs["geom"].toBlob();
		Lidar::Strip::Ptr r(new Lidar::Strip(blob));
		r->yaw(rs["Z_STRIP_YAW"].toDouble());
		r->name(rs["Z_STRIP_CS"].toString());
		_strips.insert(std::pair<std::string, Lidar::Strip::Ptr>(r->name(), r));
		if ( first ) {
			blk = r->geom();
			first = false;
		} else {
			blk = blk->Union(r->geom());
		}
		rs.next();
	}*/

	_get_overlaps(_strips);

	Lidar::Block block;
	std::map<std::string, Lidar::Strip::Ptr>::const_iterator it = _strips.begin();
	std::map<std::string, Lidar::Strip::Ptr>::const_iterator end = _strips.end();
	for (; it != end; ++it) {
		block.add((*it).second);
	}

	std::string tableb = std::string("BLOCK") + (_type == PRJ_TYPE ? "P" : "V");
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
		"'" << get_typestring(block.geom()) << "'," <<
		"'XY')";
	cnn.execute_immediate(sqlb.str());
	
	std::stringstream sqlc;
	sqlc << "INSERT INTO " << tableb << " (Z_BLOCK_ID, geom) VALUES (?1, ST_GeomFromWKB(:geom, " << SRID << ") )";
	Statement stm0(cnn);
	cnn.begin_transaction();
	stm0.prepare(sqlc.str());
	stm0[1] = SIGLA_PRJ;
	stm0[2].fromBlob(block.geom());
	stm0.execute();
	stm0.reset();
	cnn.commit_transaction();

	_get_dif();
}

void lidar_exec::_get_overlaps(const std::map<std::string, Lidar::Strip::Ptr>& rec) {
	std::string table = std::string(Z_STR_OVL) + (_type == PRJ_TYPE ? "P" : "V");
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

		std::map<std::string, Lidar::Strip::Ptr>::const_iterator it = rec.begin();
		std::map<std::string, Lidar::Strip::Ptr>::const_iterator end = rec.end();
		
		int k = 0;
		for (; it != end; it++) {
			const Lidar::Strip::Ptr source = (*it).second;
			std::map<std::string, Lidar::Strip::Ptr>::const_iterator next = it;
			for (next++; next != end; next++) {
				const Lidar::Strip::Ptr target = (*next).second;
				if (source->isParallel(target) && source->intersect(target)) {
					int dt = source->intersectionPercentage(target);
					if (dt) {
						stm[1] = ++k;
						stm[2] = source->name();
						stm[3] = target->name();
						stm[4] = dt;
						stm.execute();
						stm.reset();
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
	table = std::string("BLOCK") + (_type == PRJ_TYPE ? "P" : "V");
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
    std::string tabled = std::string("UNCOVERED_AREA") + (_type == PRJ_TYPE ? "P" : "V");
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
