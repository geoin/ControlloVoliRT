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

#define SRID 32632
#define SIGLA_PRJ "CSTP"
#define REFSCALE "RefScale_2000"
#define GEO_DB_NAME "geo.sqlite"

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::SharedPtr;
using Poco::Path;
/**************************************************************************/
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

		Path geodb(_prj_folder, GEO_DB_NAME);
		CV::Util::Spatialite::Connection cnn;
		cnn.create( geodb.toString() ); // Create or open spatialite db
		if ( cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA )
			cnn.initialize_metdata(); // Initialize metadata (if already initialized noop)

		Path avolo(_prj_folder, "assi volo");
		avolo = Path(avolo.toString(), "lvolo");
		int nrows = cnn.load_shapefile(avolo.toString(),
			"lvolo",
                   "CP1252",
                   32632,
                   "geom",
                   true,
                   false,
                   false);


	// dagli assi di volo e dai parameti del lidar ricava l'impronta al suolo delle strip
	_read_lidar();
	_get_strips();

	// se volo lidar confronta gli assi progettati con quelli effettivi

	// verifica che le strip ricoprano l'area da cartografare

	// determina il ricoprimento tra strisciate e la loro lunghezza, li confronta con i valori di riferimento

	// se disponibile la velocità calcola la densità media dei pt

	// per il volo verifica i punti dell'area di test


	// write the result on the docbook report
	_dbook.write();

	return true;
}

void lidar_exec::set_proj_dir(const std::string& nome)
{
	_prj_folder = nome;
}
void lidar_exec::set_checkType(Check_Type t)
{
	_type = t;
}
bool lidar_exec::_read_ref_val()
{
	Path ref_file(_prj_folder, "*");
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
	Path doc_file(_prj_folder, "*");
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
	std::string lidar_name = Path(_prj_folder, "lidar.xml").toString();
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

void lidar_exec::_get_strips()
{

}

