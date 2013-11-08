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
#include "Poco/stringtokenizer.h"
#include "Poco/AutoPtr.h"
#include "Poco/Path.h"
#include "Poco/sharedPtr.h"
#include <fstream>
#include <sstream>

#define SRID 32632
#define SIGLA_PRJ "CSTP"

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::SharedPtr;
using Poco::Path;

ta_exec::~ta_exec() 
{
}
bool ta_exec::run()
{
	// inizializza la connessione con spatial lite
	Poco::Path db_path(_proj_dir, "geo.sqlite");

	// legge i valori di riferimento per la verifica
	_read_ref_val();

	_init_document();

	if ( !_check_differences() )
		return false;
	if ( !_check_cpt() )
		return false;
	_dbook.write();

	return true;
}
void ta_exec::set_vdp_name(const std::string& nome)
{
	_vdp_name = nome;
}
void ta_exec::set_vdp_name2(const std::string& nome)
{
	_vdp_name_2 = nome;
}
void ta_exec::set_cam_name(const std::string& nome)
{
	_cam_name = nome;
}
void ta_exec::set_out_folder(const std::string& nome)
{
	_out_folder = nome;
}
void ta_exec::set_proj_dir(const std::string& nome)
{
	_proj_dir = nome;
}
bool ta_exec::_read_ref_val()
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
void ta_exec::_init_document()
{
	Path doc_file(_proj_dir, "*");
	doc_file.setFileName("check_ta.xml");
	_dbook.set_name(doc_file.toString());
	
	_article = _dbook.add_item("article");
	_article->add_item("title")->append("Collaudo triangolazione aerea");
}
Doc_Item ta_exec::_initpg1()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica punti appoggio");
	
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("scarti tra valori nominali e valori misurati");
	
	Poco::XML::AttributesImpl attrs;
	attrs.addAttribute("", "", "cols", "", "6");
	
	attrs.clear();
	attrs.addAttribute("", "", "align", "", "center");
	tab = tab->add_item("tgroup", attrs);
	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");
	row->add_item("entry", attrs)->append("Codice");
	row->add_item("entry", attrs)->append("Foto Sx");
	row->add_item("entry", attrs)->append("Foto Dx");
	row->add_item("entry", attrs)->append("sc X");
	row->add_item("entry", attrs)->append("sc Y");
	row->add_item("entry", attrs)->append("Sc Z");

	Doc_Item tbody = tab->add_item("tbody");
	row = tbody->add_item("row");
	return tbody;
}
Doc_Item ta_exec::_initpg2()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Confronto tra i risultati di due calcoli");
	
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("scarti tra i valori risultanti dai due calcoli");
	
	Poco::XML::AttributesImpl attrs;
	attrs.addAttribute("", "", "cols", "", "6");
	
	attrs.clear();
	attrs.addAttribute("", "", "align", "", "center");
	tab = tab->add_item("tgroup", attrs);
	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");
	row->add_item("entry", attrs)->append("sc pc-X");
	row->add_item("entry", attrs)->append("sc pc-Y");
	row->add_item("entry", attrs)->append("sc pc-Z");
	row->add_item("entry", attrs)->append("sc omega");
	row->add_item("entry", attrs)->append("sc fi");
	row->add_item("entry", attrs)->append("Sc ka");

	Doc_Item tbody = tab->add_item("tbody");
	row = tbody->add_item("row");
	return tbody;
}
void ta_exec::_add_point_to_table(Doc_Item tbody, const VecOri& pt, const VecOri& sc)
{
	Doc_Item row = tbody->add_item("row");
	Poco::XML::AttributesImpl attrr, attrc;
	attrr.addAttribute("", "", "align", "", "right");
	attrc.addAttribute("", "", "align", "", "center");

	row->add_item("entry", attrr)->append(pt[0]);
	row->add_item("entry", attrr)->append(pt[1]);
	row->add_item("entry", attrr)->append(pt[2]);

	row->add_item("entry", attrr)->append(sc[0]);
	row->add_item("entry", attrr)->append(sc[1]);
	row->add_item("entry", attrr)->append(sc[2]);
}

void ta_exec::_add_point_to_table(Doc_Item tbody, const std::string& cod, const std::string& nome1, const std::string& nome2, const DPOINT& sc)
{
	Doc_Item row = tbody->add_item("row");
	Poco::XML::AttributesImpl attrr, attrc;
	attrr.addAttribute("", "", "align", "", "right");
	attrc.addAttribute("", "", "align", "", "center");

	row->add_item("entry", attrc)->append(cod);
	row->add_item("entry", attrc)->append(nome1);
	row->add_item("entry", attrc)->append(nome2);
	if ( sc.x == 0. )
		row->add_item("entry", attrc)->append("-");
	else
		row->add_item("entry", attrr)->append(sc.x);
	if ( sc.y == 0. )
		row->add_item("entry", attrc)->append("-");
	else
		row->add_item("entry", attrr)->append(sc.y);
	if ( sc.z == 0. )
		row->add_item("entry", attrc)->append("-");
	else
		row->add_item("entry", attrr)->append(sc.z);
}
bool ta_exec::_read_cam()
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
bool ta_exec::_read_vdp(const std::string& nome, VDP_MAP& vdps)
{
	std::ifstream fvdp(nome.c_str(), std::ifstream::in);
	if ( !fvdp.is_open() )
		return false;

	vdps.clear();

	char buf[256];
	while ( fvdp.getline(buf, 256) ) {
		Poco::StringTokenizer tok(buf, " \t", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
		if ( tok.count() != 7 )
			continue;
		if ( atof(tok[1].c_str()) == 0. )
			continue;
		vdps[tok[0]] = VDPC(_cam, tok[0]);
		VDPC& vdp = vdps[tok[0]];
		vdp.Init(DPOINT(atof(tok[1].c_str()), atof(tok[2].c_str()), atof(tok[3].c_str())), atof(tok[4].c_str()), atof(tok[5].c_str()), atof(tok[6].c_str()));
	}
	return true;

}
bool ta_exec::_read_image_pat(VDP_MAP& vdps, const CPT_MAP& pm, CPT_VDP& pts) 
{
	Poco::Path gf(_vdp_name);
	std::string nome = gf.getBaseName();
	nome.append("_image");
	gf.setBaseName(nome);
	gf.setExtension("pat");

	pts.clear();
	
	std::ifstream fvdp(gf.toString().c_str(), std::ifstream::in);
	if ( !fvdp.is_open() )
		return false;

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
std::string ta_exec::_get_strip(const std::string& nome)
{
	// extract strip name from image name
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() != 2 )
		return "";
	return tok[0];
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
			if ( _get_strip(nome1) == _get_strip(nome2) ) {
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
					sc = DPOINT(pc.x - pt.x, pc.y - pt.y, pc.z - pt.z);
				_add_point_to_table(row, cod, nome1, nome2, sc);
				//std::cout << cod << " " << nome1 << "-" << nome2 << " dx=" << sc.x << " dy=" << sc.y << " dz=" << sc.z << std::endl;
			}
			nome1 = nome2;
		}
	}
	return true;
}
bool ta_exec::_read_cont_pat(CPT_MAP& pm) 
{
	// build the control point file name
	Poco::Path gf(_vdp_name);
	std::string nome = gf.getBaseName();
	nome.append("_cont");
	gf.setBaseName(nome);
	gf.setExtension("pat");
	
	std::ifstream fvdp(gf.toString().c_str(), std::ifstream::in);
	if ( !fvdp.is_open() )
		return false;

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
	VDP_MAP vdps; // mappa nome fotogramma, parametri assetto
	CPT_MAP pm; // mappa nome punto, coordinate
	CPT_VDP pts; // mappa nome punto fotogrammi in cui è stato osservato

	_read_cam();
	_read_vdp(_vdp_name, vdps);
	_read_cont_pat(pm);
	_read_image_pat(vdps, pm, pts);
	_calc_pts(vdps, pm, pts);

	return true;
}
bool ta_exec::_check_differences()
{
	Doc_Item row = _initpg2();

	_read_cam();
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
			VecOri at(RAD_DEG(vdp1.om - vdp2.om), RAD_DEG(vdp1.fi - vdp2.fi), RAD_DEG(vdp1.ka - vdp2.ka));

			_add_point_to_table(row, pc, at);
			//std::cout << "dX=" << pc[0] << " dY=" << pc[1] << " dZ=" << pc[2] << " dom=" << at[0] << " dfi=" << at[1] << " dka=" << at[2] << std::endl;
		}
	}
	return true;
}
