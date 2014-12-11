#include "lidar_final_exec.h"

#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/String.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/AutoPtr.h"
#include "Poco/NumberParser.h"

#include "proj_api.h"
#include "PJ_igmi.h"

#include "CVUtil/ogrgeomptr.h"

#include <assert.h>
#include <algorithm>
#include <random>

#include <algorithm>
#include <time.h>

#define GEO_DB_NAME "geo.sqlite"

using namespace CV;
using namespace CV::Util::Geometry;


lidar_final_exec::lidar_final_exec() {
	srand (time(NULL));
}

void lidar_final_exec::set_proj_dir(const std::string& proj) {
    _proj_dir = proj;
}

bool lidar_final_exec::run() {
    _checkBlock();
	_checkEquality();

	/*
	_checkRawRandom();
	_checkResamples();
	*/
	_checkQuota();
	
	/*
	_checkEllipsoidicData();
	*/

    
	return true;
}

bool lidar_final_exec::createReport() {
    std::cout << "Report" << std::endl;
	return true;
}

void lidar_final_exec::readFolders() {
	_step = 0;

	try {
		_raw = _getRawFolder("FINAL_RAW_STRIP_DATA", _step);
		_getCoordNameList(_raw, "las", _rawList);
	} catch (const std::exception& ex) { 
		std::cout << "Dati grezzi non inseriti" << std::endl;
	}
	
	_groundEll = _getFolder("FINAL_GROUND_ELL");
	_getCoordNameList(_groundEll, "xyzic", _groundEllList);

	_overgroundEll = _getFolder("FINAL_OVERGROUND_ELL");
	_getCoordNameList(_overgroundEll, "xyzic", _overgroundEllList);

	_groundOrto = _getFolder("FINAL_GROUND_ORTO");
	_getCoordNameList(_groundOrto, "xyzic", _groundOrtoList);

	_overgroundOrto = _getFolder("FINAL_OVERGROUND_ORTO");
	_getCoordNameList(_overgroundOrto, "xyzic", _overgroundOrtoList);
	
	_mds = _getFolder("FINAL_MDS");
	_getCoordNameList(_mds, "asc", _mdsList, true);

	_mdt = _getFolder("FINAL_MDT");
	_getCoordNameList(_mdt, "asc", _mdtList, true);

	_intensity = _getFolder("FINAL_INTENSITY");
	_getCoordNameList(_intensity, "tif", _intensityList, true);
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
	stm.prepare("SELECT FOLDER, TILE_SIZE, GRID FROM " + table);
	CV::Util::Spatialite::Recordset set = stm.recordset();
	if (set.eof()) {
		throw std::runtime_error("No data in " + table);
	}
	step = set["TILE_SIZE"].toInt();
	_gridFile = set["GRID"].toString();
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



bool lidar_final_exec::_sortAndCompare(std::vector<std::string>& list1, std::vector<std::string>& list2) {
	std::sort(list1.begin(), list1.end());
	std::sort(list2.begin(), list2.end());
	if (list1 == list2) {
		return true;
	}
	
	std::vector<std::string>::const_iterator it = list1.begin();
	std::vector<std::string>::const_iterator end = list1.end();

	for (; it != end; it++) {
		std::string val = *it;
		if (std::find_if(list2.begin(), list2.end(), [&val] (std::string in) -> bool {
			bool ret = in.find(val) != std::string::npos;
			return ret;
		}) == list2.end()) {
			return false;
		};
	}

	return true;
}

void lidar_final_exec::_getStrips(std::vector<Lidar::Strip::Ptr>& str) {
	try {
		CV::Util::Spatialite::Statement stm(cnn);
		stm.prepare("select FOLDER FROM RAW_STRIP_DATA");
		CV::Util::Spatialite::Recordset set = stm.recordset();
		if (set.eof()) {
			std::cout << "Nessun dato in RAW_STRIP_DATA" << std::endl;
			return;
		}
		if (!set.eof()) {
			_stripFolder = set["FOLDER"].toString();
		}
	} catch (const std::exception&) {
		std::cout << "Errore durante il reperimento delle strisciate, cartella non inserita" << std::endl;
	}

	try {
		CV::Util::Spatialite::Statement stm(cnn);
		stm.prepare("select Z_STRIP_CS, AsBinary(GEOM) as GEOM FROM Z_STRIPV");
		CV::Util::Spatialite::Recordset set = stm.recordset();
		if (set.eof()) {
			std::cout << "Nessun dato in Z_STRIPV" << std::endl;
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
		std::cout << "Errore durante il reperimento delle strisciate, dati non inseriti" << std::endl;
	}
}

void lidar_final_exec::_checkBlock() {
	std::vector<std::string>::iterator it = _rawList.begin();
	std::vector<std::string>::iterator end = _rawList.end();

	OGRGeomPtr rg_ = OGRGeometryFactory::createGeometry(wkbPolygon);
	OGRGeometry* pol = rg_;
	pol->setCoordinateDimension(2);

	for (; it != end; it++) {
		std::string name = *it;
		if (name.size() == 8) {
			int x = Poco::NumberParser::parse(name.substr(0, 4)) * 100;
			int y = Poco::NumberParser::parse(name.substr(4, 4)) * 1000;

			OGRGeometry* g = OGRGeometryFactory::createGeometry(wkbLinearRing);
			OGRLinearRing* gp = reinterpret_cast<OGRLinearRing*>(g);
			gp->setCoordinateDimension(2);
			gp->addPoint(x, y);
			gp->addPoint(x + _step, y);
			gp->addPoint(x + _step, y + _step);
			gp->addPoint(x, y + _step);
			gp->closeRings();

			OGRGeomPtr rg_tmp = OGRGeometryFactory::createGeometry(wkbPolygon);
			OGRGeometry* pol_tmp_ = rg_tmp;
			OGRPolygon* pol_tmp = reinterpret_cast<OGRPolygon*>(pol_tmp_);
			pol_tmp->setCoordinateDimension(2);
			pol_tmp->addRing(gp);

			pol = pol->Union(pol_tmp);
		}
	}
	
	try {
		CV::Util::Spatialite::Statement stm(cnn);
		stm.prepare("select AsBinary(GEOM) as GEOM FROM CARTO");

		CV::Util::Spatialite::Recordset set = stm.recordset();

		if (!set.eof()) {
			Lidar::Block block(set["GEOM"].toBlob());
			OGRGeomPtr dif = block.geom()->Difference(pol);
			if (dif->IsEmpty()) {

			}
		}
	} catch (const std::exception&) {
		std::cout << "Errore durante il reperimento del blocco" << std::endl;
	}

}

void lidar_final_exec::_checkEquality() {
	if (!_sortAndCompare(_rawList, _mdtList)) {
		std::cout << "Dati grezzi ed mdt non compatibili" << std::endl;
	}

	if (!_sortAndCompare(_rawList, _mdsList)) {
		std::cout << "Dati grezzi ed mds non compatibili" << std::endl;
	} 
	
	if (!_sortAndCompare(_rawList, _intensityList)) {
		std::cout << "Dati grezzi ed intensity non compatibili" << std::endl;
	}

	if (!_sortAndCompare(_rawList, _groundEllList)) {
		std::cout << "Dati grezzi e dati ground ellisoidici non compatibili" << std::endl;
	}

	if (!_sortAndCompare(_rawList, _groundOrtoList)) {
		std::cout << "Dati grezzi e dati ground ortometrici non compatibili" << std::endl;
	}

	if (!_sortAndCompare(_rawList, _overgroundEllList)) {
		std::cout << "Dati grezzi e dati overground ellisoidici non compatibili" << std::endl;
	}

	if (!_sortAndCompare(_rawList, _overgroundOrtoList)) {
		std::cout << "Dati grezzi e dati overground ortometrici non compatibili" << std::endl;
	}
}

void lidar_final_exec::_checkRawRandom() {

	std::vector<Lidar::Strip::Ptr> strips;
	_getStrips(strips);

	Poco::Path fPath(_raw);
	
	std::vector<std::string> folderContent;
	Poco::File(fPath).list(folderContent);

	std::vector<std::string>::iterator it = folderContent.begin();
	std::vector<std::string>::iterator end = folderContent.end();

	std::map< std::string, std::vector<NODE> > points;
	
	for (; it != end; it++) {
		Poco::Path p = Poco::Path(fPath).append(*it);
		DSM_Factory f;
		if (Poco::toLower(p.getExtension()) != "las" || !f.Open(p.toString(), false, false)) {
			continue;
		}

		DSM* dsm = f.GetDsm();
		for (int i = 0; i < 100; i++) {
			NODE n = dsm->Node(rand() % (dsm->Npt() - 1));
			points[p.getBaseName()].push_back(n);
		}
	}

	std::vector<Lidar::Strip::Ptr>::const_iterator sit = strips.begin();
	std::vector<Lidar::Strip::Ptr>::const_iterator send = strips.end();
	bool con = false;
	for (; sit != send; sit++) {
		Poco::Path path(_stripFolder);
		path.append((*sit)->name() + ".las");
		
		DSM_Factory f;
		f.Open(path.toString());

		std::map< std::string, std::vector<NODE> >::iterator gIt = points.begin();
		std::map< std::string, std::vector<NODE> >::iterator gEnd = points.end();
		for (; gIt != gEnd; gIt++) {
			std::vector<NODE>::iterator pIt = gIt->second.begin();
			std::vector<NODE>::iterator pEnd = gIt->second.end();
			for (; pIt != pEnd; pIt++) {
				const NODE& n = *pIt;
				OGRPoint pt(n.x, n.y);

				bool contains = (*sit)->geom()->Contains(&pt);
				if (contains) {
					double diff = f.GetDsm()->GetQuota(n.x, n.y);
				}
			}
		}
	}
}

void lidar_final_exec::_checkEllipsoidicData() {
	unsigned long m = _checkFolderWithRaw(_groundEll, _groundEllList);
	m += _checkFolderWithRaw(_overgroundEll, _overgroundEllList);
}

unsigned long lidar_final_exec::_checkFolderWithRaw(const std::string& folder, const std::vector<std::string>& data) {
	std::map< std::string, std::vector<NODE> > points;

	unsigned long mismatch = 0;
	for (int i = 0; i < 2; i++) {
		const std::string& corner = data.at(rand() % (data.size() - 1));
		std::string path = _fileFromCorner(folder, "xyzic", corner);
		DSM_Factory f;
		f.SetEcho(MyLas::single_pulse);

		File_Mask mask(5, 1, 2, 3, 1, 1);
		f.SetMask(mask);
		bool ret = f.Open(path, false, false);
		
		for (int j = 0; j < 10; j++) {
			const NODE& n = f.GetDsm()->Node(rand() % (f.GetDsm()->Npt() - 1));
			points[corner].push_back(n);
		}
	}

	std::map< std::string, std::vector<NODE> >::iterator gIt = points.begin();
	std::map< std::string, std::vector<NODE> >::iterator gEnd = points.end();
	for (; gIt != gEnd; gIt++) {		
		DSM_Factory s;
		std::string path = _fileFromCorner(_raw, "las", gIt->first);
		if (!s.Open(path)) {
			std::cout << "Impossibile aprire " << path << std::endl;
			continue;
		}
		
		DSM* dsm = s.GetDsm();

		std::vector<NODE>::iterator pIt = gIt->second.begin();
		std::vector<NODE>::iterator pEnd = gIt->second.end();
		for (; pIt != pEnd; pIt++) {
			const NODE& n = *pIt;

			for (unsigned int idx = 0; idx < dsm->Npt(); idx++) {
				const NODE& t = dsm->Node(idx);
				if (abs(t.x - n.x) < 0.00001 && abs(t.y - n.y) < 0.00001){
					double z = s.GetDsm()->GetQuota(n.x, n.y);
					if (abs(z - n.z) > 0.1) {
						mismatch++;
					}
				}
			}

			
		}
	}
	return mismatch;
}

void lidar_final_exec::_checkResamples() {
	unsigned long mismatch = 0;
	for (int i = 0; i < 2; i++) {
		const std::string& corner = _groundOrtoList.at(rand() % (_groundOrtoList.size() - 1));
		std::string groundPath = _fileFromCorner(_groundOrto, "xyzic", corner);
		DSM_Factory gr;
		File_Mask mask(5, 1, 2, 3, 1, 1);
		gr.SetMask(mask);
		if (!gr.Open(groundPath, false, true)) {
			std::cout << "Impossibile aprire " << groundPath << std::endl;
			continue;
		}

		std::string mdtPath = _fileFromCorner(_mdt, "asc", corner);
		DSM_Factory m;
		if (!m.Open(mdtPath, false, true)) {
			std::cout << "Impossibile aprire " << mdtPath << std::endl;
			continue;
		}

		for (int j = 0; j < 10; j++) {
			unsigned int size = gr.GetDsm()->Npt();
			unsigned int randomIdx = rand() % (size - 1);
			assert(randomIdx < size);

			const NODE& n = gr.GetDsm()->Node(randomIdx);
			double z = m.GetDsm()->GetQuota(n.x, n.y);
			if (abs(z - n.z) > 0.1) {
				mismatch++;
			}
		}
	}
}

void lidar_final_exec::_checkQuota() {
	std::vector<Poco::Path> files;
	_gridFile = "G:/ControlloVoli/Grigliati/25.txt";

	vGrid::HGRID_TYPE type;
	Poco::Path g(_gridFile);
	if (Poco::toLower(g.getExtension()) == "gk2") {
		type = vGrid::ty_GK;
		files.push_back(g);
	} else if (Poco::toLower(g.getExtension()) == "gr1") {
		type = vGrid::ty_GR;
		files.push_back(g);
	} else {
		std::fstream in;
		in.open(_gridFile, std::ios_base::in);

		std::string line;
		while (std::getline(in, line)) {
			Poco::Path t(line);
			if (Poco::toLower(t.getExtension()) == "gk2") {
				type = vGrid::ty_GK;
				files.push_back(t);
			} else if (Poco::toLower(t.getExtension()) == "gr1") {
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

	std::vector<std::string>::iterator it = _groundEllList.begin();
	std::vector<std::string>::iterator end = _groundEllList.end();
	for (; it != end; it++) {
		std::string ell = _fileFromCorner(_groundEll, "xyzic", *it);
		DSM_Factory ellF; 
		File_Mask mask(5, 1, 2, 3, 1, 1);
		ellF.SetMask(mask);
		bool ok = ellF.Open(ell, false, false);

		std::string orto = _fileFromCorner(_groundOrto, "xyzic", *it);
		DSM_Factory ortoF; 
		File_Mask mask1(5, 1, 2, 3, 1, 1);
		ortoF.SetMask(mask1);
		ok = ortoF.Open(orto, false, false);

		size_t ortoNpt = ortoF.GetDsm()->Npt(), ellNpt = ellF.GetDsm()->Npt();
		if (ortoNpt != ellNpt) {
			std::cout << ell << " e " << orto << " differiscono per numero di punti" << std::endl;
			continue;
		}

		size_t s = min(ortoF.GetDsm()->Npt(), ellF.GetDsm()->Npt());

		for (int i = 0; i < 100; i++) {
			unsigned int index = rand() % (s - 1);
			const NODE& ellN = ellF.GetDsm()->Node(index);
			const NODE& ortoN = ortoF.GetDsm()->Node(index);

			double x = ellN.x, y = ellN.y, z = ellN.z;
			pj_transform(utm, wgs84, 1, 1, &x, &y, &z);
	
			double diff = 0;
			if (grid.GetCorrections(y, x, &diff)) {
				z -= diff;
				assert(std::abs(ortoN.z - z) < 0.1);
			}
		}
	}

	pj_free(utm);
	pj_free(wgs84);
}

std::string lidar_final_exec::_fileFromCorner(const std::string& folder, const std::string& ext, const std::string& corner) {
	std::vector<std::string> folderContent;
	Poco::File(folder).list(folderContent);

	std::vector<std::string>::iterator it = folderContent.begin();
	std::vector<std::string>::iterator end = folderContent.end();
	
	for (; it != end; it++) {
		Poco::Path p = Poco::Path(folder).append(*it);	
		std::string name = p.getBaseName();
		if (Poco::toLower(p.getExtension()) == Poco::toLower(ext) && name.find(corner) != std::string::npos) {
			return p.toString();
		}
	}
	throw std::runtime_error("File from corner failed");
}

void lidar_final_exec::Error(const std::string& operation, const std::exception& e) {
	std::cout << "Error [" << operation << "] : " << e.what() << std::endl;
}

void lidar_final_exec::Error(const std::string& operation) {
	std::cout << "Error [" << operation << "]" << std::endl;
}

