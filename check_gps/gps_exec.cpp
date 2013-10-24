
#include "check_gps.h"
#include <set>
#include "Poco/autoPtr.h"
#include "Poco/stringtokenizer.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "ziplib/ziplib.h"
#include "Poco/String.h"
#include <spatialite.h>
#include <sstream>

void gps_exec::set_out_folder(const std::string& nome)
{
	_out_folder = nome;
}
//void gps_exec::set_rover_folder(const std::string& nome)
//{
//	_rover_folder = nome;
//}
//void gps_exec::set_base_folder(const std::string& nome)
//{
//	_base_folder = nome;
//}
std::string gps_exec::_getnome(const std::string& nome, gps_type type) 
{
	if ( type == rover_type )
		_rover_name.clear();
	if ( type == base_type )
		_sigla_base.clear();

	// seleziona tutti i files presenti nella cartella
	Poco::File dircnt(nome);
	std::vector<std::string> df;
	dircnt.list(df);
	std::vector<std::string> files;
	for (size_t i = 0; i < df.size(); i++) {
		Poco::Path fn(nome, df[i]);
		files.push_back(fn.toString());
	}

	std::set<std::string> sst;
	for (long i = 0; i < (long) files.size(); i++) {
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
void gps_exec::_record_base_file(const std::vector<DPOINT>& basi, const std::vector<std::string>& vs_base)
{
	std::string table("Basi");
	std::stringstream ss;
	char *err_msg = NULL;

	ss << "CREATE TABLE " << table << " (id INTEGER NOT NULL PRIMARY KEY,name TEXT NOT NULL)";
	int ret = sqlite3_exec (db_handle, ss.str().c_str(), NULL, NULL, &err_msg);
	if (ret != SQLITE_OK) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		//return;
	}
	ss.str("");	ss.clear(); 

	ss << "SELECT AddGeometryColumn('" << table << "', 'geom', 4326, 'POINT', 'XYZ')";
	ret = sqlite3_exec (db_handle, ss.str().c_str(), NULL, NULL, &err_msg);
	if ( ret != SQLITE_OK ) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return;
	}
	ret = sqlite3_exec (db_handle, "BEGIN", NULL, NULL, &err_msg);
	if ( ret != SQLITE_OK ) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		return;
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
	if (ret != SQLITE_OK) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
	}
}
bool gps_exec::SingleTrack(const std::string& nome, const std::string& code, std::vector<vGPS*>& vvg, MBR* mbr)
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

	ss << "CREATE TABLE " << table << 
		" (id INTEGER NOT NULL PRIMARY KEY,DATE TEXT NOT NULL,TIME TEXT NOT NULL,NSAT INTEGER NOT NULL,PDOP FLOAT NOT NULL,NBASI INTEGER NOT NULL,RMS DOUBLE NOT NULL )";
	int ret = sqlite3_exec (db_handle, ss.str().c_str(), NULL, NULL, &err_msg);
	if (ret != SQLITE_OK) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		//return;
	}
	ss.str("");	ss.clear(); 

	ss << "SELECT AddGeometryColumn('" << table << "', 'geom', 4326, 'POINT', 'XYZ')";
	ret = sqlite3_exec (db_handle, ss.str().c_str(), NULL, NULL, &err_msg);
	if ( ret != SQLITE_OK ) {
		fprintf (stderr, "Error: %s\n", err_msg);
		sqlite3_free (err_msg);
		//return;
	}

	long count = 0;

	std::set<std::string>::iterator it;
	long id = 1;
	for ( it = smap.begin(); it != smap.end(); it++) {
		std::pair<std::multimap<std::string, GRX>::iterator, std::multimap<std::string, GRX>::iterator> ret;
		std::multimap<std::string, GRX>::iterator itr;
		ret = mmap.equal_range(*it);

		double d = 0;
		int nsat = 0;
		double pdop = 1000.;
		double rms = -INF;
		std::string data;
		int nb = 0;
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
			nb++;

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
		if ( _gps_opt.Position_mode == GPS_OPT::Single)
			nb = 0;

		p.x /= d;
		p.y /= d;
		p.z /= d;

		ss.str("");	ss.clear(); 
		ss.precision(8);
		ss << "INSERT INTO " << table << " (id, DATE, TIME, NSAT, PDOP, NBASI, RMS, geom) VALUES (" << id++ <<
			", '" << data << "', '" << (*it).c_str() << "', " << nsat << ", " << pdop << ", " << nb << ", " << rms <<
			", GeomFromText('POINTZ(" << p.x << " " << p.y << " " << p.z << ")', 4326))";
		int ret1 = sqlite3_exec (db_handle, ss.str().c_str(), NULL, NULL, &err_msg);
		if ( ret1 != SQLITE_OK ) {
			fprintf (stderr, "Error: %s\n", err_msg);
			sqlite3_free (err_msg);
			continue;
		}


	}
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
bool gps_exec::RecordData(const std::string& nome, const std::string& code, vGPS& vg, MBR* mbr)
{
	return true;
}
bool gps_exec::run()
{
	//inizializza spatial lite
	spatialite_init(0);

	// imposta la massima distanza per le basi
	_gps_opt.max_base_dst = 30000.;

	Poco::Path fn(_proj_dir);
	fn.append("rilievo");

	Poco::File dircnt(fn);
	std::vector<std::string> files;
	dircnt.list(files);
	for (size_t i = 0; i < files.size(); i++) {
		Poco::Path fn(fn.toString(), files[i]);
		_mission_process(fn.toString());
	}
	return true;
}
bool gps_exec::_mission_process(const std::string& folder)
{
	// cartella dati rover non impostata
	if ( folder.empty() )
		return false;
		
	bool _single = true;

	// nome del file rover da elaborare
	std::string rover = _getnome(folder, rover_type);
	if ( rover.empty() )
		return false;

	std::string bas_fld = folder;
	std::vector<std::string> bfl;
	if ( !bas_fld.empty() ) {
		Poco::File dircnt(folder);
		std::vector<std::string> files;
		dircnt.list(files);
		std::vector<std::string> df;
		for (size_t i = 0; i < files.size(); i++) {
			Poco::Path fn(folder, files[i]);
			df.push_back(fn.toString());
		}
		// per ogni base prende l'elenco dei files da elaborare
		bfl.push_back(bas_fld);
		for (size_t i = 0; i < df.size(); i++) {
			Poco::Path base = Poco::Path(folder).pushDirectory(df[i]);
			if ( Poco::File(df[i]).exists() && Poco::File(df[i]).isDirectory() ) {
				bfl.push_back(df[i]);
			}
		}
	} else {
		// se base_folder è vuota si può fare il calcolo solo in modalità single
		if ( _gps_opt.Position_mode != GPS_OPT::Single )
			return false;
		bfl.push_back("X");
	}
	std::vector<vGPS*> vvg;

	int ret = sqlite3_open_v2(_db_name.c_str(), &db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	if ( ret != SQLITE_OK ) {
		fprintf (stderr, "cannot open '%s': %s\n", _db_name.c_str(), sqlite3_errmsg(db_handle));
		sqlite3_close(db_handle);
		db_handle = NULL;
		//return -1;
	}

	char *err_msg = NULL;
	ret = sqlite3_exec(db_handle, "SELECT InitSpatialMetadata()", NULL, NULL, &err_msg);
	if (ret != SQLITE_OK) {
		fprintf (stderr, "InitSpatialMetadata() error: %s\n", err_msg);
		sqlite3_free(err_msg);
		//return 0;
	}
	//int rrows;
 //   ret = load_shapefile (db_handle, "C:\\Google_drive\\Regione Toscana Tools\\Dati_test\\assi volo\\assi\\Castiglione_Scarlino",
	//				   "assi_volo", "cp1252", 32632,
	//				   "geom", 0,
	//				   0, 1,
	//				   1, &rrows,
	//				   err_msg);

	std::string dis;
	// attiva il calcolo per ogni vbase
	for ( size_t i = 0; i < bfl.size(); i++ ) {
		std::string base = _getnome(bfl[i], base_type);
		if ( base.empty() && _gps_opt.Position_mode != GPS_OPT::Single )
			continue;

		std::string nome = _rover_name + "_" + _sigla_base;
		Poco::Path out(folder, nome);
		out.setExtension("txt");
		//Poco::Path dis(Wrk.VecPath, nome, "gdf");
		_vs_base.push_back(_sigla_base);

		vGPS* vg = new vGPS;
		if ( RinexPost(rover, base, out.toString(), NULL, vg, &_gps_opt) ) {
			if ( _single )
				vvg.push_back(vg);
			else {
				RecordData(dis, "gps", *vg, NULL);
				delete vg;
			}
		}
	}
	std::string nome = _rover_name;


	if ( _single ) {
		SingleTrack(dis, "gps", vvg, NULL);
		for ( size_t i = 0; i < vvg.size(); i++)
			delete vvg[i];
	}

	sqlite3_close (db_handle);
	return true;
}
