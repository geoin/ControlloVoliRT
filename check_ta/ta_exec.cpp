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

	//if ( !_read_cam() )
	//	return false;
	//if ( !_read_vdp() )
	//	return false;

	if ( !_check_differences() )
		return false;
	if ( !_check_cpt() )
		return false;

	return true;
}
void ta_exec::set_vdp_name(const std::string& nome)
{
	_vdp_name = nome;
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
bool ta_exec::_read_vdp()
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
		//VDPC vdp(_cam, tok[0]);
		_vdps[tok[0]] = VDPC(_cam, tok[0]);
		VDPC& vdp = _vdps[tok[0]];
		vdp.Init(DPOINT(atof(tok[1].c_str()), atof(tok[2].c_str()), atof(tok[3].c_str())), atof(tok[4].c_str()), atof(tok[5].c_str()), atof(tok[6].c_str()));
		//_vdps[tok[0]] = vdp;
	}
	return true;

}
bool ta_exec::_read_image_pat() 
{
	Poco::Path gf(_vdp_name);
	std::string nome = gf.getBaseName();
	nome.append("_image");
	gf.setBaseName(nome);
	gf.setExtension("pat");
	
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
			if ( _vdps.find(nome) == _vdps.end() )
				vdp = NULL;
			vdp = &_vdps[nome];
			// evaluate if the coords are in micron or mm
			double foc = atof(tok[1].c_str());
			divisor = foc > 1000 ? 1000 : 1;
		} else if ( vdp && tok.count() == 3 ) {
			// the name and the plate coord. of the point
			std::string cod = tok[0];
			// only control points are considered
			if ( _pm.find(cod) != _pm.end() ) {
				// convert to mm
				double x = atof(tok[1].c_str()) / divisor;
				double y = atof(tok[2].c_str()) / divisor;
				// tranform to image coord
				float xi, yi;
				vdp->Las_Img(x, y, &xi, &yi);
				vdp->operator [](cod) = Collimation(xi, yi);
				// associate each point with the images where it is observed
				_pts.insert(std::pair<std::string, std::string>(cod, vdp->nome));
			}
		}
	}
	return true;
}
bool ta_exec::_calc_pts()
{
	std::map<std::string, DPOINT>::iterator it;
	for ( it = _pm.begin(); it != _pm.end(); it++) {
		std::pair<std::multimap<std::string, std::string>::iterator, std::multimap<std::string, std::string>::iterator> ret;
		std::string cod = it->first;
		DPOINT pc = it->second;
		ret = _pts.equal_range(cod);
		std::multimap<std::string, std::string>::iterator it1 = ret.first;
		std::string nome1 = it1->second;
		it1++;
		for (; it1 != ret.second; it1++) {
			std::string nome2 = it1->second;
			VDPC& vdp1 = _vdps[nome1];
			VDPC& vdp2 = _vdps[nome2];
			Collimation c1 = vdp1[cod];
			Collimation c2 = vdp2[cod];
			double X, Y, Z;
			vdp1.Img_TerA(vdp2, c1.xi, c1.yi, c2.xi, c2.yi, &X, &Y, &Z);
			DPOINT sc;
			if ( pc.x == 0 && pc.y == 0 )
				sc = DPOINT(0, 0, pc.z - Z);
			else if ( pc.z == 0 )
				sc = DPOINT(pc.x - X, pc.y - Y, 0);
			else
				sc = DPOINT(pc.x - X, pc.y - Y, pc.z - Z);
			std::cout << cod << " " << nome1 << "-" << nome2 << " dx=" << sc.x << " dy=" << sc.y << " dz=" << sc.z << std::endl;
		}
	}
	return true;
}
bool ta_exec::_read_cont_pat() 
{
	Poco::Path gf(_vdp_name);
	std::string nome = gf.getBaseName();
	nome.append("_cont");
	gf.setBaseName(nome);
	gf.setExtension("pat");
	
	std::ifstream fvdp(gf.toString().c_str(), std::ifstream::in);
	if ( !fvdp.is_open() )
		return false;

	char buf[256];
	while ( fvdp.getline(buf, 256) ) {
		Poco::StringTokenizer tok(buf, " \t", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
		if ( tok.count() == 5 ) {
			// planimetric point
			std::string cod = tok[0]; // point name
			if ( _pm.find(cod) != _pm.end() ) {
				DPOINT& p = _pm[cod];
				p.x = atof(tok[1].c_str());
				p.y = atof(tok[2].c_str());
			} else {
				DPOINT p(atof(tok[1].c_str()), atof(tok[2].c_str()));
				_pm[cod] = p;
			}
		} else if (tok.count() == 4 ) {
			// altimetric point
			std::string cod = tok[0];
			if ( _pm.find(cod) != _pm.end() ) {
				DPOINT& p = _pm[cod];
				p.z = atof(tok[1].c_str());
			} else {
				DPOINT p(0., 0., atof(tok[1].c_str()));
				_pm[cod] = p;
			}
		}
	}
	return true;
}
bool ta_exec::_check_cpt()
{
	_read_cam();
	_read_vdp();
	_read_cont_pat();
	_read_image_pat();
	_calc_pts();
	return true;
}
bool ta_exec::_check_differences()
{
	return true;
}
