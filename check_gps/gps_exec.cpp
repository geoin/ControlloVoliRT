/* 
	File: gps_exec.cpp
	Author:  F.Flamigni
	Date: 2013 October 22
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

#include "check_gps.h"
#include <set>
#include "Poco/autoPtr.h"
#include "Poco/stringtokenizer.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeParser.h"
#include "Poco/Util/XMLConfiguration.h"
#include "ziplib/ziplib.h"
#include "Poco/String.h"
#include "Poco/AutoPtr.h"
#include <spatialite.h>
#include <sstream>
#include <spatialite/gaiageo.h>
#include "photo_util/sun.h"

#define GPS "GPS"
#define BASI "Basi"
#define MISSIONI "missioni"
#define ASSI_VOLO "AVOLOV"
#define STRIP_NAME "A_VOL_CS"
#define REFSCALE "RefScale_2000"
#define SRID 32632 // SRID UTM32 wgs84
#define SRIDGEO 4326 // SRID lat lon wgs84
#define SHAPE_CHAR_SET "CP1252"

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::Path;
using Poco::File;
using namespace CV::Util::Spatialite;
using namespace CV::Util::Geometry;

/**************************************************************/
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
OGRGeomPtr GetGeom(QueryField& rs)
{
	std::vector<unsigned char> blob;
	rs.toBlob(blob);
	OGRGeomPtr og(blob);
	return og;
}
/****************************************************************/

gps_exec::~gps_exec()
{
}
void gps_exec::set_proj_dir(const std::string& nome)
{
	_proj_dir.assign(nome); 
}
void gps_exec::set_out_folder(const std::string& nome)
{
	_out_folder = nome;
}
bool gps_exec::run()
{
	try {
		// initialize spatial lite connection
		Poco::Path db_path(_proj_dir, "geo.sqlite");
		cnn.create(db_path.toString());
		cnn.initialize_metdata();

		int nrows = cnn.load_shapefile("C:/Google_drive/Regione Toscana Tools/Dati_test/assi volo/avolov",
		   ASSI_VOLO,
		   SHAPE_CHAR_SET,
		   SRID,
		   "geom",
		   true, // to have only XY
		   false,
		   false);

		_read_ref_val();

		// create the gps track
#ifdef PPP
		_create_gps_track();
#endif

		// add information to the fligth lines
		_update_assi_volo();
		
		// initialize docbook xml file
		_init_document();
		
		_final_check();
		
		// write the result on the docbook report
		_dbook.write();
	}
    catch(std::exception &e) {
        std::cout << std::string(e.what()) << std::endl;
    }
	return true;
}
void gps_exec::_init_document()
{
	Path doc_file(_proj_dir, "*");
	doc_file.setFileName("check_gps.xml");
	_dbook.set_name(doc_file.toString());	

	_article = _dbook.add_item("article");
	_article->add_item("title")->append("Collaudo  dati GPS");
}

bool gps_exec::_read_ref_val()
{
	Path ref_file(_proj_dir, "*");
	ref_file.popDirectory();
	ref_file.setFileName("Regione_Toscana_RefVal.xml");
	AutoPtr<XMLConfiguration> pConf;
	try {
		pConf = new XMLConfiguration(ref_file.toString());
		_MAX_PDOP = pConf->getDouble(get_key("MAX_PDOP"));
		_MIN_SAT = pConf->getInt(get_key("MIN_SAT"));
		_MAX_DIST = pConf->getInt(get_key("MAX_DIST")) * 1000;
		_MIN_SAT_ANG = pConf->getDouble(get_key("MIN_SAT_ANG"));
		_NBASI = pConf->getInt(get_key("NBASI"));
		_MIN_ANG_SOL = pConf->getDouble(get_key("MIN_ANG_SOL"));
	} catch (...) {
		return false;
	}
	return true;
}

std::string gps_exec::_getnome(const std::string& nome, gps_type type) 
{
	if ( type == rover_type )
		_rover_name.clear();
	if ( type == base_type )
		_sigla_base.clear();

	// seleziona tutti i files presenti nella cartella
	File dircnt(nome);
	std::vector<std::string> df;
	dircnt.list(df);
	std::vector<std::string> files;
	for (size_t i = 0; i < df.size(); i++) {
		Poco::Path fn(nome, df[i]);
		files.push_back(fn.toString());
	}

    std::set<std::string> sst;
    for (long i = 0; i < (long) files.size(); i++) {

        // *********************** Scompattatori da mettere nel modulo loader
        // per ogni file selezionato
		Poco::Path fn(files[i]);
		std::string ext = Poco::toLower(fn.getExtension());
		if ( !ext.size() )
			continue;
		if ( ext == "zip" ) {
			// se si tratta di uno zip
			MyUnZip mz(files[i]);
			if ( mz.IsValid() ) {
				std::vector< std::string  >& vs = mz.GetDir();
				for (size_t j = 0; j < vs.size(); j++) {
					// prende il nome del j-esimo file dello zip
					std::string st = vs[j];
					Poco::Path fn1(nome, st);

					// se non è già presente lo scompatta
					if ( std::find(files.begin(), files.end(), fn1.toString()) == files.end() ) {
						if ( mz.Extract(st.c_str()) ) {
							files.push_back(fn1.toString());
						}
					}
				}
				mz.Close();
			}
			continue;
		} else if ( ext == "z" ) {
			// nel caso di file z c'è un solo file
			MyUnZip mz(files[i].c_str());
			if ( mz.Extract(fn.toString().c_str()) ) {
				std::string st = mz.GetOutName();
				files.push_back(st);
			}
			continue;
		}
		// controlla se deve essere processato con Hatanaka
		std::string st = Hathanaka(fn.toString());
		if ( !st.empty() ) {
			files.push_back(st);
			continue;
		}
		//std::vector<std::string> vs;
		//// controlla se il file deve essere convertito da formato raw
		//vs = _rawConv(fn.toString());
		//if ( !vs.empty() ) {
		//	for (size_t i = 0; i < vs.size(); i++) {
		//		if ( std::find(files.begin(), files.end(), vs[i]) == files.end() )
		//			files.push_back(vs[i]);
		//	}
		//	continue;
		//}
        // ******************************************************************************

        if ( tolower(ext[2]) == 'o' ) {
			sst.insert(ext);
			if ( type == rover_type && _rover_name.empty() ) {
				_rover_name = Poco::Path(files[i]).getBaseName();
			}
			if ( type == base_type && _sigla_base.empty() ) {
					std::string base = Poco::Path(files[i]).getBaseName();
					_sigla_base = base.substr(0, 4);
			}
		}
	}
	if ( sst.size() != 1 )
		return "";

	Poco::Path fl(nome, "*");
	fl.setExtension(*sst.begin());
	return fl.toString();
}
bool gps_exec::_record_base_file(const std::vector<DPOINT>& basi, const std::vector<std::string>& vs_base)
{
	std::stringstream sql;
	sql << "CREATE TABLE " << BASI << 
		"(id INTEGER NOT NULL PRIMARY KEY, " << //id della stazione
		"nome TEXT NOT NULL)";			// base name
	cnn.execute_immediate(sql.str());

	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << BASI << "'," <<
		"'geom'," <<
		SRIDGEO << "," <<
		"'POINT'," <<
		"'XY')";
	cnn.execute_immediate(sql1.str());
	
	std::stringstream sql2;
	sql2 << "INSERT INTO " << BASI << " (id, nome, geom) \
		VALUES (?1, ?2, ST_GeomFromWKB(:geom, " << SRIDGEO << ") )";
	
	CV::Util::Spatialite::Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());	

	OGRSpatialReference sr;
	sr.importFromEPSG(SRIDGEO);

	for ( size_t i = 0; i < basi.size(); i++) {
		OGRGeometryFactory gf;
		OGRGeomPtr gp_ = gf.createGeometry(wkbPoint);
		gp_->setCoordinateDimension(2);
		gp_->assignSpatialReference(&sr);
		OGRPoint* gp = (OGRPoint*) ((OGRGeometry*) gp_);
		gp->setX(basi[i].x); gp->setY(basi[i].y);

		stm[1] = (int) (i + 1);
		stm[2] = vs_base[i];
		stm[3].fromBlob(gp_); 
		stm.execute();
		stm.reset();
	}
	cnn.commit_transaction();

	return true;
}
bool gps_exec::_single_track(const std::string& mission, std::vector< Poco::SharedPtr<vGPS> >& vvg, MBR* mbr)
{
	typedef struct {
		std::string data;
		DPOINT pos;
		int nsat;
		double pdop;
		double rms;
		int id_base;
		double dist;
	} GRX;

	std::vector<DPOINT> basi;

	std::multimap<std::string, GRX> mmap;
	std::set<std::string> smap;

	std::string actdate;
	std::string mintime, maxtime;
	// scandisce i dati relativi ad ogni base
	for (size_t l = 0; l < vvg.size(); l++) {
		vGPS& vg = *vvg[l];
		DPOINT pbase;

		for ( size_t i = 0; i < vg.size(); i++ ) {
			double x = RAD_DEG(vg[i].pos.x);
			double y = RAD_DEG(vg[i].pos.y);
			double z = vg[i].pos.z;

			if ( vg[i].data == "base" ) {
				pbase = DPOINT(x, y, z);
				basi.push_back(pbase);
				continue;
			}
			if ( mbr != NULL &&  !mbr->IsInside(vg[i].pos.x,  vg[i].pos.y) )
				continue;

			GRX gr;
			gr.pos = DPOINT(x, y, z);
			if ( _gps_opt.Position_mode != GPS_OPT::Single ) {
				gr.dist = gr.pos.dist_lat2D(pbase);
			} else {
				gr.dist = 0;
			}
			Poco::StringTokenizer tok(vg[i].data, ";");
			std::string time;
			if ( tok.count() >= 3 ) {
				Poco::StringTokenizer tok1(tok[0], " ");
				if ( tok1.count() == 2 ) {
					gr.data = tok1[0];
					time = tok1[1];

					if ( actdate.empty() )
						actdate = gr.data;
					if ( mintime.empty() )
						mintime = time;
					else {
						if ( strcmp(mintime.c_str(), time.c_str()) > 0 )
							mintime = time;
					}
					if ( maxtime.empty() )
						maxtime = time;
					else {
						if ( strcmp(maxtime.c_str(), time.c_str()) < 0 )
							maxtime = time;
					}
				}
				gr.nsat = atoi(tok[1].c_str());
				gr.pdop = atof(tok[2].c_str());
				if ( tok.count() >= 5 ) {
                    gr.rms = max(atof(tok[3].c_str()), atof(tok[4].c_str()));
				}
				gr.id_base = (int) l;
				mmap.insert(std::pair<std::string, GRX>(time, gr));
				smap.insert(time);
			}
		}
	}

	_record_base_file(basi, _vs_base);

	// crea la tabella del GPS
	std::stringstream sql;
	sql << "CREATE TABLE " << GPS << 
		" (id INTEGER NOT NULL PRIMARY KEY,\
		DATE TEXT NOT NULL,\
		TIME TEXT NOT NULL,\
		NSAT INTEGER NOT NULL,\
		PDOP FLOAT NOT NULL,\
		NBASI INTEGER NOT NULL,\
		RMS DOUBLE NOT NULL,\
		MISSION TEXT NOT NULL )";
	cnn.execute_immediate(sql.str());

	std::stringstream sql1;
	sql1 << "SELECT AddGeometryColumn('" << GPS << "'," <<
		"'geom'," <<
		SRIDGEO << "," <<
		"'POINT'," <<
		"'XY')";
	cnn.execute_immediate(sql1.str());

	std::stringstream sql2;
	sql2 << "INSERT INTO " << GPS << " (id, DATE, TIME, NSAT, PDOP, NBASI, RMS, MISSION, geom) \
		VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ST_GeomFromWKB(:geom, " << SRIDGEO << ") )";
	
	CV::Util::Spatialite::Statement stm(cnn);
	cnn.begin_transaction();
	stm.prepare(sql2.str());	

	int nsat;
	double pdop;
	double rms;
	std::string data;
	std::string time;
	int nbasi;
	long id = 1;

	OGRSpatialReference sr;
	sr.importFromEPSG(SRIDGEO);

	std::set<std::string>::iterator it;
	for ( it = smap.begin(); it != smap.end(); it++) {
		std::pair<std::multimap<std::string, GRX>::iterator, std::multimap<std::string, GRX>::iterator> ret;
		std::multimap<std::string, GRX>::iterator itr;
		ret = mmap.equal_range(*it);
		time = *it;

		double d = 0;
		nsat = 0;
		pdop = 1000.;
		rms = -INF;
		nbasi = 0;

		DPOINT p;
		for (itr = ret.first; itr != ret.second; ++itr) {
			GRX gr = (*itr).second;

			if ( gr.dist > _gps_opt.max_base_dst ) {
				continue;
			}
			double pi = 1.;
			if ( gr.dist != 0 )
				pi = 1. / gr.dist;
			d += pi;
			nbasi++;

			p += (gr.pos * pi);

			data = gr.data;
            nsat = max(nsat, gr.nsat);
            pdop = min(pdop, gr.pdop);
            rms = max(rms, gr.rms);
		}
		if ( d == 0  && _gps_opt.Position_mode != GPS_OPT::Single ) {
			//cnl.printf("Epoca scartata perché nessuna base");
			continue;
		}

		p /= d;
		
		OGRGeometryFactory gf;
		OGRGeomPtr gp_ = gf.createGeometry(wkbPoint);
		gp_->setCoordinateDimension(2);
		gp_->assignSpatialReference(&sr);
		OGRPoint* gp = (OGRPoint*) ((OGRGeometry*) gp_);
		*gp = OGRPoint(p.x, p.y);

		stm[1] = id++;
		stm[2] = data;
		stm[3] = time;
		stm[4] = nsat;
		stm[5] = pdop;
		stm[6] = nbasi;
		stm[7] = rms;
		stm[8] = mission;
		stm[9].fromBlob(gp_); 
		stm.execute();
		stm.reset();
	}
	cnn.commit_transaction();
	return true;
}

bool gps_exec::_create_gps_track()
{
	// imposta la massima distanza per le basi
	_gps_opt.max_base_dst = _MAX_DIST;
	_gps_opt.min_sat_angle = _MIN_SAT_ANG;

	Poco::Path fn(_proj_dir);
	fn.append(MISSIONI);

	cnn.remove_layer(BASI);
	cnn.remove_layer(GPS);

	// ogni cartella presente corrisponde ad una missione
	Poco::File dircnt(fn);
	std::vector<std::string> files;
	dircnt.list(files);
	for (size_t i = 0; i < files.size(); i++) {
		Poco::Path fn(fn.toString(), files[i]);
		Poco::File fl(fn);
		if ( fl.isDirectory() )
			_mission_process(fn.toString());
	}
	return true;
}
bool gps_exec::_mission_process(const std::string& folder)
{
	// cartella dati rover non impostata
	if ( folder.empty() )
		return false;
		
	// nome del file rover da elaborare
	std::string rover = _getnome(folder, rover_type);
	if ( rover.empty() )
		return false;

	// determina le basi da processare
	std::vector<std::string> bfl;
	Poco::File dircnt(folder);
	std::vector<std::string> files;
	dircnt.list(files);

	for (size_t i = 0; i < files.size(); i++) {
		Poco::Path fn(folder, files[i]);
		Poco::File fl(fn);
		if ( fl.isDirectory() )
			bfl.push_back(fn.toString());
	}

	std::vector< Poco::SharedPtr<vGPS> > vvg;

	// attiva il calcolo per ogni vbase
	for ( size_t i = 0; i < bfl.size(); i++ ) {
		std::string base = _getnome(bfl[i], base_type);
		if ( base.empty() )
			continue;

		std::string nome = _rover_name + "_" + _sigla_base;
		Poco::Path out(folder, nome);
		out.setExtension("txt");
		_vs_base.push_back(_sigla_base);

		fprintf (stderr, "Elaborazione %s\n", _sigla_base.c_str());

		Poco::SharedPtr<vGPS> vg(new vGPS);
		if ( RinexPost(rover, base, out.toString(), NULL, vg, &_gps_opt) )
			vvg.push_back(vg);
	}

	fprintf (stderr, "Registrazione dati \n", _sigla_base);
	std::string mission = Poco::Path(folder).getBaseName();
	_single_track(mission, vvg, NULL);

	return true;
}
void gps_exec::_add_column(const std::string& col_name) 
{
	std::stringstream sql;
	sql << "ALTER TABLE " << ASSI_VOLO << " ADD COLUMN " << col_name;
	try {
		cnn.execute_immediate(sql.str());
	}
	catch(std::exception e) {
	}
}
void gps_exec::_data_analyze()
{
}
typedef struct feature {
	std::string strip;
	std::string time;
	std::string date;
	std::string mission;
	int nsat;
	int nbasi;
	double pdop;
	OGRGeomPtr pt;
} feature;

void gps_exec::_update_assi_volo()
{
	_add_column("DATE TEXT");
	_add_column("TIME_S TEXT");
	_add_column("TIME_E TEXT");
	_add_column("MISSION TEXT");
	_add_column("SUN_HL DOUBLE");
	_add_column("NBASI INTEGER");
	_add_column("NSAT INTEGER");
	_add_column("PDOP DOUBLE");

	std::stringstream sql;
	sql << "SELECT a." << STRIP_NAME << " as strip, b.*, AsBinary(b.geom) as geo, min(st_Distance(st_PointN(ST_Transform(a.geom," << SRIDGEO << "), ?), b.geom)) FROM " <<
		ASSI_VOLO << " a, gps b group by strip";

	std::string ss(sql.str());

	size_t q = ss.find('?');

	Statement stm(cnn);

	ss.at(q) = '1';
	stm.prepare(ss);
	Recordset rs = stm.recordset();

	// determina l'istante GPS relativo al primo estremo dell'asse
	std::vector<feature> ft1;
	while ( !rs.eof() ) {
		feature f;
		f.strip = rs["strip"].toString();
		f.mission = rs["MISSION"].toString();
		f.time = rs["TIME"].toString();
		f.date = rs["DATE"].toString();
		f.nsat = rs["NSAT"].toInt();
		f.nbasi = rs["NBASI"].toInt();
		f.pdop = rs["PDOP"].toDouble();
		f.pt = GetGeom(rs["geo"]);
		ft1.push_back(f);
		rs.next();
	}

	ss.at(q) = '2';
	stm.prepare(ss);
	rs = stm.recordset();

	// determina l'istante GPS relativo al secondo estremo dell'asse
	std::vector<feature> ft2;
	while ( !rs.eof() ) {
		feature f;
		f.strip = rs["strip"].toString();
		f.mission = rs["MISSION"].toString();
		f.time = rs["TIME"].toString();
		f.date = rs["DATE"].toString();
		f.nsat = rs["NSAT"].toInt();
		f.nbasi = rs["NBASI"].toInt();
		f.pdop = rs["PDOP"].toDouble();
		f.pt = GetGeom(rs["geo"]);
		ft2.push_back(f);
		rs.next();
	}

	std::stringstream sql1;
	sql1 << "UPDATE " << ASSI_VOLO << " SET MISSION=?1, DATE=?2, TIME_S=?3, TIME_E=?4, NSAT=?5, PDOP=?6, NBASI=?7, SUN_HL=?8 where " << STRIP_NAME  << "=?9";
	Statement stm1(cnn);
	stm1.prepare(sql1.str());
	cnn.begin_transaction();

	// per ogni strip determina i parametri gps con cui è stata acquisita
	for ( size_t i = 0; i < ft1.size(); i++) {
		const std::string & val = ft1[i].strip;
		std::string t1 = ft1[i].time;
		for ( size_t j = 0; j < ft2.size(); j++) {
			if ( ft2[j].strip == ft1[i].strip ) {
				std::string t2 = ft2[j].time;
				if ( t1 > t2 )
					std::swap(t1, t2);

				std::stringstream sql;
				sql << "SELECT MISSION, DATE, min(NSAT) NSAT, max(PDOP) PDOP, min(NBASI) NBASI from " << GPS << " where TIME >= '" << t1 << "' and TIME <= '" << t2 << "'";
				stm.prepare(sql.str());
				rs = stm.recordset();
				if ( rs.eof() )
					return;

				// determina l'altezza media del sole sull'orizzonte
				OGRPoint* pt = (OGRPoint*) ((OGRGeometry*) ft1[i].pt);
				Sun sun(pt->getY(), pt->getX());
				int td;
				std::stringstream ss2;
				ss2 << ft1[i].date << " " << t1;
				Poco::DateTime dt = Poco::DateTimeParser::parse(ss2.str(), td);
				sun.calc(dt.year(), dt.month(), dt.day(), dt.hour());
				double h = sun.altit();

				stm1[1] = rs[0].toString(); // mission
				stm1[2] = rs[1].toString(); // date
				stm1[3] = t1;
				stm1[4] = t2;
				stm1[5] = rs[2].toInt(); // minimal number of satellite
				stm1[6] = rs[3].toDouble(); // max pdop
				stm1[7] = rs[4].toInt(); // number of bases
				stm1[8] = h;	// sun elevation
				stm1[9] = val;
				stm1.execute();
				stm1.reset();
				break;
			}
		}
	}
	cnn.commit_transaction();
}
void gps_exec::_final_check()
{
	// check finale
	std::stringstream sql;
	sql << "SELECT " << STRIP_NAME << ", MISSION, DATE, NSAT, PDOP, NBASI, SUN_HL from " << ASSI_VOLO <<  " where NSAT<" << _MIN_SAT <<
		" OR PDOP >" << _MAX_PDOP << " OR NBASI <" << _NBASI << " OR SUN_HL <" << _MIN_ANG_SOL;

	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();

	//std::vector<feature> ft1;
	while ( !rs.eof() ) {
		//ft1.push_back(f);
		;//rs.eof
	}
}