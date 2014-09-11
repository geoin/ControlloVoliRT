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

#include "cv_version.h"

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
		CV::Version::print();

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

		_findReferenceColumns();

		// dagli assi di volo e dai parameti del lidar ricava l'impronta al suolo delle strip
		
		if ( _type == FLY_TYPE ) {
			// legge i dati del sensore in base alla missione
			_read_lidar_from_mission();
		} else {
			// legge i dati del sensore di progetto
			_read_lidar();
		}
		// read digital terrain model
		//_dem_name = Path(_proj_dir, DEM).toString();
		if ( !_read_dem() )
			throw std::runtime_error("Modello numerico non trovato");

		_process_strips();
		_process_block();
		
		if ( _type == FLY_TYPE ) {
			_check_sample_cloud();
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

// TODO (CHECK): removing mission for now
void lidar_exec::_get_planned_axis(std::vector<CV::Lidar::Axis::Ptr>& _projectAxis) {
	std::stringstream sql;
    sql << "SELECT ROWID, " << _stripNameCol << ", "/*mission, "*/ << _quotaCol << ", AsBinary(GEOM) as GEOM FROM AVOLOP";
    Statement stm(cnn);
    stm.prepare(sql.str());
    Recordset rs = stm.recordset();

	while (!rs.eof()) {
		Blob b = rs["GEOM"].toBlob();
		OGRGeomPtr g = b;
		
		CV::Lidar::Axis::Ptr axis(new CV::Lidar::Axis(g, rs[_quotaCol ].toDouble()));
		axis->stripName(rs[_stripNameCol].toString());
		axis->missionName("");//rs["mission"].toString());
		axis->id(rs[0].toInt());

		_projectAxis.push_back(axis);

		rs.next();
	}
}

void lidar_exec::_compare_axis() {
	std::vector<CV::Lidar::Axis::Ptr> _projectAxis;
	_get_planned_axis(_projectAxis);

	if (_projectAxis.size() != _strips.size()) {
		std::cout << "Different axis number" << std::endl;
	}

	//<real axis, proj axis>
	std::map<CV::Lidar::Axis::Ptr, CV::Lidar::Axis::Ptr> matches;
	std::vector<unsigned short> matchesId;

	int tollerance = 10;
	std::map<std::string, CV::Lidar::Strip::Ptr>::const_iterator it = _strips.begin();
	std::map<std::string, CV::Lidar::Strip::Ptr>::const_iterator end = _strips.end();
	for (; it != end; it++) {
		CV::Lidar::Axis::Ptr stripAxis = (*it).second->axis();
		std::vector<CV::Lidar::Axis::Ptr>::const_iterator tIt = _projectAxis.begin();
		std::vector<CV::Lidar::Axis::Ptr>::const_iterator tEnd = _projectAxis.end();

		struct Target {
			unsigned short id;
			double distance;
		} target = {-1, 1e10};

		int i = 0;
		for (; tIt != tEnd; ++tIt, ++i) {
			CV::Lidar::Axis::Ptr tAxis = (*tIt);
			double minFirst = std::min(stripAxis->first().dist2D(tAxis->first()), stripAxis->first().dist2D(tAxis->last()));
			double minLast = std::min(stripAxis->last().dist2D(tAxis->last()), stripAxis->last().dist2D(tAxis->first())); 
			double med = (minFirst + minLast) / 2;
			if (med < target.distance) {
				target.id = i;
				target.distance = med;

				matchesId.push_back(i);
			}
		}

		if (target.distance < tollerance && target.id != -1) {
			matches.insert(std::pair<CV::Lidar::Axis::Ptr, CV::Lidar::Axis::Ptr>(stripAxis, _projectAxis.at(target.id)));
		} else {
			matches.insert(std::pair<CV::Lidar::Axis::Ptr, CV::Lidar::Axis::Ptr>(stripAxis, CV::Lidar::Axis::Ptr(NULL)));
		}
	}

	for (unsigned short i = 0; i < _projectAxis.size(); ++i) {
		if (std::find(matchesId.begin(), matchesId.end(), i) == matchesId.end()) {
			matches.insert(std::pair<CV::Lidar::Axis::Ptr, CV::Lidar::Axis::Ptr>(CV::Lidar::Axis::Ptr(NULL), _projectAxis.at(i)));
		}
	}

	_compare_axis_report(matches);
}

void lidar_exec::_compare_axis_report(std::map<CV::Lidar::Axis::Ptr, CV::Lidar::Axis::Ptr>& matches) {
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Confronto del volo col progetto di volo");
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("Accoppiamento tra strisciate progettate e volate");

	Poco::XML::AttributesImpl attr;
	attr.addAttribute("", "", "cols", "", "64");
	tab = tab->add_item("tgroup", attr);

	attr.clear();
	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");
	attr.addAttribute("", "", "align", "", "center");
	row->add_item("entry", attr)->append("Strisciata pianificata");
	row->add_item("entry", attr)->append("lung.");
	row->add_item("entry", attr)->append("Strisciata volata");
	row->add_item("entry", attr)->append("lung.");

	Doc_Item tbody = tab->add_item("tbody");

		
	std::map<CV::Lidar::Axis::Ptr, CV::Lidar::Axis::Ptr>::iterator b = matches.begin();
	for (; b != matches.end(); b++) {
		row = tbody->add_item("row");

		CV::Lidar::Axis::Ptr real = b->first;
		CV::Lidar::Axis::Ptr proj = b->second;

		if (!proj.isNull()) {
			row->add_item("entry", attr)->append(proj->stripName());
			std::stringstream s1; s1 << proj->length();
			row->add_item("entry", attr)->append(s1.str());
		} else {
			row->add_item("entry", attr)->append(" - ");
			row->add_item("entry", attr)->append(" - ");
		}

		if (!real.isNull()) {
			row->add_item("entry", attr)->append(real->stripName());
			std::stringstream s2; s2 << real->length();
			row->add_item("entry", attr)->append(s2.str());
		} else {
			row->add_item("entry", attr)->append(" - ");
			row->add_item("entry", attr)->append(" - ");
		}
	}
}

void lidar_exec::_final_report() {
    if ( _type == FLY_TYPE ) {
		_compare_axis();
		_control_points_report();
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
        sec->add_item("para")->append("Tutte le aree da rilevare sono state ricoperte da modelli.");
    } else {
        sec->add_item("para")->append("Esistono delle aree da rilevare non completamente ricoperte da modelli.");
    }
    stm.reset();

    // verifica ricoprimento tra strisciate
    // verifica lunghezza strisciate
    _strips_comp_report();
	_strip_report();
}

void lidar_exec::_control_points_report() {
	Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Punti di controllo");

    sec->add_item("para")->append("Validità punti di controllo");

    Doc_Item tab = sec->add_item("table");
    tab->add_item("title")->append("Punti di controllo");

    Poco::XML::AttributesImpl attr;
    attr.addAttribute("", "", "cols", "", "2");
    tab = tab->add_item("tgroup", attr);

    Doc_Item thead = tab->add_item("thead");
    Doc_Item row = thead->add_item("row");

    attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Punto di controllo");
    row->add_item("entry", attr)->append("Z diff");
    Doc_Item tbody = tab->add_item("tbody");

    Poco::XML::AttributesImpl attrr;
    attrr.addAttribute("", "", "align", "", "right");

	std::vector<CV::Lidar::ControlPoint::Ptr>::iterator it = _controlVal.begin();
	std::vector<CV::Lidar::ControlPoint::Ptr>::iterator end = _controlVal.end();

	for (; it != end; it++) {
		CV::Lidar::ControlPoint::Ptr point = *it;
		const std::string& name = point->name();

		row = tbody->add_item("row");
        row->add_item("entry", attr)->append(name);
		
		if (point->isValid()) {
			double diff = point->zDiff();
			print_item(row, attrr, diff, abs_less_ty, LID_TOL_Z);
		} else {
			Doc_Item r = row->add_item("entry", attr);
			r->add_instr("dbfo", "bgcolor=\"red\"");

			Lidar::ControlPoint::Status status = point->status();
			if (status == Lidar::ControlPoint::NO_VAL) {
				r->append("NO_VAL");
			} else if (status == Lidar::ControlPoint::OUT_VAL) {
				r->append("OUT_VAL");
			}
		}
    }
    return;
}

void lidar_exec::_strips_comp_report() {
    Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Verifica ricoprimento strisciate");

    double minVal = STRIP_OVERLAP * (1 - STRIP_OVERLAP_RANGE / 100.0);
    double maxVal = STRIP_OVERLAP * (1 + STRIP_OVERLAP_RANGE / 100.0);

    sec->add_item("para")->append("Valori di riferimento:");
    Doc_Item itl = sec->add_item("itemizedlist");
    std::stringstream ss;
    ss << "Ricoprimento Trasversale compreso tra " << minVal << "% e " << maxVal << "%";
    itl->add_item("listitem")->add_item("para")->append(ss.str());

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
    attr.addAttribute("", "", "cols", "", "3");
    tab = tab->add_item("tgroup", attr);

    Doc_Item thead = tab->add_item("thead");
    Doc_Item row = thead->add_item("row");

    attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Strip.");
    //row->add_item("entry", attr)->append("Lung.");
    row->add_item("entry", attr)->append("Ric. trasv.");
    row->add_item("entry", attr)->append("Strip adiac.");

    Doc_Item tbody = tab->add_item("tbody");

    Poco::XML::AttributesImpl attrr;
    attrr.addAttribute("", "", "align", "", "right");
    while ( !rs.eof() ) {
        row = tbody->add_item("row");

        row->add_item("entry", attr)->append(rs[0].toString());

        //print_item(row, attrr, rs[1], less_ty, MAX_STRIP_LENGTH);
        print_item(row, attrr, rs[2], between_ty, minVal, maxVal);

        row->add_item("entry", attr)->append(rs[3].toString());
        rs.next();
    }
    return;
}

void lidar_exec::_strip_report() {
    Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Verifica parametri strisciate");

	sec->add_item("para")->append("Valori di riferimento:");
    
	Doc_Item itl = sec->add_item("itemizedlist");
    std::stringstream ss;
    ss << "Massima lunghezza strisciate minore di " << MAX_STRIP_LENGTH << " km";
    itl->add_item("listitem")->add_item("para")->append(ss.str());

	/*ss.str("");
	ss << "Angolo di scansione minore di " << LID_ANG_SCAN << " deg";
    itl->add_item("listitem")->add_item("para")->append(ss.str());*/

	ss.str("");
	ss << "Densita' dei punti al suolo maggiore di " << PT_DENSITY << " pt/mq";
    itl->add_item("listitem")->add_item("para")->append(ss.str());

	std::map<std::string, CV::Lidar::Strip::Ptr>::const_iterator it = _strips.begin();
	std::map<std::string, CV::Lidar::Strip::Ptr>::const_iterator end = _strips.end();

	bool hasEntry = false;
	Doc_Item tbody;
	for (; it != end; it++) {
		CV::Lidar::Strip::Ptr strip = it->second;

		double d = strip->density();
		double len = strip->axis()->length() / 1000.0f;
		const std::string& name = it->first;

		if (len > MAX_STRIP_LENGTH || d < PT_DENSITY) {
			if (hasEntry == false) {
				hasEntry = true;
				
				sec->add_item("para")->append("Nelle seguenti strisciate i parametri verificati non rientrano nei range previsti");
				Doc_Item tab = sec->add_item("table");
				tab->add_item("title")->append("Strisciate con parametri fuori range");

				Poco::XML::AttributesImpl attr;
				attr.addAttribute("", "", "cols", "", "3");
				tab = tab->add_item("tgroup", attr);

				Doc_Item thead = tab->add_item("thead");
				Doc_Item row = thead->add_item("row");

				attr.clear();
				attr.addAttribute("", "", "align", "", "center");
				row->add_item("entry", attr)->append("Strip.");
				row->add_item("entry", attr)->append("Lung.");
				row->add_item("entry", attr)->append("Dens.");

				tbody = tab->add_item("tbody");
			}

			Doc_Item row = tbody->add_item("row");
			
			Poco::XML::AttributesImpl attr;
			attr.addAttribute("", "", "align", "", "center");
			row->add_item("entry", attr)->append(name);

			attr = Poco::XML::AttributesImpl();
			attr.addAttribute("", "", "align", "", "right");
			print_item(row, attr, len, less_ty, MAX_STRIP_LENGTH);
			print_item(row, attr, d, great_ty, d);
		}
	}

	if (!hasEntry) {
        sec->add_item("para")->append("In tutte le strisciate i parametri verificati rientrano nei range previsti");
	}
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
		LID_TOL_Z = pConf->getDouble(get_key("LID_TOL_Z"));
		PT_DENSITY = pConf->getDouble(get_key("PT_DENSITY"));
		LID_ANG_SCAN =  pConf->getDouble(get_key("LID_ANG_SCAN"));
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
	_lidar.assign(new Lidar::Sensor);
    try {
        CV::Util::Spatialite::Statement stm(cnn);

        std::stringstream query;
        query << "select FOV, IFOV, FREQ, SCAN_RATE, SPEED from SENSOR where PLANNING = ?1";
        stm.prepare(query.str());
		stm[1] = 1;
        CV::Util::Spatialite::Recordset set = stm.recordset();
        if (set.eof()) {
            return false;
        }
        _lidar->fov(set[0].toDouble());
        _lidar->ifov(set[1].toDouble());
        _lidar->freq(set[2].toDouble()*1000); //KHz -> Hz
        _lidar->scan(set[3].toDouble());
		_lidar->speed(set[4].toDouble());
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

bool lidar_exec::_check_sample_cloud() {
	if (!_read_cloud()) {
		return false;
	}

	try {
		DSM* dsm = _sampleCloudFactory->GetDsm();

		CV::Util::Spatialite::Recordset set = _read_control_points();
		while (!set.eof()) {
			Blob b = set["GEOM"].toBlob();
			Lidar::ControlPoint::Ptr point(new Lidar::ControlPoint(b, set["Z_QUOTA"].toDouble()));
			point->name(set["NAME"].toString());

			point->zDiffFrom(dsm); //can be Z_NOVAL Z_OUT
			_controlVal.push_back(point);

			set.next();
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		(void*)&err;
		return false;
	}
	return true;
}

CV::Util::Spatialite::Recordset lidar_exec::_read_control_points() {
	CV::Util::Spatialite::Statement stm(cnn);

    std::stringstream query;
	query << "select Z_QUOTA, NAME, AsBinary(GEOM) as GEOM FROM CONTROL_CLOUD";
	stm.prepare(query.str());

    CV::Util::Spatialite::Recordset set = stm.recordset();
	return set;
}

bool lidar_exec::_read_cloud() {
	CV::Util::Spatialite::Statement stm(cnn);

    std::stringstream query;
    query << "select URI from CLOUD_SAMPLE";
    stm.prepare(query.str());
    CV::Util::Spatialite::Recordset set = stm.recordset();
    if (set.eof()) {
        return false;
    }
	std::string samplePath = Path(_proj_dir, set[0].toString()).toString();

	_sampleCloudFactory.assign(new DSM_Factory);
	_sampleCloudFactory->SetEcho(MyLas::last_pulse);
	if (!_sampleCloudFactory->Open(samplePath, false)) {
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
	// create the table for the strip footprint
	_createStripTable();
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
	sql3 << "SELECT ROWID, " << _quotaCol << ", " << _stripNameCol << /*", mission*/", AsBinary(geom) geom from " << table;
	Statement stm1(cnn);
	stm1.prepare(sql3.str());
	Recordset rs = stm1.recordset();

	std::cout << "Layer:" << table << std::endl;

	DSM* ds = _df->GetDsm();

	while ( !rs.eof() ) {
        Blob blob =  rs["geom"].toBlob();
        OGRGeomPtr pol = blob;
		double z = rs[1];
		std::string strip = rs[2];
		std::string mission = "";//rs[2];

		Lidar::Axis::Ptr axis(new Lidar::Axis(blob, z));
		axis->stripName(strip);
		axis->missionName(mission);
		axis->id(rs[0].toInt());

		if (!axis->isValid()) { 
			throw std::runtime_error("Asse di volo non valido");
		}

		Lidar::Sensor::Ptr lidar;
		if (_type == FLY_TYPE) {
			lidar = _lidarsList[mission];
		} else {
			lidar = _lidar; 
		}
		if (lidar.isNull()) {
			throw std::runtime_error("Valori lidar non validi");
		} 

		Lidar::Strip::Ptr stripPtr(new Lidar::Strip);
		stripPtr->fromAxis(axis, ds, lidar->tanHalfFov());
		stripPtr->computeDensity(lidar, ds);

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

		_strips[f->strip()]->axis()->addFirstSample(f);
		rs.next();
	}
	stm.reset();
	stm[1] = 2;
	rs = stm.recordset();

	// for every strip get the GPS time of the last point
	std::vector<GPS::Sample::Ptr> ft2;
	while (!rs.eof()) {
		GPS::Sample::Ptr f(new GPS::Sample);
        f->strip(rs["strip"].toString());
        f->mission(rs["MISSION"].toString());
        f->dateTime(rs["DATE"].toString(), rs["TIME"].toString());
		f->gpsData(rs["NSAT"].toInt(), rs["NBASI"].toInt(), rs["PDOP"].toDouble());
        Blob blob = rs["geo"].toBlob();
		f->point(blob);
		ft2.push_back(f);

		_strips[f->strip()]->axis()->addLastSample(f);
		rs.next();
	}

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

void lidar_exec::_findReferenceColumns() {
	std::stringstream sql;
	sql << "SELECT TARGET FROM _META_COLUMNS_ WHERE CONTROL = 3 AND OBJECT = 2 and REF = ?1";
	Statement stm(cnn);

	stm.prepare(sql.str());
	stm[1] = "A_VOL_QT";
	Recordset rs = stm.recordset();
	if (rs.eof()) {
		throw std::runtime_error("Errore durante il reperimento di A_VOL_QT");
	}

	_quotaCol = rs[0].toString();
	if (_quotaCol.size() == 0) {
		throw std::runtime_error("Errore, selezionare la colonna dello shape contente i valori di quota");
	}

	stm.reset();
	
	stm.prepare(sql.str());
	stm[1] = "A_VOL_CS";
	rs = stm.recordset();
	if (rs.eof()) {
		throw std::runtime_error("Errore durante il reperimento di A_VOL_CS");
	}

	_stripNameCol = rs[0].toString();
	if (_stripNameCol.size() == 0) {
		throw std::runtime_error("Errore, selezionare la colonna dello shape contente il nome delle strisciate");
	}
}
