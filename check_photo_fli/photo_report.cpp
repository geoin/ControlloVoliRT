/*
    File: photo_report.h
    Author:  F.Flamigni
    Date: 2014 February 04
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
#include "common/util.h"
#include "check_photo.h"
#include "Poco/DateTime.h"

#define OUT_DOCV "check_photoV.xml"
#define OUT_DOCP "check_photoP.xml"

using namespace CV::Util::Spatialite;
using namespace CV::Util::Geometry;
using Poco::Path;

void photo_exec::_init_document()
{
	Path doc_file(_proj_dir, "*");
	doc_file.setFileName(_type == fli_type ? OUT_DOCV : OUT_DOCP);
	_dbook.set_name(doc_file.toString());	

	_article = _dbook.add_item("article");
	_article->add_item("title")->append(_type == fli_type ? "Collaudo ripresa aerofotogrammetrica" : "Collaudo progetto di ripresa aerofotogrammetrica");

	Doc_Item sec = _article->add_item("section");
	Poco::DateTime dt;
	dt.makeLocal(+2);
	std::stringstream ss;
	ss << "Data: " << dt.day() << "/" << dt.month() << "/" << dt.year() << "  " << dt.hour() << ":" << dt.minute();
	sec->add_item("para")->append(ss.str());
}

void photo_exec::_final_report()
{
	// test to be done only for the real flight
	if ( _type == fli_type ) {
		_prj_report();
		_gps_report();
	}
	
	// common tests

	// coverage of cartographic areas
	std::stringstream sql;
	std::string table = std::string(Z_UNCOVER) + (_type == Prj_type ? "P" : "V");
	sql << "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='" << table << "'";
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();

	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica Copertura aree da cartografare");

	int cv = rs[0];
	if ( cv == 0 ) {
		sec->add_item("para")->append("Tutte le aree da cartografare sono state ricoperte da modelli stereoscopici");
	} else {
		sec->add_item("para")->append("Esistono delle aree da cartografare non completamente ricoperte da modelli stereoscopici");
	}
	_foto_report();
	_model_report();
	_strip_report();
}

void photo_exec::_gps_report()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica traccia GPS");

	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
	ss << "Minimo numero di satelli con angolo sull'orizzonte superiore a " << _MIN_SAT_ANG << " deg non inferiore a " << _MIN_SAT;
	itl->add_item("listitem")->append(ss.str());
	std::stringstream ss1;
	ss1 << "Massimo PDOP non superiore a " << _MAX_PDOP;
	itl->add_item("listitem")->append(ss1.str());
	std::stringstream ss2;
	ss2 << "Minimo numero di stazioni permanenti entro " << _MAX_DIST / 1000 << " km non inferiore a " << _NBASI;
	itl->add_item("listitem")->append(ss2.str());
	std::stringstream ss3;
	ss3 << "Minimo angolo del sole rispetto all'orizzonte al momento del rilievo " << _MIN_ANG_SOL << " deg";
	itl->add_item("listitem")->append(ss3.str());

	// check finale
	std::string assi = ASSI_VOLO + std::string("V");
	std::stringstream sql;
	sql << "SELECT " << STRIP_NAME << ", MISSION, DATE, NSAT, PDOP, NBASI, SUN_HL from " << assi <<  " where NSAT<" << _MIN_SAT <<
		" OR PDOP >" << _MAX_PDOP << " OR NBASI <" << _NBASI << " OR SUN_HL <" << _MIN_ANG_SOL << " order by " << STRIP_NAME;

	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	if ( rs.fields_count() == 0 ) {
		sec->add_item("para")->append("Durante l'acquisizione delle strisciate i parametri del GPS rientrano nei range previsti");
		return;
	}

	sec->add_item("para")->append("Le seguenti strisciate presentano dei parametri che non rientrano nei range previsti");
	
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("Strisciate acquisite con parametri GPS fuori range");

	Poco::XML::AttributesImpl attr;
	attr.addAttribute("", "", "cols", "", "5");
	tab = tab->add_item("tgroup", attr);

	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");

	attr.clear();
	attr.addAttribute("", "", "align", "", "center");
	row->add_item("entry", attr)->append("Strip");
	row->add_item("entry", attr)->append("N. sat.");
	row->add_item("entry", attr)->append("PDOP");
	row->add_item("entry", attr)->append("N. staz.");
	row->add_item("entry", attr)->append("Ang. sole");

	Doc_Item tbody = tab->add_item("tbody");

	Poco::XML::AttributesImpl attrr;
	attrr.addAttribute("", "", "align", "", "right");

	while ( !rs.eof() ) {
		row = tbody->add_item("row");

		row->add_item("entry", attr)->append(rs[STRIP_NAME].toString());
		
		print_item(row, attrr, rs["NSAT"], great_ty, _MIN_SAT);
		print_item(row, attrr, rs["PDOP"], less_ty, _MAX_PDOP);
		print_item(row, attrr, rs["NBASI"], great_ty, _NBASI);
		print_item(row, attrr, rs["SUN_HL"], great_ty, _MIN_ANG_SOL);
		rs.next();
	}
	return;
}

bool photo_exec::_strip_report()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica parametri strisciate");

	double v1 = _STRIP_OVERLAP * (1 - _STRIP_OVERLAP_RANGE / 100);
	double v2 = _STRIP_OVERLAP * (1 + _STRIP_OVERLAP_RANGE / 100);

	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
	ss << "Ricoprimento Trasversale compreso tra " << v1 << "% e " << v2 << "%";
	itl->add_item("listitem")->append(ss.str());
	std::stringstream ss1;
	ss1 << "Massima lunghezza strisciate minore di " << _MAX_STRIP_LENGTH << " km";
	itl->add_item("listitem")->append(ss1.str());

	// Strip verification
	std::string table = std::string(Z_STRIP) + (_type == Prj_type ? "P" : "V");
	std::string table2 = std::string(Z_STR_OVL) + (_type == Prj_type ? "P" : "V");
	std::stringstream sql;
	sql << "SELECT Z_STRIP_CS, Z_STRIP_LENGTH, Z_STRIP_T_OVERLAP, Z_STRIP2 FROM " << table << " a inner JOIN " << 
		table2 << " b on b.Z_STRIP1 = a.Z_STRIP_CS WHERE Z_STRIP_LENGTH>" << _MAX_STRIP_LENGTH << " OR Z_STRIP_T_OVERLAP<" << v1 << " OR Z_STRIP_T_OVERLAP>" << v2;

	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	if ( rs.fields_count() == 0 ) {
		sec->add_item("para")->append("In tutte le strisciate i parametri verificati rientrano nei range previsti");
		return true;
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
		
		print_item(row, attrr, rs[1], less_ty, _MAX_STRIP_LENGTH);
		print_item(row, attrr, rs[2], between_ty, v1, v2);

		row->add_item("entry", attr)->append(rs[3].toString());
		rs.next();
	}
	return false;
}
bool photo_exec::_foto_report()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica Parametri immagini");

	double v1 = _GSD * (1 - _MAX_GSD / 100);
	double v2 = _GSD * (1 + _MAX_GSD / 100);

	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
	ss << "GSD compreso tra " << v1 << " m e " << v2 << " m";
	itl->add_item("listitem")->append(ss.str());
	std::stringstream ss1;
	ss1 << "angoli di pitch e roll minori di " << _MAX_ANG << " deg";
	itl->add_item("listitem")->append(ss1.str());

	// verifica della dimensione del pixel e dei valori di picth e roll
	std::stringstream sql;
	std::string table = std::string(Z_FOTO) + (_type == Prj_type ? "P" : "V");
	sql << "SELECT Z_FOTO_NF, Z_FOTO_DIMPIX, Z_FOTO_PITCH, Z_FOTO_ROLL FROM " << table << " WHERE Z_FOTO_DIMPIX not between " << v1 << " and " << v2 <<
		" OR Z_FOTO_PITCH>" << _MAX_ANG << " OR Z_FOTO_ROLL>" << _MAX_ANG;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	if ( rs.fields_count() == 0 ) {
		sec->add_item("para")->append("In tutte le immagini i parametri verificati rientrano nei range previsti");
		return true;
	}
	sec->add_item("para")->append("Nelle seguenti immagini i parametri verificati non rientrano nei range previsti");
	
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("Immagini con parametri fuori range");

	Poco::XML::AttributesImpl attr;
	attr.addAttribute("", "", "cols", "", "4");
	tab = tab->add_item("tgroup", attr);

	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");

	attr.clear();
	attr.addAttribute("", "", "align", "", "center");
	row->add_item("entry", attr)->append("Foto");
	row->add_item("entry", attr)->append("GSD");
	row->add_item("entry", attr)->append("Pitch");
	row->add_item("entry", attr)->append("Roll");

	Doc_Item tbody = tab->add_item("tbody");

	Poco::XML::AttributesImpl attrr;
	attrr.addAttribute("", "", "align", "", "right");
	while ( !rs.eof() ) {
		row = tbody->add_item("row");

		row->add_item("entry", attr)->append(rs[0].toString());
		
		print_item(row, attrr, rs[1], between_ty, v1, v2);
		print_item(row, attrr, rs[2], abs_less_ty, _MAX_ANG);
		print_item(row, attrr, rs[3], abs_less_ty, _MAX_ANG);
		rs.next();
	}
	return false;
}
bool photo_exec::_model_report()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica parametri stereo modelli");

	double v1 = _MODEL_OVERLAP * (1 - _MODEL_OVERLAP_RANGE / 100);
	double v2 = _MODEL_OVERLAP * (1 + _MODEL_OVERLAP_RANGE / 100);

	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
	ss << "Ricoprimento longitudinale compreso tra " << v1 << "% e " << v2 << "%";
	itl->add_item("listitem")->append(ss.str());
	std::stringstream ss1;
	ss1 << "Ricoprimento trasversale maggiore di " << _MODEL_OVERLAP_T << "%";
	itl->add_item("listitem")->append(ss1.str());
	std::stringstream ss2;
	ss2 << "Differenza di heading tra i fotogrammi minori di " << _MAX_HEADING_DIFF << " deg";
	itl->add_item("listitem")->append(ss2.str());

	// Check the photos
	std::string table = std::string(Z_MODEL) + (_type == Prj_type ? "P" : "V");
	std::stringstream sql;
	sql << "SELECT Z_MODEL_LEFT, Z_MODEL_RIGHT, Z_MODEL_L_OVERLAP, Z_MODEL_T_OVERLAP, Z_MODEL_D_HEADING FROM " << table << " WHERE Z_MODEL_L_OVERLAP not between " << v1 << " and " << v2 <<
		" OR Z_MODEL_T_OVERLAP <" << _MODEL_OVERLAP_T << " OR Z_MODEL_D_HEADING >" << _MAX_HEADING_DIFF;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	if ( rs.fields_count() == 0 ) {
		sec->add_item("para")->append("In tutti i modelli i parametri verificati rientrano nei range previsti");
		return true;
	}
	sec->add_item("para")->append("Nei seguenti modelli i parametri verificati non rientrano nei range previsti");
	
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("modelli con parametri fuori range");

	Poco::XML::AttributesImpl attr;
	attr.addAttribute("", "", "cols", "", "5");
	tab = tab->add_item("tgroup", attr);

	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");

	attr.clear();
	attr.addAttribute("", "", "align", "", "center");
	row->add_item("entry", attr)->append("Foto Sx");
	row->add_item("entry", attr)->append("Foto Dx");
	row->add_item("entry", attr)->append("Ric. long.");
	row->add_item("entry", attr)->append("Ric. trasv.");
	row->add_item("entry", attr)->append("Dif. head.");

	Doc_Item tbody = tab->add_item("tbody");

	Poco::XML::AttributesImpl attrr;
	attrr.addAttribute("", "", "align", "", "right");
	while ( !rs.eof() ) {
		row = tbody->add_item("row");

		row->add_item("entry", attr)->append(rs[0].toString());
		row->add_item("entry", attr)->append(rs[1].toString());
		
		print_item(row, attrr, rs[2], between_ty, v1, v2);
		print_item(row, attrr, rs[3], great_ty, _MODEL_OVERLAP_T);
		print_item(row, attrr, rs[4], abs_less_ty, _MAX_HEADING_DIFF);
		rs.next();
	}
	return false;
}