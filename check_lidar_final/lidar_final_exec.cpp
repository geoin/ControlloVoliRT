#include "lidar_final_exec.h"
#include "common/statistics.h"

#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/String.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/AutoPtr.h"
#include "Poco/LocalDateTime.h"
#include "Poco/NumberParser.h"

#include "proj_api.h"
#include "PJ_igmi.h"
#include "common/logger.h"

#include "CVUtil/ogrgeomptr.h"

#include <assert.h>
#include <algorithm>
#include <random>

#include <algorithm>
#include <numeric>
#include <time.h>
#include <set>

#include "sampler.h"

#define GEO_DB_NAME "geo.sqlite"
#define LIDAR "Lidar"

using namespace CV;
using namespace CV::Util::Geometry;

Logger Check_log;

std::string get_key(const std::string& val) {
    return std::string(LIDAR) + "." + val;
}

std::istream& _Getline(std::istream& is, std::string& t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case EOF:
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:
            t += (char)c;
        }
    }
}

class ortometric {
public:
    ortometric(const std::string& gridFile): _gridFile(gridFile) {
        utm = pj_init_plus("+proj=utm +ellps=WGS84 +zone=32 +datum=WGS84 +units=m +no_defs");
        wgs84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
        InitIGMIgrid();
    }

    bool InitIGMIgrid( ) {
        std::vector<Poco::Path> files;

        vGrid::HGRID_TYPE type;
        Poco::Path g( _gridFile );
        if( Poco::toLower( g.getExtension() ) == "gk1" || Poco::toLower( g.getExtension() ) == "gk2" ) {
            type = vGrid::ty_GK;
            files.push_back( g );
        } else if( Poco::toLower( g.getExtension() ) == "gr1" || Poco::toLower( g.getExtension() ) == "gr2" ) {
            type = vGrid::ty_GR;
            files.push_back( g );
        } else {
            std::fstream in;
            in.open( _gridFile, std::ios_base::in );

            std::string line;
            while( _Getline( in, line ) ) {
                Poco::Path t( line );
                if( Poco::toLower( t.getExtension() ) == "gk1" || Poco::toLower( t.getExtension() ) == "gk2" ) {
                    type = vGrid::ty_GK;
                    files.push_back( t );
                } else if( Poco::toLower( t.getExtension() ) == "gr1" || Poco::toLower( t.getExtension() ) == "gr2" ) {
                    type = vGrid::ty_GR;
                    files.push_back( t );
                }
            }
        }

        bool ret = true;
        for( std::vector<Poco::Path>::iterator it = files.begin(); it != files.end(); it++ ) {
            if( it == files.begin() ) {
                ret |= grid.Init( it->toString().c_str(), type );
            } else {
                ret |= grid.MergeGrid( it->toString().c_str() );
            }
        }
        return true;
    }

    double ortoQ(double x, double y, double z )  {
        double lat(y);
        double lon(x);
        double quote(z);
        pj_transform(utm, wgs84, 1, 1, &lon, &lat, &quote);
        double diff(0);
        if (grid.GetCorrections(lat, lon, &diff)) {
            quote -= diff;
        } else
         quote = Z_NOVAL;
        return quote;
    }

private:
    std::string _gridFile;
    vGrid grid;
    projPJ utm; // = pj_init_plus("+proj=utm +ellps=WGS84 +zone=32 +datum=WGS84 +units=m +no_defs");
    projPJ wgs84; // = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

};

lidar_final_exec::lidar_final_exec(): LID_ANG_SCAN( 0 ), LID_TOL_A(0) {
	srand(time(NULL));
	_coversAll = false;

	
	_tilePP = 10, _classFP = 10, _classPP = 10, _resFP = 10, _resPP = 10, _qPP = 10;
}

void lidar_final_exec::set_proj_dir(const std::string& proj) {
    _proj_dir = proj;
}

bool lidar_final_exec::readReference() {
	try {
		Poco::Path ref_file( _proj_dir, "*" );
		ref_file.setFileName( "refval.xml" );
		Poco::AutoPtr<Poco::Util::XMLConfiguration> pConf;
		pConf = new Poco::Util::XMLConfiguration( ref_file.toString() );
		//LID_TOL_Z = pConf->getDouble( get_key( "LID_TOL_Z" ) );
		LID_TOL_A = pConf->getDouble( get_key( "LID_TOL_A" ) );
		LID_ANG_SCAN = pConf->getDouble( get_key( "LID_ANG_SCAN" ) );
		return true;
	}
	catch( const std::exception& e ) {
		Error( "Reading reference values..", e );
		return false;
	}
}

bool lidar_final_exec::run() {
    Poco::Path lp(_proj_dir, "Lidar_final.log");
    Check_log.Init(lp.toString());

    Poco::LocalDateTime dt;
    Check_log << dt.day() << "/" << dt.month() << "/" << dt.year() << "  " << dt.hour() << ":" << dt.minute() << std::endl;

    if (!openDBConnection() ) {
        return false;
    }
	if( !readReference() )
		return false;

    std::string str;
    if (!GetProjData(cnn, _note, str)) {
		return false;
	}

    readFolders();

    // usando i dati ground o overground determina l'ingombro dei fogli e lo sottrae da quello di carto
    Check_log << "Analisi copertura aree da rilevare.." << std::endl;
    //_checkBlock();
	
    // confronta che il dato tile ground contenga gli stessi elementi degli altri oggetti
    Check_log << "Analisi completezza dati.." << std::endl;
    _checkEquality();
	
	// verifica che il dato groud abbia corrispondenti nelle strip
    Check_log << "Analisi classificazione ground" << std::endl;
    //_checkRawRandom( _groundEll, MyLas::last_pulse, groundRandomDiffg );

    // come sopra ma per il dato overground
    Check_log << "Analisi classificazione overground" << std::endl;
    //_checkRawRandom( _overgroundEll, MyLas::first_pulse, overRandomDiff );
	
//    Check_log << "Analisi classificazione.." << std::endl;
//    _checkEllipsoidicData();
	
//    Check_log << "Analisi conversione ground grid" << std::endl;
//	_checkQuota(_groundEll, _groundOrto, statsGround);
	
//    Check_log << "Analisi conversione overgground grid" << std::endl;
//	_checkQuota(_overgroundEll, _overgroundOrto, statsOverGround);
	
    // Verifica che ricampionamento e conversione quote produca risultati congrui
    Check_log << "Analisi ricampionamento ground ortometrico.." << std::endl;
    _checkResamples( _groundEll, _groundEllList, _mdt, _mdtList, diffMdt);

//    Check_log << "Analisi ricampionamento overground ortometrico.." << std::endl;
//	_checkResamples(_overgroundOrto, _overgroundOrtoList, _mds, _mdsList, diffMds);

    createReport();

    Poco::LocalDateTime dte;
    Check_log << dte.day() << "/" << dte.month() << "/" << dte.year() << "  " << dte.hour() << ":" << dte.minute() << std::endl;

	return true;
}

void lidar_final_exec::createReport() {
    std::string title ="Collaudo dati finali";

	Poco::Path doc_file(_proj_dir, "*");
	doc_file.setFileName("check_lidar_final.xml");

	init_document(_dbook, doc_file.toString(), title, _note);
	char* dtd_ = getenv("DOCBOOKRT");
	std::string dtd;
	if (dtd_ != NULL) {
		dtd = std::string("file:") + dtd_;
	}

	_dbook.set_dtd(dtd);
	_article = _dbook.get_item("article");

    Check_log << "Produzione del report finale: " << _dbook.name() << std::endl;
	
	_reportBlock();
	_reportEquality();

    _reportRawRandom(groundRandomDiffg);
    _reportRawRandom(overRandomDiff);

	_reportEllipsoidic();
	
	_reportQuota();
	_reportResamples();

	_dbook.write();	
}

void lidar_final_exec::readFolders() {
	_step = 0;

//	try {
        // tile raw non più in uso
//		_raw = _getRawFolder("FINAL_RAW_STRIP_DATA", _step);
//		_getCoordNameList(_raw, "las", _rawList);
//	} catch (const std::exception& ex) {
//        Check_log << "Dati grezzi non inseriti" << std::endl;
//	}

	_groundEll = _getFolder("FINAL_GROUND_ELL");
	_getCoordNameList(_groundEll, "xyzi", _groundEllList);

	_overgroundEll = _getFolder("FINAL_OVERGROUND_ELL");
	_getCoordNameList(_overgroundEll, "xyzi", _overgroundEllList);

	//_groundOrto = _getFolder("FINAL_GROUND_ORTO");
	//_getCoordNameList(_groundOrto, "xyzic", _groundOrtoList);

	//_overgroundOrto = _getFolder("FINAL_OVERGROUND_ORTO");
	//_getCoordNameList(_overgroundOrto, "xyzic", _overgroundOrtoList);
	
	_mds = _getFolder("FINAL_MDS");
	_getCoordNameList(_mds, "asc", _mdsList, true);

	_mdt = _getFolder("FINAL_MDT");
	_getCoordNameList(_mdt, "asc", _mdtList, true);

	_intensity = _getFolder("FINAL_INTENSITY");
	_getCoordNameList(_intensity, "tif", _intensityList, true);
	_getCoordNameList(_intensity, "tiff", _intensityList, true);

	CV::Util::Spatialite::Statement stm(cnn);
	stm.prepare("SELECT GRID FROM FINAL_IGM_GRID");
	CV::Util::Spatialite::Recordset set = stm.recordset();
	if (set.eof()) {
		throw std::runtime_error("No data in FINAL_IGM_GRID");
	}
	_gridFile = _proj_dir + "/" + set["GRID"].toString();
}

std::string lidar_final_exec::_getFolder(const std::string& table) {
	CV::Util::Spatialite::Statement stm(cnn);
	stm.prepare("SELECT FOLDER FROM " + table);
	CV::Util::Spatialite::Recordset set = stm.recordset();
	if (set.eof()) {
		throw std::runtime_error("No data in " + table);
	}
	return set["FOLDER"].toString();
}
	
std::string lidar_final_exec::_getRawFolder(const std::string& table, unsigned int& step) {
	CV::Util::Spatialite::Statement stm(cnn);
	stm.prepare("SELECT FOLDER, TILE_SIZE FROM " + table);
	CV::Util::Spatialite::Recordset set = stm.recordset();
	if (set.eof()) {
		throw std::runtime_error("No data in " + table);
	}
	step = set["TILE_SIZE"].toInt();
	return set["FOLDER"].toString();
}

bool lidar_final_exec::openDBConnection() {
	try {
		Poco::Path geodb(_proj_dir, GEO_DB_NAME);
		cnn.open(geodb.toString());
		return true;
	} catch (const std::exception& e) {
		Error("Opening connection..", e);
		return false;
	}
}

void lidar_final_exec::_getCoordNameList(const std::string& fold, const std::string& ext, std::vector<std::string>& list, bool complete) {
    Poco::Path fPath(fold);
	
	std::vector<std::string> folderContent;
	Poco::File(fPath).list(folderContent);

	std::vector<std::string>::iterator it = folderContent.begin();
	std::vector<std::string>::iterator end = folderContent.end();
	
	for (; it != end; it++) {
		Poco::Path p = Poco::Path(fPath).append(*it);
		if (Poco::toLower(p.getExtension()) == ext) {
			std::string name = p.getBaseName();
			if (complete) {
				list.push_back(name);
			} else {
				list.push_back(name.substr(name.size() - 8));
			}
		}
	}
}
std::string lidar_final_exec::build_gridname( const std::vector<std::string>& list, const std::string& nome ) {
	std::string Nome( nome );
	std::transform( Nome.begin(), Nome.end(), Nome.begin(), ::toupper );
	size_t k1 = Nome.find_last_of( '_' );
	if( k1 != std::string::npos )
		Nome = Nome.substr( k1 );

	for( size_t i = 0; i < list.size(); i++ ) {
		std::string val = list[i];
		size_t k = val.find_last_of( '_' );
		if( k != std::string::npos )
			val= val.substr( k );

		if( val == Nome )
			return val;
	}
	return "";
}

bool lidar_final_exec::_sortAndCompare(std::vector<std::string>& list1, std::vector<std::string>& list2, std::vector<std::string>& diff) {
    if ( list1.size() != list2.size() ) {
        Check_log << "Different size" << std::endl;
        return false;
    }
	for( size_t i = 0; i < list1.size(); i++ ) {
		std::transform( list1[i].begin(), list1[i].end(), list1[i].begin(), ::toupper );
		std::transform( list2[i].begin(), list2[i].end(), list2[i].begin(), ::toupper );
	}

	std::sort(list1.begin(), list1.end());
	std::sort(list2.begin(), list2.end());

    for ( size_t i = 0; i < list1.size(); i++) {
        std::string val1 = list1[i];
        size_t k1 = val1.find_last_of('_');
        if ( k1 != std::string::npos )
            val1 = val1.substr(k1);
        std::string val2 = list2[i];
        size_t k2 = val2.find_last_of('_');
        if ( k2 != std::string::npos )
            val2 = val2.substr(k2);
        if ( val1 != val2)
            diff.push_back(val1);
    }
    return diff.size() == 0;

//	if (list1 != list2) {

//		std::vector<std::string>::const_iterator it = list1.begin();
//		std::vector<std::string>::const_iterator end = list1.end();

//		for (; it != end; it++) {
//			std::string val = *it;
//			if (std::find_if(list2.begin(), list2.end(), [&val] (std::string in) -> bool {
//				bool ret = in.find(val) != std::string::npos;
//				return ret;
//			}) == list2.end()) {
//				diff.push_back(val);
//			};
//		}

//		return diff.size() == 0;
//	}

//	return true;
}

void lidar_final_exec::_getStrips(std::vector<Lidar::Strip::Ptr>& str) {
	try {
		CV::Util::Spatialite::Statement stm(cnn);
		stm.prepare("select FOLDER FROM RAW_STRIP_DATA");
		CV::Util::Spatialite::Recordset set = stm.recordset();
		if (set.eof()) {
            Check_log << "Nessun dato in RAW_STRIP_DATA" << std::endl;
			return;
		}
		if (!set.eof()) {
			_stripFolder = set["FOLDER"].toString();
		}
	} catch (const std::exception&) {
        Check_log << "Errore durante il reperimento delle strisciate, cartella non inserita" << std::endl;
	}

	try {
		CV::Util::Spatialite::Statement stm(cnn);
		stm.prepare("select Z_STRIP_CS, AsBinary(GEOM) as GEOM FROM Z_STRIPV");
		CV::Util::Spatialite::Recordset set = stm.recordset();
		if (set.eof()) {
            Check_log << "Nessun dato in Z_STRIPV" << std::endl;
			return;
		}
	
		while (!set.eof()) {
			Blob b = set["GEOM"].toBlob();

			Lidar::Strip::Ptr strip(new Lidar::Strip(b));
			strip->name(set["Z_STRIP_CS"].toString());

			str.push_back(strip);

			set.next();
		}
	} catch (const std::exception& ex) {
        Check_log << "Errore durante il reperimento delle strisciate, dati non inseriti" << std::endl;
	}
}

void lidar_final_exec::_checkBlock() {
	std::string table("FINAL_CARTO_DIFF");
	std::string rawGrid("Z_RAW_F");
	cnn.remove_layer(table);
	cnn.remove_layer(rawGrid);

    std::vector<std::string>::iterator it = _groundEllList.begin(); // _rawList.begin();
    std::vector<std::string>::iterator end = _groundEllList.end(); //_rawList.end();

	OGRGeomPtr rg_ = OGRGeometryFactory::createGeometry(wkbPolygon);
	OGRGeometry* pol = rg_;
	pol->setCoordinateDimension(2);
	
	std::stringstream sql;
	sql << "CREATE TABLE " << rawGrid << "( NAME TEXT )";
	cnn.execute_immediate(sql.str());

	// add the geom column
	sql.str("");
	sql << "SELECT AddGeometryColumn('" << rawGrid << "'," <<
		"'GEOM'," <<
		SRID(cnn)<< "," <<
		"'POLYGON'," <<
		"'XY')";
	cnn.execute_immediate(sql.str());

	cnn.begin_transaction();

	for (; it != end; it++) {
		std::string name = *it;
        Poco::Path pth(_groundEll, name);
        pth.setExtension("xyzi");
		DSM_Factory fac;
        File_Mask mask( 4, 1, 2, 3, -1, -1 );
        fac.SetMask( mask );
        if( !fac.Open( pth.toString(), false, false ) )
			continue;
		DSM* dsm = fac.GetDsm();
        Check_log << "Letti " << dsm->Npt() << " punti da " << name << std::endl;

		//int x = Poco::NumberParser::parse(name.substr(0, 4)) * 100;
		//int y = Poco::NumberParser::parse(name.substr(4, 4)) * 1000;

		OGRGeometry* g = OGRGeometryFactory::createGeometry(wkbLinearRing);
		OGRLinearRing* gp = reinterpret_cast<OGRLinearRing*>(g);
		gp->setCoordinateDimension(2);
		gp->addPoint(dsm->Xmin(), dsm->Ymin());
		gp->addPoint( dsm->Xmax(), dsm->Ymin() );
		gp->addPoint( dsm->Xmax(), dsm->Ymax() );
		gp->addPoint( dsm->Xmin(), dsm->Ymax() );
		gp->closeRings();

		OGRGeomPtr rg_tmp = OGRGeometryFactory::createGeometry(wkbPolygon);
		OGRGeometry* pol_tmp_ = rg_tmp;
		OGRPolygon* pol_tmp = reinterpret_cast<OGRPolygon*>(pol_tmp_);
		pol_tmp->setCoordinateDimension(2);
		pol_tmp->addRing(gp);

		std::stringstream sqlc;
		sqlc << "INSERT INTO " << rawGrid << " (NAME, GEOM) VALUES ('" << name << "', ST_GeomFromWKB(:geom, " << SRID(cnn) << ") )";
		CV::Util::Spatialite::Statement stm0(cnn);
		stm0.prepare(sqlc.str());
		stm0[1].fromBlob(rg_tmp);
		stm0.execute();

		rg_ = pol->Union(pol_tmp);
		pol = rg_;
	}

	cnn.commit_transaction();

	std::cout << "Layer:" << rawGrid << std::endl;

	try {
		CV::Util::Spatialite::Statement stm(cnn);
		stm.prepare("select AsBinary(GEOM) as GEOM FROM CARTO");

		CV::Util::Spatialite::Recordset set = stm.recordset();

		if (!set.eof()) {
			Lidar::Block block(set["GEOM"].toBlob());
			OGRGeomPtr dif = block.geom()->Difference(pol);
			if (!dif->IsEmpty()) {

				std::stringstream sql;
				sql << "CREATE TABLE " << table << "( ID TEXT )";

				cnn.execute_immediate(sql.str());

				// add the geom column
				std::stringstream sql1;
				sql1 << "SELECT AddGeometryColumn('" << table << "'," <<
					"'GEOM'," <<
					SRID(cnn)<< "," <<
					"'" << get_typestring(dif)  << "'," <<
					"'XY')";
				cnn.execute_immediate(sql1.str());

				std::stringstream sqlc;
				sqlc << "INSERT INTO " << table << " (geom) VALUES (ST_GeomFromWKB(:geom, " << SRID(cnn) << ") )";
				CV::Util::Spatialite::Statement stm0(cnn);
				stm0.prepare(sqlc.str());
				stm0[1].fromBlob(dif);
				stm0.execute();
			} else {
				_coversAll = true;
			}
		}
	} catch (const std::exception& e) {
        Check_log << "Errore durante il reperimento del blocco" << e.what() << std::endl;
	}
	
	std::cout << "Layer:" << table << std::endl;
}

void lidar_final_exec::_checkEquality() {
    if (!_sortAndCompare(_groundEllList, _overgroundEllList, overGroundEllDiff)) {
        Check_log << "Dati grezzi e dati overground ellisoidici non compatibili" << std::endl;
    } else {
        Check_log << "Dati grezzi e dati overground ellisoidici compatibili" << std::endl;
    }

    if (!_sortAndCompare(_groundEllList, _mdtList, mdtDiff)) {
        Check_log << "Dati grezzi ed mdt non compatibili" << std::endl;
    } else {
        Check_log << "Dati grezzi e mdt compatibili" << std::endl;
    }
	
    if (!_sortAndCompare(_groundEllList, _mdsList, mdsDiff)) {
        Check_log << "Dati grezzi ed mds non compatibili" << std::endl;
    } else {
        Check_log << "Dati grezzi e mds compatibili" << std::endl;
    }
	
    if (!_sortAndCompare(_groundEllList, _intensityList, intensityDiff)) {
        Check_log << "Dati grezzi ed intensity non compatibili" << std::endl;
    } else {
        Check_log << "Dati grezzi e intensity compatibili" << std::endl;
    }
	
//	if (!_sortAndCompare(_groundEllList, _groundOrtoList, groundOrtoDiff)) {
//        Check_log << "Dati grezzi e dati ground ortometrici non compatibili" << std::endl;
//	}
	

	
//	if (!_sortAndCompare(_rawList, _overgroundOrtoList, overGroundOrtoDiff)) {
//        Check_log << "Dati grezzi e dati overground ortometrici non compatibili" << std::endl;
//	}
}

// verifica se almeno un punto ricade nella strip
bool lidar_final_exec::_isStripUsed( std::map< std::string, std::vector<NODE> >& points, Lidar::Strip::Ptr sit ) {
	std::map< std::string, std::vector<NODE> >::iterator gIt = points.begin();
	std::map< std::string, std::vector<NODE> >::iterator gEnd = points.end();
	for( ; gIt != gEnd; gIt++ ) {
		std::vector<NODE>::iterator pIt = gIt->second.begin();
		std::vector<NODE>::iterator pEnd = gIt->second.end();
		for( ; pIt != pEnd; pIt++ ) {
			const NODE& n = *pIt;
			OGRPoint pt( n.x, n.y );

			bool contains = sit->geom()->Contains( &pt );
			if( contains )
				return true;
		}
	}
	return false;
}

void lidar_final_exec::_checkRawRandom( const std::string& raw, int pulse, std::map< std::string, std::vector<double> >& rawRandomDiff ) {

	std::vector<Lidar::Strip::Ptr> strips;
	_getStrips(strips);

	Poco::Path fPath(raw);
	
	std::vector<std::string> folderContent;
	Poco::File(fPath).list(folderContent);

    size_t sizef = folderContent.size();
    Geoin::Util::Sampler samplerf(sizef);

    int campione = 10; /*0.1 * sizef*/
    samplerf.sample(campione, sizef);
    Check_log << "Verifica di un campione di " << campione << " su " << sizef << std::endl;

	std::vector<std::string>::iterator it = folderContent.begin();
	std::vector<std::string>::iterator end = folderContent.end();

	std::map< std::string, std::vector<NODE> > points;
	
//	for (; it != end; it++) {

    size_t cnt = 1500;  // punti da verificare per ogni tile

    // seleziona alcune tile
    for (auto it = samplerf.begin(); it != samplerf.end(); it++) {
        auto index = *it;

        Poco::Path p = Poco::Path(fPath).append(folderContent[index]);
		File_Mask fm( 4, 1, 2, 3 );
		DSM_Factory f;
		f.SetMask( fm );
        if( Poco::toLower( p.getExtension() ) != "las" && Poco::toLower( p.getExtension() ) != "xyzi" ) {
            Check_log << "Estensione non supportata " << p.getExtension() << std::endl;
			continue;
        }

        Check_log << "Apertura di " << p.getBaseName() << std::endl;
		if ( !f.Open(p.toString(), false, false)) {
            Check_log << "Errore nell'apertura di " << p.getBaseName() << std::endl;
			continue;
		}

		DSM* dsm = f.GetDsm();
		size_t size = dsm->Npt();

		//size_t c = _getSamplesCount(1000, size/2, size);

//		cnt = _tilePP/100.0f*size;
//		if (cnt == 0) {
//			cnt = 1;
//		}
//		cnt = 1500;

		Geoin::Util::Sampler sampler(size);
		sampler.sample(cnt, size);
		for (auto it = sampler.begin(); it != sampler.end(); it++) {
			auto index = *it;
			NODE n = dsm->Node(index);
			points[p.getBaseName()].push_back(n);
		}
	}

    Check_log << " Verifica di " << cnt << " punti per tile" << std::endl;

	std::vector<Lidar::Strip::Ptr>::const_iterator sit = strips.begin();
	std::vector<Lidar::Strip::Ptr>::const_iterator send = strips.end();
	bool con = false;
	for (; sit != send; sit++) {
        if ( !_isStripUsed( points, *sit) ) {
            Check_log << "No points in " << (*sit)->name() << std::endl;
            continue;
        }

		Poco::Path path(_stripFolder);
		std::string actual = (*sit)->name() + ".las";
		path.append(actual);

        Check_log << " * " << actual << std::endl;
		
		DSM_Factory f;
        f.SetEcho(pulse);
        f.SetAngle( LID_ANG_SCAN );
        if ( !f.Open(path.toString(), false, false) ) {
            Check_log << "Error opening " << (*sit)->name() << std::endl;
            continue;
        }
		DSM* dsm = f.GetDsm();
		dsm->CreateIndex();
        Check_log << "Creato indice" << std::endl;

		std::map< std::string, std::vector<NODE> >::iterator gIt = points.begin();
		std::map< std::string, std::vector<NODE> >::iterator gEnd = points.end();

        // iterazioni sui tutti i punti presi dalle tile
		for (; gIt != gEnd; gIt++) {
			//std::vector<NODE>::iterator pIt = gIt->second.begin();
			//std::vector<NODE>::iterator pEnd = gIt->second.end();
			std::vector<NODE>& vn = gIt->second;

            std::set<size_t> ilist;
            statistics st(1);
            // iterazioni dei punti di una singola tile
			for( size_t i = 0; i < vn.size(); i++) {
				const NODE& n = vn[i]; // *pIt;
				OGRPoint pt(n.x, n.y);

				bool contains = (*sit)->geom()->Contains(&pt);
				if (contains) {
                    double diff = dsm->getPoint(n);
                    if ( diff > 0 && diff < 10. * LID_TOL_A ) {
                        rawRandomDiff[gIt->first].push_back(diff);
                        std::vector<double> dd;
                        dd.push_back(diff);
                        st.add_point(dd);
                        ilist.insert(i);
                    }
				}
			}
			std::vector<NODE> vn1;
            for ( size_t i = 0; i < vn.size(); i++ ) {
				if ( ilist.find(i) == ilist.end() )
					vn1.push_back(vn[i]);
            }
			size_t sz = vn.size();
			vn = vn1;

            if ( st.count() ) {
                Check_log << "Tile " << gIt->first << " trovati " << (long) st.count() << " punti su " << sz << std::endl;
                Check_log << "  Stdev " << st.st_devS() << " mean " << st.meanS() << " max val " << st.aMaxS() << std::endl;
                Check_log << "  Punti residui " << gIt->second.size() << std::endl;
            }

		}
	}
    std::map< std::string, std::vector<NODE> >::iterator gIt = points.begin();
    std::map< std::string, std::vector<NODE> >::iterator gEnd = points.end();

    // iterazioni sui tutti i punti presi dalle tile
    long count = 0;
    for (; gIt != gEnd; gIt++) {
        count += gIt->second.size();
    }
    Check_log << "Punti non accoppiati " << count << std::endl;
}

void lidar_final_exec::_checkEllipsoidicData() {
	_checkFolderWithRaw(_groundEll, _groundEllList, "Ground");
	_checkFolderWithRaw(_overgroundEll, _overgroundEllList, "Overground");
}

void lidar_final_exec::_checkFolderWithRaw(const std::string& folder, const std::vector<std::string>& data, const std::string& group) {
	std::map< std::string, std::vector<NODE> > points;

	size_t size = data.size();
	size_t cnt = _classFP/100.0f * size;
	if (cnt == 0) {
		cnt = 1;
	}
	Geoin::Util::Sampler sampler(size);
	sampler.sample(cnt, size);
	for (auto it = sampler.begin(); it != sampler.end(); it++) {
		size_t index = *it;
		std::string corner = data.at(index);

		std::string path = _fileFromCorner(folder, "xyzic", corner);
		DSM_Factory f;
		f.SetEcho(MyLas::single_pulse);
        //f.SetAngle(LID_ANG_SCAN / 2.);

		File_Mask mask(5, 1, 2, 3, 1, 1);
		f.SetMask(mask);
		bool ret = f.Open(path, false, false);
		
		unsigned int npt = f.GetDsm()->Npt();
		size_t cntp = _classPP/100.0f * npt;
		if (cntp == 0) {
			cntp = 1;
		}
		Geoin::Util::Sampler sampler(npt);
		sampler.sample(cntp, npt);
		
		for (auto itp = sampler.begin(); itp != sampler.end(); itp++) {
			const NODE& n = f.GetDsm()->Node(*itp);
			points[corner].push_back(n);
		}
	}

	std::map< std::string, std::vector<NODE> >::iterator gIt = points.begin();
	std::map< std::string, std::vector<NODE> >::iterator gEnd = points.end();
	for (; gIt != gEnd; gIt++) {		
		DSM_Factory s;
		std::string path = _fileFromCorner(_raw, "las", gIt->first);
		if (!s.Open(path)) {
            Check_log << "Impossibile aprire " << path << std::endl;
			continue;
		}
		
		DSM* dsm = s.GetDsm();

		std::vector<NODE>::iterator pIt = gIt->second.begin();
		std::vector<NODE>::iterator pEnd = gIt->second.end();

		PointCheck pc;
		pc.target = gIt->first;
		pc.group = group;
		
		unsigned int trIdx = -1;
		unsigned long match = 0;
		for (; pIt != pEnd; pIt++) {
			const NODE& n = *pIt;
			
			trIdx = dsm->FindTriangle(n.x, n.y, trIdx);
			const TRIANGLE& tri = dsm->Triangle(trIdx);

			for (unsigned int idx = 0; idx < 3; idx++) {
				const NODE& t = dsm->Node(tri.p[idx]);
				if (t.z == Z_NOVAL || t.z == Z_OUT) {
					continue;
				}
				if (abs(t.x - n.x) < 0.1 && abs(t.y - n.y) < 0.1) {
					double z = s.GetDsm()->GetQuota(n.x, n.y);
					if (z != Z_NOVAL && z != Z_OUT && abs(z - n.z) < 0.1) {
						pc.ok++;
					} else {
						pc.ko++;
					}
					break;
				}
			}
		}
		_pc.push_back(pc);
	}
}

void lidar_final_exec::_checkResamples(const std::string& folder1, const std::vector<std::string>& list1, const std::string& folder2, const std::vector<std::string>& list2, std::vector<Stats>& stats) { 

	size_t size = list1.size();
	size_t cnt = _resFP / 100.f * size;
	if (cnt == 0) {
		cnt = 1;
	}

    ortometric orto(_gridFile);

	Geoin::Util::Sampler sampler(size);
	sampler.sample(cnt, size);
	
	for (auto it = sampler.begin(); it != sampler.end(); it++) {
		std::string corner = list1.at(*it);

		std::string groundPath = _fileFromCorner(folder1, "xyzi", corner);
		DSM_Factory gr;
		File_Mask mask(4, 1, 2, 3);
		gr.SetMask(mask);
		if (!gr.Open(groundPath, false, true)) {
            Check_log << "Impossibile aprire " << groundPath << std::endl;
			continue;
		}

		std::string mdtnome = build_gridname( list2, corner );
		if( mdtnome.empty() ) {
			Check_log << "Non è stato trovato l'MDT corrispondente a  " << corner << std::endl;
			continue;
		}
		std::string mdtPath	= _fileFromCorner(folder2, "asc", mdtnome );

		DSM_Factory m;
		if (!m.Open(mdtPath, false, false)) {
            Check_log << "Impossibile aprire " << mdtPath << std::endl;
			continue;
		}
		std::vector<double> diff;
		unsigned int npt = m.GetDsm()->Npt();
		size_t cntp = _resPP/100.0f * npt;
		if (cntp == 0) {
			cntp = 1;
		}
		Geoin::Util::Sampler sampler(npt);
		sampler.sample(cntp, npt);
		
//		vGrid grid;
//        InitIGMIgrid( grid );
		for (auto itp = sampler.begin(); itp != sampler.end(); itp++) {
			const NODE& n = m.GetDsm()->Node(*itp);
			double z = gr.GetDsm()->GetQuota(n.x, n.y);

			if (z != Z_NOVAL && z != Z_OUT && n.z != Z_NOVAL && n.z != Z_OUT) {
                z = orto.ortoQ(n.x, n.y, z);
                if (z != Z_NOVAL )
//				double cr;
//				if( grid.GetCorrections( n.y, n.x, &cr ) ) {
//					z -= cr;
//				}
                    diff.push_back(z - n.z);
			}
		}

        Check_log << corner << " analizzati " << diff.size() << " punti" << std::endl;
		Stats s = { corner, 0, 0, diff.size()};
        GetStats(diff, s);
        Check_log << "   media " << s.mean << " st dev " << s.stdDev << std::endl;
		stats.push_back(s);
	}
}



void lidar_final_exec::_checkQuota(const std::string& folder1, const std::string& folder2, std::vector<Stats>& stats) { 
	std::vector<Poco::Path> files;

	vGrid::HGRID_TYPE type;
	Poco::Path g(_gridFile);
	if (Poco::toLower(g.getExtension()) == "gk1" || Poco::toLower(g.getExtension()) == "gk2") {
		type = vGrid::ty_GK;
		files.push_back(g);
	} else if (Poco::toLower(g.getExtension()) == "gr1" || Poco::toLower(g.getExtension()) == "gr2") {
		type = vGrid::ty_GR;
		files.push_back(g);
	} else {
		std::fstream in;
        in.open(_gridFile, std::ios_base::in);

		std::string line;
        while (_Getline(in, line)) {
			Poco::Path t(line);
            if (Poco::toLower(t.getExtension()) == "gk1" || Poco::toLower(t.getExtension()) == "gk2") {
				type = vGrid::ty_GK;
				files.push_back(t);
            } else if (Poco::toLower(t.getExtension()) == "gr1" || Poco::toLower(t.getExtension()) == "gr2") {
				type = vGrid::ty_GR;
				files.push_back(t);
			}
		}
	}
	
	vGrid grid;

	bool ret = true;
	for (std::vector<Poco::Path>::iterator it = files.begin(); it !=  files.end(); it++) {
		if (it == files.begin()) {
			ret |=  grid.Init(it->toString().c_str(), type);
		} else {
			ret |= grid.MergeGrid(it->toString().c_str());
		}
	}

	projPJ utm = pj_init_plus("+proj=utm +ellps=WGS84 +zone=32 +datum=WGS84 +units=m +no_defs");
	projPJ wgs84 = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");

	std::vector<std::string>::iterator it = _rawList.begin();
	std::vector<std::string>::iterator end = _rawList.end();
	for (; it != end; it++) {
		std::string ell = _fileFromCorner(folder1, "xyzic", *it);
		DSM_Factory ellF; 
		File_Mask mask(5, 1, 2, 3, 1, 1);
		ellF.SetMask(mask);
		bool ok = ellF.Open(ell, false, false);

		std::string orto = _fileFromCorner(folder2, "xyzic", *it);
		DSM_Factory ortoF; 
		File_Mask mask1(5, 1, 2, 3, 1, 1);
		ortoF.SetMask(mask1);
		ok = ortoF.Open(orto, false, false);

		size_t ortoNpt = ortoF.GetDsm()->Npt(), ellNpt = ellF.GetDsm()->Npt();
		if (ortoNpt != ellNpt) {
            Check_log << ell << " e " << orto << " differiscono per numero di punti" << std::endl; //REPORT
			continue;
		}
		std::vector<double> diffData;

		size_t npt = ortoF.GetDsm()->Npt(); 
		size_t cntp = _qPP/100.0f * npt;
		if (cntp == 0) {
			cntp = 1;
		}

		Geoin::Util::Sampler sampler(npt);
		sampler.sample(cntp, npt);
		
		for (auto itp = sampler.begin(); itp != sampler.end(); itp++) {
			auto index = *itp;
			const NODE& ellN = ellF.GetDsm()->Node(index);
			const NODE& ortoN = ortoF.GetDsm()->Node(index);

			double x = ellN.x, y = ellN.y, z = ellN.z;
			pj_transform(utm, wgs84, 1, 1, &x, &y, &z);
	
			double diff = 0;
			if (grid.GetCorrections(y, x, &diff)) {
				z -= diff;
				if (z != Z_NOVAL && z != Z_OUT && ortoN.z != Z_NOVAL && ortoN.z != Z_OUT) {
					diffData.push_back(ortoN.z - z); 
				}
			}
		}

		Stats st = { *it, 0, 0, diffData.size() };
		GetStats(diffData, st);

		stats.push_back(st);
	}

	pj_free(utm);
	pj_free(wgs84);
}

std::string lidar_final_exec::_fileFromCorner(const std::string& folder, const std::string& ext, const std::string& corner) {
	std::vector<std::string> folderContent;
	Poco::File(folder).list(folderContent);

	std::string& cnr = Poco::toLower( corner );

	std::vector<std::string>::iterator it = folderContent.begin();
	std::vector<std::string>::iterator end = folderContent.end();
	
	for (; it != end; it++) {
		Poco::Path p = Poco::Path(folder).append(*it);	
		std::string name = Poco::toLower( p.getBaseName());
		if (Poco::toLower(p.getExtension()) == Poco::toLower(ext) && name.find(cnr) != std::string::npos) {
			return p.toString();
		}
	}
	throw std::runtime_error("File from corner failed");
}

size_t lidar_final_exec::_getSamplesCount(size_t minVal, size_t maxVal, size_t size, double perc) {
	size_t count = size * perc;

    count = std::min(count, maxVal);
    count = std::max(count, minVal);
	return count < size ? count : size - 1;
}

void lidar_final_exec::Error(const std::string& operation, const std::exception& e) {
    Check_log << "Error [" << operation << "] : " << e.what() << std::endl;
}

void lidar_final_exec::Error(const std::string& operation) {
    Check_log << "Error [" << operation << "]" << std::endl;
}

void lidar_final_exec::GetStats(const std::vector<double>& diff, Stats& s) {
	double size = static_cast<double>(diff.size());
	double mean = std::accumulate(diff.begin(), diff.end(), 0.0) / size;

	std::vector<double>::const_iterator j = diff.begin();
	std::vector<double>::const_iterator end = diff.end();
	double mm = 0.0;
	for (; j != end; j++) {
		double val = *j;
		mm += std::pow(val - mean, 2);
	}

	s.mean = mean;
	s.stdDev = std::sqrt(mm / size);
}

void lidar_final_exec::_reportBlock() {
	Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Copertura aree da rilevare");
	if (_coversAll) {
		sec->add_item("para")->append("Tutte le zone richieste sono state rilevate");
	} else {
		sec->add_item("para")->append("Esistono zone richieste non rilevate");
	}
}
	
void lidar_final_exec::_reportEquality() {
	Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Completezza dati");

	bool missed = false;

	if (mdtDiff.size()) {
		sec->add_item("para")->append("Dati mdt non completi");
		Doc_Item itl = sec->add_item("itemizedlist");

		for (int i = 0; i < mdtDiff.size(); i++) {
			itl->add_item("listitem")->add_item("para")->append(mdtDiff.at(i));
		}

		missed |= true;
	}

	if (mdsDiff.size()) {
		sec->add_item("para")->append("Dati mds non completi");
		Doc_Item itl = sec->add_item("itemizedlist");

		for (int i = 0; i < mdsDiff.size(); i++) {
			itl->add_item("listitem")->add_item("para")->append(mdsDiff.at(i));
		}
	}

	if (intensityDiff.size()) {
		sec->add_item("para")->append("Dati intensita' non completi");

		Doc_Item itl = sec->add_item("itemizedlist");
		for (int i = 0; i < intensityDiff.size(); i++) {
			itl->add_item("listitem")->add_item("para")->append(intensityDiff.at(i));
		}
		missed |= true;
	}

	if (groundEllDiff.size()) {
		sec->add_item("para")->append("Dati ground ellissoidici non completi");

		Doc_Item itl = sec->add_item("itemizedlist");
		for (int i = 0; i < groundEllDiff.size(); i++) {
			itl->add_item("listitem")->add_item("para")->append(groundEllDiff.at(i));
		}
		missed |= true;
	}

	if (groundOrtoDiff.size()) {
		sec->add_item("para")->append("Dati ground ortometrici non completi");

		Doc_Item itl = sec->add_item("itemizedlist");
		for (int i = 0; i < groundEllDiff.size(); i++) {
			itl->add_item("listitem")->add_item("para")->append(groundOrtoDiff.at(i));
		}
		missed |= true;
	}

	if (overGroundEllDiff.size()) {
		sec->add_item("para")->append("Dati overground ellissoidici non completi");

		Doc_Item itl = sec->add_item("itemizedlist");
		for (int i = 0; i < overGroundEllDiff.size(); i++) {
			itl->add_item("listitem")->add_item("para")->append(overGroundEllDiff.at(i));
		}
		missed |= true;
	}


	if (overGroundOrtoDiff.size()) {
		sec->add_item("para")->append("Dati overground ortometrici non completi");

		Doc_Item itl = sec->add_item("itemizedlist");
		for (int i = 0; i < overGroundEllDiff.size(); i++) {
			itl->add_item("listitem")->add_item("para")->append(overGroundOrtoDiff.at(i));
		}
		missed |= true;
	}

	if (!missed) {
		sec->add_item("para")->append("I dati consegnati risultano completi");
	}
}
	
void lidar_final_exec::_reportRawRandom(std::map< std::string, std::vector<double> >& rawRandomDiff) {
	if (!rawRandomDiff.size()) {
		return;
	}

	Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Verifica tile grezze");
    sec->add_item("para")->append("Si confrontano punti presi a campione dalle tile grezze con le strisciate acquisite.");
    sec->add_item("para")->append("Sono riportate la media degli scarti in z e la loro standard deviation.");

	Doc_Item tab = sec->add_item("table");
    tab->add_item("title")->append("Statistiche tile");

    Poco::XML::AttributesImpl attr;
    attr.addAttribute("", "", "cols", "", "4");
    tab = tab->add_item("tgroup", attr);

    Doc_Item thead = tab->add_item("thead");
    Doc_Item row = thead->add_item("row");

	attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Foglio");
    row->add_item("entry", attr)->append("Numero punti");
    row->add_item("entry", attr)->append("Media");
    row->add_item("entry", attr)->append("Deviazione standard");
    Doc_Item tbody = tab->add_item("tbody");

    Poco::XML::AttributesImpl attrr;
    attrr.addAttribute("", "", "align", "", "right");

	for (std::map< std::string, std::vector<double> >::iterator it = rawRandomDiff.begin(); it != rawRandomDiff.end(); it++) {
		Stats s = { it->first, 0, 0 , it->second.size()};
		GetStats(it->second, s);

		row = tbody->add_item("row");
		row->add_item("entry", attr)->append(s.target);
		row->add_item("entry", attr)->append((int)s.count);
		row->add_item("entry", attr)->append(s.mean);
		row->add_item("entry", attr)->append(s.stdDev);
	}
}
	
void lidar_final_exec::_reportResamples() {
	Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Verifica ricampionamento");
    sec->add_item("para")->append("Si confrontano le quote dei dati MDT con le quote ground ortometriche interpolate e le quote dei dati MDS con le quote overground ortometriche interpolate.");
    sec->add_item("para")->append("Sono riportate la media degli scarti in z e la loro standard deviation.");

	Doc_Item tab = sec->add_item("table");
    tab->add_item("title")->append("Statistiche ricampionamento MDT");

    Poco::XML::AttributesImpl attr;
    attr.addAttribute("", "", "cols", "", "4");
    tab = tab->add_item("tgroup", attr);

    Doc_Item thead = tab->add_item("thead");
    Doc_Item row = thead->add_item("row");

    attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Foglio");
    row->add_item("entry", attr)->append("Numero punti");
    row->add_item("entry", attr)->append("Media");
    row->add_item("entry", attr)->append("Deviazione standard");
    Doc_Item tbody = tab->add_item("tbody");

    Poco::XML::AttributesImpl attrr;
    attrr.addAttribute("", "", "align", "", "right");

	for (std::vector< Stats >::iterator it = diffMdt.begin(); it != diffMdt.end(); it++) {
		row = tbody->add_item("row");
		row->add_item("entry", attr)->append(it->target);
		row->add_item("entry", attr)->append((int)it->count);
		row->add_item("entry", attr)->append(it->mean);
		row->add_item("entry", attr)->append(it->stdDev);
	}

	tab = sec->add_item("table");
    tab->add_item("title")->append("Statistiche ricampionamento MDS");
	
	attr.clear();
    attr.addAttribute("", "", "cols", "", "4");
    tab = tab->add_item("tgroup", attr);

    thead = tab->add_item("thead");
    row = thead->add_item("row");

    attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Foglio");
    row->add_item("entry", attr)->append("Numero punti");
    row->add_item("entry", attr)->append("Media");
    row->add_item("entry", attr)->append("Deviazione standard");
    tbody = tab->add_item("tbody");

	attrr.clear();
    attrr.addAttribute("", "", "align", "", "right");

	for (std::vector< Stats >::iterator it = diffMds.begin(); it != diffMds.end(); it++) {
		row = tbody->add_item("row");
		row->add_item("entry", attr)->append(it->target);
		row->add_item("entry", attr)->append((int)it->count);
		row->add_item("entry", attr)->append(it->mean);
		row->add_item("entry", attr)->append(it->stdDev);
	}
}
	
void lidar_final_exec::_reportQuota() {
	Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Verifica conversione quote");
    sec->add_item("para")->append("Si confrontano le quote dei dati ortometrici con quelle che si ricavano convertendo le quote ellissoidiche tramite i grigliati IGMI.");
    sec->add_item("para")->append("Sono riportate la media degli scarti in z e la loro standard deviation.");

	Doc_Item tab = sec->add_item("table");
    tab->add_item("title")->append("Statistiche quote ground");

    Poco::XML::AttributesImpl attr;
    attr.addAttribute("", "", "cols", "", "4");
    tab = tab->add_item("tgroup", attr);

    Doc_Item thead = tab->add_item("thead");
    Doc_Item row = thead->add_item("row");

    attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Foglio");
    row->add_item("entry", attr)->append("Numero punti");
    row->add_item("entry", attr)->append("Media");
    row->add_item("entry", attr)->append("Deviazione standard");
    Doc_Item tbody = tab->add_item("tbody");

    Poco::XML::AttributesImpl attrr;
    attrr.addAttribute("", "", "align", "", "right");

	for (std::vector< Stats >::iterator it = statsGround.begin(); it != statsGround.end(); it++) {
		row = tbody->add_item("row");
		row->add_item("entry", attr)->append(it->target);
		row->add_item("entry", attr)->append((int)it->count);
		row->add_item("entry", attr)->append(it->mean);
		row->add_item("entry", attr)->append(it->stdDev);
	}

	tab = sec->add_item("table");
    tab->add_item("title")->append("Statistiche quote overground");
	
	attr.clear();
    attr.addAttribute("", "", "cols", "", "4");
    tab = tab->add_item("tgroup", attr);

    thead = tab->add_item("thead");
    row = thead->add_item("row");

    attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Foglio");
    row->add_item("entry", attr)->append("Numero punti");
    row->add_item("entry", attr)->append("Media");
    row->add_item("entry", attr)->append("Deviazione standard");
    tbody = tab->add_item("tbody");

	attrr.clear();
    attrr.addAttribute("", "", "align", "", "right");

	for (std::vector< Stats >::iterator it = statsOverGround.begin(); it != statsOverGround.end(); it++) {
		row = tbody->add_item("row");
		row->add_item("entry", attr)->append(it->target);
		row->add_item("entry", attr)->append((int)it->count);
		row->add_item("entry", attr)->append(it->mean);
		row->add_item("entry", attr)->append(it->stdDev);
	}
}
	
void lidar_final_exec::_reportEllipsoidic() {
	Doc_Item sec = _article->add_item("section");
    sec->add_item("title")->append("Verifica classificazione");
    sec->add_item("para")->append("Si confrontano i dati presenti in ground e overground con le tile grezze.");
    sec->add_item("para")->append("Sono riportati il numero di punti trovati nelle tile grezze ed il numero dei punti fuori tolleranza.");

	Doc_Item tab = sec->add_item("table");
    tab->add_item("title")->append("Statistiche quote");

    Poco::XML::AttributesImpl attr;
    attr.addAttribute("", "", "cols", "", "4");
    tab = tab->add_item("tgroup", attr);

    Doc_Item thead = tab->add_item("thead");
    Doc_Item row = thead->add_item("row");

    attr.clear();
    attr.addAttribute("", "", "align", "", "center");
    row->add_item("entry", attr)->append("Foglio");
    row->add_item("entry", attr)->append("Origine");
    row->add_item("entry", attr)->append("OK");
    row->add_item("entry", attr)->append("KO");
    Doc_Item tbody = tab->add_item("tbody");

    Poco::XML::AttributesImpl attrr;
    attrr.addAttribute("", "", "align", "", "right");

	for (std::vector< PointCheck >::iterator it = _pc.begin(); it != _pc.end(); it++) {
		row = tbody->add_item("row");
		row->add_item("entry", attr)->append(it->target);
		row->add_item("entry", attr)->append(Poco::Path(it->group).getBaseName());

		std::stringstream str; str << it->ok;
		row->add_item("entry", attr)->append(str.str());
		str.str(""); str << it->ko;
		row->add_item("entry", attr)->append(str.str());
	}
}

