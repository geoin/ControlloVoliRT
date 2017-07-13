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
#include <iostream>

using namespace CV::Util::Spatialite;
using namespace CV::Util::Geometry;
using Poco::Path;

//void photo_exec::_init_document(const std::string& nome, const std::string& title)
//{
//	//Path doc_file(_proj_dir, "*");
//	//doc_file.setFileName(_type == fli_type ? OUT_DOCV : OUT_DOCP);
//	//_dbook.set_name(doc_file.toString());	
//	_dbook.set_name(nome);	
//
//	Poco::XML::AttributesImpl attr;
//	attr.addAttribute("", "", "lang", "", "it");
//	_article = _dbook.add_item("article", attr);
//	_article->add_item("title")->append(title);
//	//_article->add_item("title")->append(_type == fli_type ? "Collaudo ripresa aerofotogrammetrica" : "Collaudo progetto di ripresa aerofotogrammetrica");
//
//	Doc_Item sec = _article->add_item("section");
//	sec->add_item("title")->append("Intestazione");
//
//	std::stringstream sql;
//	sql << "SELECT NOTE from JOURNAL where CONTROL=1";
//	Statement stm(cnn);
//	stm.prepare(sql.str());
//	Recordset rs = stm.recordset();
//	std::string head = rs[0];
//	std::string head1;
//	for ( int i = 0; i < head.size(); i++) {
//		if ( head[i] == 10 ) {
//			sec->add_item("para")->append(head1);
//			head1.clear();
//		} else
//			head1.push_back(head[i]);
//	}
//
//	Poco::DateTime dt;
//	dt.makeLocal(+2);
//	std::stringstream ss;
//	ss << "Data: " << dt.day() << "/" << dt.month() << "/" << dt.year() << "  " << dt.hour() << ":" << dt.minute();
//	sec->add_item("para")->append(ss.str());
//}

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
	itl->add_item("listitem")->add_item("para")->append(ss.str());
	std::stringstream ss1;
	ss1 << "Massimo PDOP non superiore a " << _MAX_PDOP;
	itl->add_item("listitem")->add_item("para")->append(ss1.str());
	std::stringstream ss2;
	ss2 << "Minimo numero di stazioni permanenti entro " << _MAX_DIST / 1000 << " km non inferiore a " << _NBASI;
	itl->add_item("listitem")->add_item("para")->append(ss2.str());
	std::stringstream ss3;
	ss3 << "Minimo angolo del sole rispetto all'orizzonte al momento del rilievo " << _MIN_ANG_SOL << " deg";
	itl->add_item("listitem")->add_item("para")->append(ss3.str());

	// check finale
	std::string assi = ASSI_VOLO + std::string("V");
	std::stringstream sql;
	sql << "SELECT " << STRIP_NAME << ", MISSION, DATE, NSAT, PDOP, NBASI, SUN_HL, GPS_GAP from " << assi <<  " where NSAT<" << _MIN_SAT <<
		" OR PDOP >" << _MAX_PDOP << " OR NBASI <" << _NBASI << " OR SUN_HL <" << _MIN_ANG_SOL << " OR GPS_GAP >" <<  _MAX_GPS_GAP << " order by " << STRIP_NAME;

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
	attr.addAttribute("", "", "cols", "", "6");
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
	row->add_item("entry", attr)->append("Gap GPS");

	Doc_Item tbody = tab->add_item("tbody");

	Poco::XML::AttributesImpl attrr;
	attrr.addAttribute("", "", "align", "", "right");

	while ( !rs.eof() ) {
		row = tbody->add_item("row");

		row->add_item("entry", attr)->append(rs[STRIP_NAME].toString());
		
		print_item(row, attrr, rs["NSAT"], great_ty, _MIN_SAT-1);
		print_item(row, attrr, rs["PDOP"], less_ty, _MAX_PDOP);
		print_item(row, attrr, rs["NBASI"], great_ty, _NBASI-1);
		print_item(row, attrr, rs["SUN_HL"], great_ty, _MIN_ANG_SOL);
		print_item(row, attrr, rs["GPS_GAP"], less_ty, _MAX_GPS_GAP);
		rs.next();
	}
	return;
}

bool photo_exec::_strip_report()
{
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Verifica parametri strisciate");

    double minVal = _STRIP_OVERLAP * (1 - _STRIP_OVERLAP_RANGE / 100);
    double maxVal = _STRIP_OVERLAP * (1 + _STRIP_OVERLAP_RANGE / 100);

	sec->add_item("para")->append("Valori di riferimento:");
	Doc_Item itl = sec->add_item("itemizedlist");
	std::stringstream ss;
    ss << "Ricoprimento Trasversale maggiore di " << minVal << "%";
	itl->add_item("listitem")->add_item("para")->append(ss.str());
	std::stringstream ss1;
	ss1 << "Massima lunghezza strisciate minore di " << _MAX_STRIP_LENGTH << " km";
	itl->add_item("listitem")->add_item("para")->append(ss1.str());

	// Strip verification
	std::string table = std::string(Z_STRIP) + (_type == Prj_type ? "P" : "V");
	std::string table2 = std::string(Z_STR_OVL) + (_type == Prj_type ? "P" : "V");
	std::stringstream sql;
	sql << "SELECT Z_STRIP_CS, Z_STRIP_LENGTH, Z_STRIP_T_OVERLAP, Z_STRIP2 FROM " << table << " a inner JOIN " << 
        table2 << " b on b.Z_STRIP1 = a.Z_STRIP_CS WHERE Z_STRIP_LENGTH>" << _MAX_STRIP_LENGTH << " OR Z_STRIP_T_OVERLAP<" << minVal;

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
        print_item(row, attrr, rs[2], great_ty, minVal);

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
	itl->add_item("listitem")->add_item("para")->append(ss.str());
	std::stringstream ss1;
	ss1 << "angoli di pitch e roll minori di " << _MAX_ANG << " deg";
	itl->add_item("listitem")->add_item("para")->append(ss1.str());

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
	itl->add_item("listitem")->add_item("para")->append(ss.str());
	std::stringstream ss1;
	ss1 << "Ricoprimento trasversale maggiore di " << _MODEL_OVERLAP_T << "%";
	itl->add_item("listitem")->add_item("para")->append(ss1.str());
	std::stringstream ss2;
	ss2 << "Differenza di heading tra i fotogrammi minori di " << _MAX_HEADING_DIFF << " deg";
	itl->add_item("listitem")->add_item("para")->append(ss2.str());

	// Check the photos
	std::string table = std::string(Z_MODEL) + (_type == Prj_type ? "P" : "V");
	std::stringstream sql;
	sql << "SELECT Z_MODEL_LEFT, Z_MODEL_RIGHT, Z_MODEL_L_OVERLAP, Z_MODEL_T_OVERLAP, Z_MODEL_D_HEADING FROM " << table << " WHERE Z_MODEL_L_OVERLAP not between " << v1 << " and " << v2 <<
		" OR Z_MODEL_T_OVERLAP <" << _MODEL_OVERLAP_T << " OR Z_MODEL_D_HEADING >" << _MAX_HEADING_DIFF;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();

	bool ret = false;
	if ( rs.fields_count() == 0 ) {
		sec->add_item("para")->append("In tutti i modelli i parametri verificati rientrano nei range previsti");
		ret = true;
	} else {
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
	}
	if ( !_useless_models.empty() ) {
		sec->add_item("para")->append("I seguenti modelli non ricoprono le aree da cartografare:");
		Doc_Item itl = sec->add_item("itemizedlist");
		std::stringstream ss;
		for ( size_t i = 0; i < _useless_models.size(); i++) {
			itl->add_item("listitem")->add_item("para")->append(_useless_models[i]);
		}
	}
	return ret;
}
class strip_desc {
public:
	void clear() {
		id.clear();
		n_fot = 0;
		len = 0;
	}
	std::string id;
	int n_fot;
	DPOINT p1, p2;
	double len;
};
bool photo_exec::_prj_report()
{
	std::cout << "Confronto del volo col progetto di volo" << std::endl;
	Doc_Item sec = _article->add_item("section");
	sec->add_item("title")->append("Confronto del volo col progetto di volo");

	std::vector<strip_desc> real, plan;

	strip_desc stp;
	bool start = true;

	std::map<std::string, VDP>::iterator it1;
	for ( it1 = _vdps.begin(); it1 != _vdps.end(); it1++) {
		VDP& vdp = it1->second;
		std::string nn = vdp.nome;
		std::string strip = get_strip(nn);
		if ( strip == stp.id ) {
			stp.n_fot++;
			stp.p2 = vdp.Pc;
			stp.len = stp.p1.dist2D(stp.p2);
		} else {
			if ( !start ) {
				//stp.p2 = vdp.Pc;
				//stp.len = stp.p1.dist2D(stp.p2);
				real.push_back(stp);
				
			}
			start = false;
			stp.clear();
			stp.id = strip;
			stp.n_fot = 1;
			stp.p1 = vdp.Pc;
		}
	}
	real.push_back(stp);

	stp.clear();
	start = true;
	for ( it1 = _vdps_plan.begin(); it1 != _vdps_plan.end(); it1++) {
		VDP& vdp = it1->second;
		std::string nn = vdp.nome;
		std::string strip = get_strip(nn);
		if ( strip == stp.id ) {
			stp.n_fot++;
			stp.p2 = vdp.Pc;
			stp.len = stp.p1.dist2D(stp.p2);
		} else {
			if ( !start ) {
				//stp.p2 = vdp.Pc;
				//stp.len = stp.p1.dist2D(stp.p2);
				plan.push_back(stp);
				
			}
			start = false;
			stp.clear();
			stp.id = strip;
			stp.n_fot = 1;
			stp.p1 = vdp.Pc;
		}
	}
	plan.push_back(stp);

	if ( real.size() != plan.size() ) {
		std ::stringstream ss;
		ss << " Numero di strisciate pianificate: " << plan.size() 
			<< " Numero di strisciate volate: " << real.size();
		sec->add_item("para")->append(ss.str());
		std::cout << ss.str() << std::endl;
	} else {
		std ::stringstream ss;
		ss << "Il numero di strisciate pianificate coincide con quelle volate" << std::endl;
		sec->add_item("para")->append(ss.str());
		std::cout << ss.str() << std::endl;
	}
	
	std::map<int, int> mp;
	for ( size_t i = 0; i < plan.size(); i++) {
		double dmin = 1e10;
		mp[i] = -1;
		for ( size_t j = 0; j < real.size(); j++) {
			double d1 = std::min(plan[i].p1.dist2D(real[j].p1), plan[i].p1.dist2D(real[j].p2));
			double d2 = std::min(plan[i].p2.dist2D(real[j].p1), plan[i].p2.dist2D(real[j].p2));
			double d = (d1 + d2) / 2;
			if ( d < dmin) {
				dmin = d;
				mp[i] = j;
			}
		}
	}
	Doc_Item tab = sec->add_item("table");
	tab->add_item("title")->append("Accoppiamento tra strisciate progettate e volate");

	Poco::XML::AttributesImpl attr;
	attr.addAttribute("", "", "cols", "", "6");
	tab = tab->add_item("tgroup", attr);

	attr.clear();
	Doc_Item thead = tab->add_item("thead");
	Doc_Item row = thead->add_item("row");
	attr.addAttribute("", "", "align", "", "center");
	row->add_item("entry", attr)->append("Strisciata pianificata");
	row->add_item("entry", attr)->append("N. foto");
	row->add_item("entry", attr)->append("lung.");
	row->add_item("entry", attr)->append("Strisciata volata");
	row->add_item("entry", attr)->append("N. foto");
	row->add_item("entry", attr)->append("lung.");

	Doc_Item tbody = tab->add_item("tbody");
		
	std::map<int, int>::iterator itt;
	for ( itt = mp.begin(); itt != mp.end(); itt++) {
		row = tbody->add_item("row");
		int i = itt->first;
		row->add_item("entry", attr)->append(plan[i].id);
		std::stringstream s1; s1 << plan[i].n_fot;
		row->add_item("entry", attr)->append(s1.str());
		std::stringstream s2; s2 << plan[i].len;
		row->add_item("entry", attr)->append(s2.str());

		int j = itt->second;
		if (j != -1) {
			row->add_item("entry", attr)->append(real[j].id);
			std::stringstream s3; s3 << real[j].n_fot;
			row->add_item("entry", attr)->append(s3.str());
			std::stringstream s4; s4 << real[j].len;
			row->add_item("entry", attr)->append(s4.str());
		} else {
			row->add_item("entry", attr)->append("-");
			row->add_item("entry", attr)->append("-");
			row->add_item("entry", attr)->append("-");
		}
	}
	return true;
}
