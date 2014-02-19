/*
    File: ta_exec.cpp
    Author:  F.Flamigni
    Date: 2013 November 06
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
#include "check_ta.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/StringTokenizer.h"
#include "Poco/AutoPtr.h"
#include "Poco/Path.h"
#include "Poco/SharedPtr.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "common/util.h"

#define SRID 32632
#define SIGLA_PRJ "CSTP"
#define CAMERA "camera.xml"
#define OUT_DOC "check_ta.xml"
#define REF_FILE "Regione_Toscana_RefVal.xml"
#define FOTOGRAMMETRIA "Fotogrammetria"
#define Z_CAMERA "Camera"

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::SharedPtr;
using Poco::Path;
using namespace CV::Util::Spatialite;
/**************************************************************************/

/***************************************************************************************/
std::string ta_exec::_get_key(const std::string& val)
{
	return std::string(FOTOGRAMMETRIA) + "." + _refscale + "." + val;
}
ta_exec::~ta_exec() 
{
}
bool ta_exec::run()
{
	if ( _proj_dir.empty() )
		throw std::runtime_error("cartella di lavoro non impostata");

	try {
		// initialize spatial lite connection
		Poco::Path db_path(_proj_dir, DB_NAME);
		cnn.open(db_path.toString());

		if ( !GetProjData(cnn, _note, _refscale) )
			throw std::runtime_error("dati progetto incompleti");

		if ( _refscale.empty() )
			throw std::runtime_error("scala di lavoro non impostata");
		if ( _vdp_name.empty() )
			throw std::runtime_error("File degli assetti non impostato nessuna operazione può essere fatta");

		// Read the reference values
		_read_ref_val();

		// initialize docbook xml file
		std::string title = "Collaudo triangolazione aerea";
		Path doc_file(_proj_dir, "*");
		doc_file.setFileName(OUT_DOC);

		init_document(_dbook, doc_file.toString(), title, _note);
		char* dtd_ = getenv("DOCBOOKRT");
		std::string dtd;
		if ( dtd_ != NULL )
			dtd = std::string("file:") + dtd_;
		_dbook.set_dtd(dtd);
		_article = _dbook.get_item("article");
		
		std::cout << "Produzione del report finale: " << _dbook.name() << std::endl;

		// read camera data and assign to strips
		read_cams(cnn, _map_strip_cam);

		// check on control points
		_check_cpt();

		// check the differences on two results
		_check_differences();

		// write the result on the docbook report
		_dbook.write();
		std::cout << "Procedura terminata correttamente" << std::endl;
	}
    catch(std::exception &e) {
        std::cout << std::string(e.what()) << std::endl;
    }

	return true;
}
//void ta_exec::set_ref_scale(const std::string& nome)
//{
//	if ( nome  == "1000" )
//		_refscale = "RefScale_1000";
//	else if ( nome  == "2000" )
//		_refscale = "RefScale_2000";
//	else if ( nome  == "5000" )
//		_refscale = "RefScale_5000";
//	else if ( nome  == "10000" )
//		_refscale = "RefScale_10000";
//}
void ta_exec::_resume()
{
}
void ta_exec::set_vdp_name(const std::string& nome)
{
	// first results file used for comparison and control point check
	_vdp_name = nome;
}
void ta_exec::set_vdp_name2(const std::string& nome)
{
	// second results file used for comparison
	_vdp_name_2 = nome;
}
void ta_exec::set_obs_name(const std::string& nome)
{
	// observation file
	_obs_name = nome;
}
void ta_exec::set_proj_dir(const std::string& nome)
{
	_proj_dir = nome;
}
bool ta_exec::_read_ref_val()
{
	Path ref_file(_proj_dir, "*");
	ref_file.popDirectory();
	ref_file.setFileName(REF_FILE);
	AutoPtr<XMLConfiguration> pConf;
	try {
		pConf = new XMLConfiguration(ref_file.toString());
		// toll of projection center
		_T_CP = pConf->getDouble(_get_key("T_CP"));
		// toll for pitch and roll
		_T_PR = pConf->getDouble(_get_key("T_PR"));
		//toll for headin angle
		_T_H = pConf->getDouble(_get_key("T_H"));
		// toll of planimetric control points
		_TP_PA = pConf->getDouble(_get_key("TP_PA"));
		// toll of altimetric control points
		_TA_PA = pConf->getDouble(_get_key("TA_PA"));

		_T_CP *= 3 * sqrt(2.);
		_T_PR *= 3 * sqrt(2.);
		_T_H *=  3 * sqrt(2.);
	} catch (...) {
		throw std::runtime_error("Errore nela lettura dei valori di riferimento");
	}
	return true;
}
//void ta_exec::_init_document()
//{
//	Path doc_file(_proj_dir, "*");
//	doc_file.setFileName(OUT_DOC);
//	_dbook.set_name(doc_file.toString());
//	
//	_article = _dbook.add_item("article");
//	_article->add_item("title")->append("Collaudo triangolazione aerea");
//}
Doc_Item ta_exec::_initpg1()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica punti appoggio");

	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
	ss << "Tolleranza planimetrica " << _TP_PA << " m";
	itl->add_item("listitem")->add_item("para")->append(ss.str());
	std::stringstream ss1;
	ss1 << "Tolleranza altimetrica " << _TA_PA << " m";
	itl->add_item("listitem")->add_item("para")->append(ss1.str());
	
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
Doc_Item ta_exec::_initpg2()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Confronto tra i risultati di due calcoli");
	
	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
	ss << "Massima differenza tra ciascuna coordinata dei centri di presa " << _T_CP << " m";
	itl->add_item("listitem")->add_item("para")->append(ss.str());
	std::stringstream ss1;
	ss1 << "Massima differenza tra i valori di pitch e roll " << _T_PR << " mdeg";
	itl->add_item("listitem")->add_item("para")->append(ss1.str());
	std::stringstream ss2;
	ss2 << "Massima differenza tra i valori ddellheading " << _T_PR << " mdeg";
	itl->add_item("listitem")->add_item("para")->append(ss2.str());

	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("scarti tra i valori risultanti dai due calcoli");
	
	Poco::XML::AttributesImpl attrs;
	attrs.addAttribute("", "", "cols", "", "7");
	tab = tab->add_item("tgroup", attrs);
	
	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");
	
	attrs.clear();
	attrs.addAttribute("", "", "align", "", "center");
	row->add_item("entry", attrs)->append("Foto");
	row->add_item("entry", attrs)->append("sc pc-X");
	row->add_item("entry", attrs)->append("sc pc-Y");
	row->add_item("entry", attrs)->append("sc pc-Z");
	row->add_item("entry", attrs)->append("sc omega");
	row->add_item("entry", attrs)->append("sc fi");
	row->add_item("entry", attrs)->append("Sc ka");

	Doc_Item tbody = tab->add_item("tbody");
	return tbody;
}

/// checks for differences between two triangulations
bool ta_exec::_add_point_to_table(Doc_Item tbody, const std::string& foto, const VecOri& pt, const VecOri& sc)
{
	Doc_Item row = tbody->add_item("row");
	Poco::XML::AttributesImpl attrr, attrc;
	attrr.addAttribute("", "", "align", "", "right");
	attrc.addAttribute("", "", "align", "", "center");

	row->add_item("entry", attrc)->append(foto);

	bool b = true;
	b &= print_item(row, attrr, pt[0], abs_less_ty, _T_CP);
	b &= print_item(row, attrr, pt[1], abs_less_ty, _T_CP);
	b &= print_item(row, attrr, pt[2], abs_less_ty, _T_CP);
	b &= print_item(row, attrr, sc[0], abs_less_ty, _T_PR);
	b &= print_item(row, attrr, sc[1], abs_less_ty, _T_PR);
	b &= print_item(row, attrr, sc[2], abs_less_ty, _T_H);
	return b; // false if at least one element is out of tolerance
}

bool ta_exec::_add_point_to_table(Doc_Item tbody, const std::string& cod, const std::string& nome1, const std::string& nome2, const DPOINT& sc)
{
	Doc_Item row = tbody->add_item("row");
	Poco::XML::AttributesImpl attrr, attrc;
	attrr.addAttribute("", "", "align", "", "right");
	attrc.addAttribute("", "", "align", "", "center");

	row->add_item("entry", attrc)->append(cod);
	row->add_item("entry", attrc)->append(nome1);
	row->add_item("entry", attrc)->append(nome2);

	bool b = true;
	if ( sc.x == 0 )
		row->add_item("entry", attrc)->append("-");
	else
		b &= print_item(row, attrr, sc.x, abs_less_ty, _TP_PA);
	if ( sc.y == 0 )
		row->add_item("entry", attrc)->append("-");
	else
		b &= print_item(row, attrr, sc.y, abs_less_ty, _TP_PA);
	if ( sc.z == 0 )
		row->add_item("entry", attrc)->append("-");
	else
		b &= print_item(row, attrr, sc.z, abs_less_ty, _TA_PA);
	return b;
}
//bool ta_exec::_read_cam(std::map<std::string, Camera>& map_strip_cam)
//{
//	std::stringstream sql;
//	sql << "SELECT * from " << Z_CAMERA;
//	Statement stm(cnn);
//	stm.prepare(sql.str());
//	Recordset rs = stm.recordset();
//	
//	std::map<std::string, Camera> cams;
//	Camera cam_plan;
//
//	// mappa delle camere usate compresa quella di progetto
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
//		cams[cam.id] = cam;
//		if ( cam.planning )
//			cams["progetto"] = cam;
//	}
//	// get the camera associated to each mission
//	std::string table = "MISSION";
//	std::stringstream sql1;
//	sql1 << "SELECT ID_CAMERA, NAME from " << table;
//	stm.prepare(sql1.str());
//	rs = stm.recordset();
//
//	std::map<std::string, std::string> map_mission_cam;
//
//	while ( !rs.eof() ) {
//		map_mission_cam[ rs["NAME"] ] = rs["ID_CAMERA"]; // mission name camera id
//		rs.next();
//	}
//
//	// get the mission associated to each strip
//	table = std::string(ASSI_VOLO) + "V";
//	std::stringstream sql2;
//	sql2 << "SELECT A_VOL_CS, MISSION from " << table;
//	stm = Statement(cnn);
//	stm.prepare(sql2.str());
//	rs = stm.recordset();
//	while ( !rs.eof() ) {
//		std::string strip = rs["A_VOL_CS"]; // strip name - mission name
//		std::string mission = rs["MISSION"];
//		std::string cam_id = map_mission_cam[mission]; // camera id for mission
//		if ( _cams.find(cam_id) != cams.end() )
//			map_strip_cam[strip] = cams[cam_id];
//		else
//			map_strip_cam[strip] = cams["progetto"];
//		rs.next();
//	}
//	return true;
//}
//bool ta_exec::_strip_cam()
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
void ta_exec::_read_vdp(const std::string& nome, VDP_MAP& vdps)
{
	std::ifstream fvdp(nome.c_str(), std::ifstream::in);
	if ( !fvdp.is_open() ) {
		std::string mes("Impossibile aprire il file: ");
		mes += nome;
		throw std::runtime_error(mes);
	}

	vdps.clear();

	char buf[256];
	while ( fvdp.getline(buf, 256) ) {
		Poco::StringTokenizer tok(buf, " \t", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
		if ( tok.count() != 7 )
			continue;
		if ( atof(tok[1].c_str()) == 0. )
			continue;
		Camera cam;
		vdps[tok[0]] = VDPC(cam, tok[0]);
		VDPC& vdp = vdps[tok[0]];
		vdp.Init(DPOINT(atof(tok[1].c_str()), atof(tok[2].c_str()), atof(tok[3].c_str())), atof(tok[4].c_str()), atof(tok[5].c_str()), atof(tok[6].c_str()));
	}

	std::map<std::string, VDPC>::iterator it;
	for ( it = vdps.begin(); it != vdps.end(); it++) {
		std::string strip = get_strip(it->first);
		Camera cam;
		if ( _map_strip_cam.find(strip) != _map_strip_cam.end() )
			cam = _map_strip_cam[strip];
		else
			throw std::runtime_error("Strip senza fotocamera");
		it->second.InitIor(cam);
	}
}
bool ta_exec::_read_image_pat(VDP_MAP& vdps, const CPT_MAP& pm, CPT_VDP& pts) 
{
	Poco::Path gf(_obs_name);
	std::string nome = gf.getBaseName();
	std::string dir = gf.parent().toString();
	std::string ext = gf.getExtension();
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	int n = tok.count();
	if ( n > 1 ) {
		int k = n - 1;
		if ( tok[k] != "image" )
			throw std::runtime_error("nome del file delle osservazioni non valido");
	} else
		throw std::runtime_error("nome del file delle osservazioni non valido");

	pts.clear();
	
	std::ifstream fvdp(gf.toString().c_str(), std::ifstream::in);
	if ( !fvdp.is_open() ) {
		std::string mes("Impossibile aprire il file: ");
		mes += gf.toString();
		throw std::runtime_error(mes);
	}

	char buf[256];

	VDPC* vdp = NULL;
	double divisor = 1;
	while ( fvdp.getline(buf, 256) ) {
		Poco::StringTokenizer tok(buf, " \t", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
		if ( tok.count() == 2 ) {
			// the name of the image and the focal
			std::string nome = tok[0];
			if ( vdps.find(nome) == vdps.end() )
				vdp = NULL;
			vdp = &vdps[nome];
			// evaluate if the coords are in micron or mm
			double foc = atof(tok[1].c_str());
			divisor = foc > 1000 ? 1000 : 1;
		} else if ( vdp && tok.count() == 3 ) {
			// the name and the plate coord. of the point
			std::string cod = tok[0];
			// only control points are considered
			if ( pm.find(cod) != pm.end() ) {
				// convert to mm
				Collimation cl(atof(tok[1].c_str()) / divisor, atof(tok[2].c_str()) / divisor);
				// tranform to image coord
				Collimation ci = vdp->Las_Img(cl);
				vdp->operator [](cod) = ci;
				// associate each point with the images where it is observed
				pts.insert(std::pair<std::string, std::string>(cod, vdp->nome));
			}
		}
	}
	return true;
}
bool ta_exec::_calc_pts(VDP_MAP& vdps, const CPT_MAP& pm, const CPT_VDP& pts)
{
	Doc_Item row = _initpg1();
	CPT_MAP::const_iterator it;
	for ( it = pm.begin(); it != pm.end(); it++) {
		std::pair<CPT_VDP::const_iterator, CPT_VDP::const_iterator> ret;
		std::string cod = it->first;
		DPOINT pc = it->second;
		ret = pts.equal_range(cod);
		CPT_VDP::const_iterator it1 = ret.first;
		std::string nome1 = it1->second;
		it1++;
		for (; it1 != ret.second; it1++) {
			std::string nome2 = it1->second;
			if ( get_strip(nome1) == get_strip(nome2) ) {
				// only for images of the same strip
				VDPC& vdp1 = vdps[nome1];
				VDPC& vdp2 = vdps[nome2];
				Collimation c1 = vdp1[cod];
				Collimation c2 = vdp2[cod];
				DPOINT pt;
				vdp1.Img_TerA(vdp2, c1, c2, pt);
				DPOINT sc;
				if ( pc.x == 0 && pc.y == 0 )
					sc = DPOINT(0, 0, pc.z - pt.z);
				else if ( pc.z == 0 )
					sc = DPOINT(pc.x - pt.x, pc.y - pt.y, 0);
				else
					sc = pc - pt;//DPOINT(pc.x - pt.x, pc.y - pt.y, pc.z - pt.z);
				if ( !_add_point_to_table(row, cod, nome1, nome2, sc) )
					_cpt_out_tol.push_back(cod);
			}
			nome1 = nome2;
		}
	}
	Doc_Item sec = _article->get_item("section");
	if ( sec.get() == NULL )
		return false;
	if ( _cpt_out_tol.empty() ) {
		std::stringstream ss;
		ss << "Tutti i punti di controllo rientrano nelle tolleranze";
		sec->add_item("para")->append(ss.str());
	} else {
		std::stringstream ss;
		ss << "I seguenti punti risultano fuori dalle tolleranze";
		sec->add_item("para")->append(ss.str());
		Doc_Item itl = sec->add_item("itemizedlist");
		std::list<std::string>::iterator it;
		for (it = _cpt_out_tol.begin(); it != _cpt_out_tol.end(); it++) {
			itl->add_item("listitem")->add_item("para")->append(*it);
		}
	}
	return true;
}
bool ta_exec::_read_cont_pat(CPT_MAP& pm) 
{
	Poco::Path gf(_obs_name);
	std::string nome = gf.getBaseName();
	std::string dir = gf.parent().toString();
	std::string ext = gf.getExtension();

	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	int n = tok.count();
	if ( n > 1 ) {
		int k = n - 1;
		if ( tok[k] != "image" )
			throw std::runtime_error("nome del file dei punti noti non valido");
	} else
		throw std::runtime_error("nome del file dei punti noti non valido");

	nome.clear();
	for (int i = 0; i < n - 1; i++) {
		if ( i )
			nome.append("_");
		nome.append(tok[i]);
	}
	nome.append("_cont");
	gf = Poco::Path(dir, nome);
	gf.setExtension(ext);

	std::ifstream fvdp(gf.toString().c_str(), std::ifstream::in);
	if ( !fvdp.is_open() ) {
		std::string mes("Impossibile aprire il file: ");
		mes += gf.toString();
		throw std::runtime_error(mes);	}

	pm.clear();

	char buf[256];
	while ( fvdp.getline(buf, 256) ) {
		Poco::StringTokenizer tok(buf, " \t", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
		if ( tok.count() == 5 ) {
			// planimetric point
			std::string cod = tok[0]; // point name
			if ( pm.find(cod) != pm.end() ) {
				DPOINT& p = pm[cod];
				p.x = atof(tok[1].c_str());
				p.y = atof(tok[2].c_str());
			} else {
				DPOINT p(atof(tok[1].c_str()), atof(tok[2].c_str()));
				pm[cod] = p;
			}
		} else if (tok.count() == 4 ) {
			// altimetric point
			std::string cod = tok[0];
			if ( pm.find(cod) != pm.end() ) {
				DPOINT& p = pm[cod];
				p.z = atof(tok[1].c_str());
			} else {
				DPOINT p(0., 0., atof(tok[1].c_str()));
				pm[cod] = p;
			}
		}
	}
	return true;
}
bool ta_exec::_check_cpt()
{
	if ( _obs_name.empty() )
		return true;
	std::cout << "Controllo sui punti noti" << std::endl;

	try {
		VDP_MAP vdps; // mappa nome fotogramma, parametri assetto
		CPT_MAP pm; // mappa nome punto, coordinate
		CPT_VDP pts; // mappa nome punto fotogrammi in cui è stato osservato

		//_cam_name = Path(_proj_dir, CAMERA).toString();

		_read_vdp(_vdp_name, vdps);
		_read_cont_pat(pm);
		_read_image_pat(vdps, pm, pts);
		_calc_pts(vdps, pm, pts);
	} catch(std::exception &e) {
		throw std::runtime_error(std::string("Controllo sui punti noti ") + std::string(e.what()));
    }
	return true;
}
bool ta_exec::_check_differences()
{
	if ( _vdp_name_2.empty() )
		return true;
		
	std::cout << "Confronto tra i risultati di due calcoli" << std::endl;

	try {
		Doc_Item row = _initpg2();

		//read_cams(cnn, _map_strip_cam);

		VDP_MAP vdps1; // mappa nome fotogramma, parametri assetto
		VDP_MAP vdps2; // mappa nome fotogramma, parametri assetto
		_read_vdp(_vdp_name, vdps1);
		_read_vdp(_vdp_name_2, vdps2);
		VDP_MAP::iterator it;
		for (it = vdps1.begin(); it != vdps1.end(); it++) {
			std::string nome = it->first;
			if ( vdps2.find(nome) != vdps2.end() ) {
				VDP& vdp1 = vdps1[nome];
				VDP& vdp2 = vdps2[nome];
				VecOri pc = vdp1.Pc - vdp2.Pc;
				VecOri at(1000 * RAD_DEG(vdp1.om - vdp2.om), 1000 * RAD_DEG(vdp1.fi - vdp2.fi), 1000 * RAD_DEG(vdp1.ka - vdp2.ka));

				if ( !_add_point_to_table(row, nome, pc, at) )
					_tria_out_tol.push_back(nome);
			}
		}
		Doc_Item sec = _article->get_item("section");
		if ( sec.get() == NULL )
			return false;
		if ( _tria_out_tol.empty() ) {
			std::stringstream ss;
			ss << "I due risultati sono compatibili";
			sec->add_item("para")->append(ss.str());
		} else {
			std::stringstream ss;
			ss << "I due risultati sono diversi nei seguenti fotogrammi";
			sec->add_item("para")->append(ss.str());
			Doc_Item itl = sec->add_item("itemizedlist");
			std::list<std::string>::iterator it;
			for (it = _tria_out_tol.begin(); it != _tria_out_tol.end(); it++) {
				itl->add_item("listitem")->add_item("para")->append(*it);
			}
		}
	} catch(std::exception &e) {
		std::string mes("Confronto tra due calcoli ");
		mes.append(e.what());
		throw std::runtime_error(mes);
	}
	return true;
}
