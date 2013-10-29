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
#define ASSI_VOLO "assi_volo"
#define STRIP_NAME "A_VOL_CS"
#define REFSCALE "RefScale_2000"

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::Path;
using Poco::File;

std::string get_key(const std::string& val)
{
	return std::string(REFSCALE) + "." + val;
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
void gps_exec::set_out_folder(const std::string& nome)
{
	_out_folder = nome;
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
		std::string st = _hathanaka(fn.toString());
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
	std::string table(BASI);
	std::stringstream ss;
	char *err_msg = NULL;

	ss << "CREATE TABLE " << table << " (id INTEGER NOT NULL PRIMARY KEY,name TEXT NOT NULL)";
	int ret = sqlite3_exec (db_handle, ss.str().c_str(), NULL, NULL, &err_msg);
	if (ret != SQLITE_OK) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return false;
	}
	ss.str("");	ss.clear(); 

	ss << "SELECT AddGeometryColumn('" << table << "', 'geom', 4326, 'POINT', 'XYZ')";
	ret = sqlite3_exec (db_handle, ss.str().c_str(), NULL, NULL, &err_msg);
	if ( ret != SQLITE_OK ) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return false;
	}
	ret = sqlite3_exec (db_handle, "BEGIN", NULL, NULL, &err_msg);
	if ( ret != SQLITE_OK ) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return false;
	}

	ss.precision(8);
	for ( size_t i = 0; i < basi.size(); i++) {
		ss.str("");	ss.clear(); 
		ss << "INSERT INTO " << table << " (id, name, geom) VALUES (" << i + 1 << ", '" << vs_base[i] << 
			"', GeomFromText('POINTZ(" << basi[i].x << " " << basi[i].y << " " << basi[i].z << ")', 4326))";
		ret = sqlite3_exec (db_handle, ss.str().c_str(), NULL, NULL, &err_msg);
		if ( ret != SQLITE_OK ) {
			fprintf (stderr, "Error: %s\n", err_msg);
			sqlite3_free (err_msg);
			continue;
		}
	}
	ret = sqlite3_exec (db_handle, "COMMIT", NULL, NULL, &err_msg);
	if ( ret != SQLITE_OK ) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return false;
	}
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

	std::string table("Gps");
	std::stringstream ss;
	char *err_msg = NULL;

	// crea la tabella del GPS
	ss << "CREATE TABLE " << table << 
		" (id INTEGER NOT NULL PRIMARY KEY,\
		DATE TEXT NOT NULL,\
		TIME TEXT NOT NULL,\
		NSAT INTEGER NOT NULL,\
		PDOP FLOAT NOT NULL,\
		NBASI INTEGER NOT NULL,\
		RMS DOUBLE NOT NULL,\
		MISSION TEXT NOT NULL )";
	int ret = sqlite3_exec (db_handle, ss.str().c_str(), NULL, NULL, &err_msg);
	if (ret != SQLITE_OK) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return false;
	}

	// aggiunge la colonna geometrica
	ss.str("");	ss.clear(); 
	ss << "SELECT AddGeometryColumn('" << table << "', 'geom', 4326, 'POINT', 'XYZ')";
	ret = sqlite3_exec (db_handle, ss.str().c_str(), NULL, NULL, &err_msg);
	if ( ret != SQLITE_OK ) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return false;
	}

	ret = sqlite3_exec (db_handle, "BEGIN", NULL, NULL, &err_msg);
	if (ret != SQLITE_OK) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return false;
	}

	ss.str("");	ss.clear(); 
	ss << "INSERT INTO " << table << " (id, DATE, TIME, NSAT, PDOP, NBASI, RMS, MISSION, geom) \
		VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

	ret = sqlite3_prepare_v2 (db_handle, ss.str().c_str(), ss.str().size(), &stmt, NULL);
	if ( ret != SQLITE_OK ) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return false;
	}

	int nsat;
	double pdop;
	double rms;
	std::string data;
	std::string time;
	int nbasi;
	long id = 1;

	int blob_size;

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

			p.x += gr.pos.x * pi;
			p.y += gr.pos.y * pi;
			p.z += gr.pos.z * pi;

			data = gr.data;
            nsat = max(nsat, gr.nsat);
            pdop = min(pdop, gr.pdop);
            rms = max(rms, gr.rms);
		}
		if ( d == 0  && _gps_opt.Position_mode != GPS_OPT::Single ) {
			//cnl.printf("Epoca scartata perché nessuna base");
			continue;
		}

		p.x /= d;
		p.y /= d;
		p.z /= d;

		sqlite3_reset(stmt);
		sqlite3_clear_bindings(stmt);

		gaiaGeomCollPtr geo = gaiaAllocGeomColl ();
		geo->Srid = 4326;
		geo->DimensionModel  = GAIA_XY_Z;
		geo->DeclaredType = GAIA_POINTZ;
		gaiaAddPointToGeomCollXYZ(geo, p.x, p.y, p.z);
		unsigned char *blob;
		gaiaToSpatiaLiteBlobWkb(geo, &blob, &blob_size);

		// we can now destroy the geometry object
		gaiaFreeGeomColl (geo);

		// binding parameters to Prepared Statement
		sqlite3_bind_int(stmt, 1, id++);
		sqlite3_bind_text(stmt, 2, data.c_str(), data.size(), SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, time.c_str(), time.size(), SQLITE_STATIC);
		sqlite3_bind_int(stmt, 4, nsat);
		sqlite3_bind_double(stmt, 5, pdop);
		sqlite3_bind_int(stmt, 6, nbasi);
		sqlite3_bind_double(stmt, 7, rms);
		sqlite3_bind_text(stmt, 8, mission.c_str(), mission.size(), SQLITE_STATIC);
		sqlite3_bind_blob (stmt, 9, blob, blob_size, SQLITE_STATIC);
		
		int retv = sqlite3_step(stmt);
		if ( retv != SQLITE_DONE && retv != SQLITE_ROW) {
		      printf ("sqlite3_step() error: %s\n", sqlite3_errmsg (db_handle));
		      sqlite3_finalize (stmt);
		      break;
		  }
		gaiaFree(blob);
	}
	sqlite3_finalize(stmt);
	ret = sqlite3_exec (db_handle, "COMMIT", NULL, NULL, &err_msg);
	if (ret != SQLITE_OK) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
	}
	return true;
}

std::vector<std::string> gps_exec::_rawConv(const std::string& nome) 
{
	Poco::Path fn(nome);
	std::string ext = fn.getExtension();
	std::vector<std::string> vs;
	Raw2Rnx(nome.c_str(), ext.c_str(), vs);
	return vs;
}
std::string gps_exec::_hathanaka(const std::string& nome)
{
	Poco::Path fn(nome);
	std::string ext = fn.getExtension();
	if ( tolower(ext[2]) == 'd' ) {
		Poco::Path fn1(nome);
		std::string ext1 = ext;
		ext1[2] = 'o';
		fn1.setExtension(ext1);
		Poco::File f(fn1.toString());
		if ( !f.exists() )
			if ( Crx2Rnx(nome.c_str()) != 1 ) {
				return fn1.toString();
			}
	}
	return std::string("");
}
gps_exec::~gps_exec()
{
	//if ( db_cache != NULL )
		spatialite_cleanup();
	if ( db_handle != NULL )
		sqlite3_close_v2(db_handle);
}

bool gps_exec::_init_splite()
{
	try {
		//inizializza spatial lite
		//spatialite_init(0);

		// costruisce il nome dl db
		//std::string db_name  = Poco::Path(_proj_dir).getBaseName();
		Poco::Path db_path(_proj_dir, "geo.sqlite");
		//db_path.setExtension("sqlite");
		 
		// connette il db sqlite
		int ret = sqlite3_open_v2(db_path.toString().c_str(), &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if ( ret != SQLITE_OK ) {
			fprintf (stderr, "cannot open '%s': %s\n", _db_name.c_str(), sqlite3_errmsg(db_handle));
			//sqlite3_close(db_handle);
			//db_handle = NULL;
			return false;
		}
		db_cache = spatialite_alloc_connection ();
		spatialite_init_ex(db_handle, db_cache, 0);

		// inizializza i metadati spaziali
		char *err_msg = NULL;
		ret = sqlite3_exec(db_handle, "SELECT InitSpatialMetadata()", NULL, NULL, &err_msg);
		if (ret != SQLITE_OK) {
			fprintf (stderr, "InitSpatialMetadata() error: %s\n", err_msg);
			sqlite3_free(err_msg);
			return false;
		}
		// legge i valori di riferimento per la verifica
		_read_ref_val();
	} catch (Poco::Exception& e) {
		fprintf(stderr, "Error: %s\n", e.what());
		return false;
	}
	return true;
}
bool gps_exec::run()
{
	try {
		// inizializza la connessione con spatial lite
		if ( !_init_splite() )
			return false;
		// cancella la tabella del gps
		char *err_msg = NULL;
		std::stringstream ss;
		ss << "DROP TABLE " << GPS;
		sqlite3_exec(db_handle, ss.str().c_str(), NULL, NULL, &err_msg);

		// cancella la tabella delle basi
		std::stringstream ss1;
		ss1 << "DROP TABLE " << BASI;
		sqlite3_exec(db_handle, ss1.str().c_str(), NULL, NULL, &err_msg);
		
		// imposta la massima distanza per le basi
		_gps_opt.max_base_dst = _MAX_DIST;
		_gps_opt.min_sat_angle = _MIN_SAT_ANG;

		Poco::Path fn(_proj_dir);
		fn.append(MISSIONI);

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

		// chiude la connessione sqlite
		//sqlite3_close (db_handle);
	} 
	catch (...) {
		fprintf (stderr, "Exception catched\n");
		return false;
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

	int rrows;
	char* err_msg = NULL;
    load_shapefile (db_handle, "C:\\Google_drive\\Regione Toscana Tools\\Dati_test\\assi volo\\AVOLOV",
					   ASSI_VOLO, "cp1252", 32632,
					   "geom", 0,
					   0, 1,
					   1, &rrows,
					   err_msg);
	sqlite3_free(err_msg);


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
void gps_exec::data_analyze()
{
	// inizializza la connessione con spatial lite
	if ( !_init_splite() )
		return;

	char *err_msg = NULL;
	std::stringstream ss;
	ss << "ALTER TABLE " << ASSI_VOLO << " ADD COLUMN ";
	std::string query;

	// modifica la tabella degli assi di volo aggiungendo i campi relativi ai parametri oggetto di verifica
	try {
		query = ss.str() + "DATE TEXT";
		if ( sqlite3_exec(db_handle, query.c_str(), NULL, NULL, &err_msg) != SQLITE_OK )
			throw std::runtime_error(err_msg);
		query = ss.str() + "TIME TEXT";
		if ( sqlite3_exec(db_handle, query.c_str(), NULL, NULL, &err_msg) != SQLITE_OK )
			throw std::runtime_error(err_msg);
		query = ss.str() + "MISSION TEXT";
		if ( sqlite3_exec(db_handle, query.c_str(), NULL, NULL, &err_msg) != SQLITE_OK )
			throw std::runtime_error(err_msg);
		query = ss.str() + "SUN_HL DOUBLE";
		if ( sqlite3_exec(db_handle, query.c_str(), NULL, NULL, &err_msg) != SQLITE_OK )
			throw std::runtime_error(err_msg);
		query = ss.str() + "NBASI INTEGER";
		if ( sqlite3_exec(db_handle, query.c_str(), NULL, NULL, &err_msg) != SQLITE_OK )
			throw std::runtime_error(err_msg);
		query = ss.str() + "NSAT INTEGER";
		if ( sqlite3_exec(db_handle, query.c_str(), NULL, NULL, &err_msg) != SQLITE_OK )
			throw std::runtime_error(err_msg);
		query = ss.str() + "PDOP DOUBLE";
		if ( sqlite3_exec(db_handle, query.c_str(), NULL, NULL, &err_msg) != SQLITE_OK )
			throw std::runtime_error(err_msg);
	} catch (std::runtime_error& e) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free(err_msg);
	}

	// aggiorna la tabella degli assi di volo con i dati della traccia gps
	_update_assi_volo();
	_final_check();
	return;
}

class feature {
public:
	std::string operator[](const std::string& nome) {
		std::string s("");
		if ( attributes.find(nome) == attributes.end() )
			return s;
		s = attributes[nome];
		return s;
	}
	gaiaGeomCollPtr geom;
	std::map<std::string, std::string> attributes;
	void clear(void) {
		attributes.clear();
	}
};
class splite_query {
public:
	splite_query(): _db_handle(NULL), _stmt(NULL) {}
	splite_query(sqlite3* db): _db_handle(db), _stmt(NULL) {}
	bool prepare(const std::string& query) {
		int ret = sqlite3_prepare_v2(_db_handle, query.c_str(), query.size(), &_stmt, NULL);
		if ( ret != SQLITE_OK )
			return false;
		_n_columns = sqlite3_column_count(_stmt);
		return true;
	}
	bool get_next(feature& f) {
		f.clear();
		int ret = sqlite3_step(_stmt);
		if ( ret == SQLITE_DONE ) {
			// there are no more rows to fetch - we can stop looping
		      return false;
		}
		if ( ret == SQLITE_ROW ) {
			for (int ic = 0; ic < _n_columns; ic++) {
				std::string nome = (char*) sqlite3_column_name(_stmt, ic);
				switch ( sqlite3_column_type(_stmt, ic) ) {
				case SQLITE_BLOB: {
					const void* blob = sqlite3_column_blob(_stmt, ic);
					int blob_size = sqlite3_column_bytes(_stmt, ic);
					// checking if this BLOB actually is a GEOMETRY
					gaiaGeomCollPtr geom = gaiaFromSpatiaLiteBlobWkb((unsigned char*) blob, blob_size);
					//gaiaFree((void*) blob);
					if ( geom )
						f.geom = geom;
					break;
				} 
				case SQLITE_TEXT: {
					std::string val = (char*) sqlite3_column_text(_stmt, ic);
					f.attributes[nome] = val;
					break;
				}
				case SQLITE_FLOAT: {
					double val = sqlite3_column_double(_stmt, ic);
					std::stringstream ss;
					ss.precision(5);
					ss << val;
					f.attributes[nome] = ss.str();
					break;
				}
				case SQLITE_INTEGER: {
					int val = sqlite3_column_int(_stmt, ic);
					std::stringstream ss;
					ss << val;
					f.attributes[nome] = ss.str();
					break;
				}
				}
			}
		}
		return true;
	}
	void close(void)  {
		if ( _stmt != NULL )
			sqlite3_finalize(_stmt);
		_stmt = NULL;
	}
private:
	sqlite3* _db_handle;
	sqlite3_stmt* _stmt;
	int _n_columns;
};

void gps_exec::_update_assi_volo()
{
	std::stringstream sst;
	sst << "SELECT a." << STRIP_NAME << " as p1, b.*, min(st_Distance(st_PointN(ST_Transform(a.geom, 4326), ?), b.geom)) FROM " <<
		ASSI_VOLO << " a, gps b group by p1";

	std::string ss(sst.str());

	size_t q = ss.find('?');

	splite_query sq(db_handle);

	ss.at(q) = '1';
	if ( !sq.prepare(ss) )
		return;

	feature f;

	// determina l'istante GPS relativo al primo estremo dell'asse
	std::vector<feature> ft1;
	while ( sq.get_next(f) )
		ft1.push_back(f);
	sq.close();

	ss.at(q) = '2';
	if ( !sq.prepare(ss) )
		return;

	// determina l'istante GPS relativo al secondo estremo dell'asse
	std::vector<feature> ft2;
	while ( sq.get_next(f) )
		ft2.push_back(f);
	sq.close();

	// per ogni strip determina i parametri gps con cui è stata acquisita
	for ( size_t i = 0; i < ft1.size(); i++) {
		const std::string & val = ft1[i].operator []("p1");
		std::string t1 = ft1[i].operator []("TIME");
		for ( size_t j = 0; j < ft2.size(); j++) {
			if ( ft2[j].operator []("p1") == val ) {
				std::stringstream ss;
				std::string t2 = ft2[j].operator []("TIME");
				if ( t1 > t2 )
					std::swap(t1, t2);
				ss << "SELECT MISSION, DATE, min(NSAT) NSAT, max(PDOP) PDOP, min(NBASI) NBASI from " << GPS << " where TIME >= '" << t1 << "' and TIME <= '" << t2 << "'";
				//else
				//	ss << "SELECT MISSION, DATE, min(NSAT) NSAT, max(PDOP) PDOP, min(NBASI) NBASI from " << GPS << " where TIME >= '" << t2 << "' and TIME <= '" << t1 << "'";

				if ( !sq.prepare(ss.str()) )
					return;
				if ( !sq.get_next(f) )
					return;
				sq.close();

				// determina l'altezza media del sole sull'orizzonte
				Sun sun(ft1[i].geom->FirstPoint->Y, ft1[i].geom->FirstPoint->X);
				int td;
				std::stringstream ss2;
				ss2 << f["DATE"] << " " << t1;
				Poco::DateTime dt = Poco::DateTimeParser::parse(ss2.str(), td);
				sun.calc(dt.year(), dt.month(), dt.day(), dt.hour());
				double h = sun.altit();

				std::stringstream ss1;
				ss1 << "update " << ASSI_VOLO <<" SET MISSION='" << f["MISSION"] << "', DATE='" << f["DATE"] <<
					"', NSAT=" << f["NSAT"] << ", PDOP=" << f["PDOP"] << ", NBASI=" <<
					f["NBASI"] << ", SUN_HL=" << h << " where " << STRIP_NAME  << "='" << val << "'";

				char *err_msg = NULL;
				int ret = sqlite3_exec(db_handle, ss1.str().c_str(), NULL, NULL, &err_msg);
				if ( ret != SQLITE_OK ) {
					fprintf (stderr, "Error: %s\n", err_msg);
					sqlite3_free (err_msg);
					return;
				}
				break;
			}
		}
	}
}
void gps_exec::_final_check()
{
	// check finale
	std::stringstream ssq;
	ssq << "SELECT MISSION, DATE, NSAT PDOP, NBASI, SUN_HL from " << ASSI_VOLO <<  " where NSAT<" << _MIN_SAT <<
		" OR PDOP >" << _MAX_PDOP << "OR NBASI <" << _NBASI << " OR SUN_HL <" << _MIN_ANG_SOL;

	splite_query sq(db_handle);
	if ( !sq.prepare(ssq.str()) )
		return;

	feature f;

	std::vector<feature> ft1;
	while ( sq.get_next(f) )
		ft1.push_back(f);
	sq.close();
int a = 1;
}