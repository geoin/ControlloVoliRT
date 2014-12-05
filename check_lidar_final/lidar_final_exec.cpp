#include "lidar_final_exec.h"

#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/String.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/AutoPtr.h"
#include "Poco/NumberParser.h"

#include "CVUtil/ogrgeomptr.h"

#include <assert.h>
#include <algorithm>
#include <random>

#include <time.h>

#define GEO_DB_NAME "geo.sqlite"

using namespace CV;
using namespace CV::Util::Geometry;


lidar_final_exec::lidar_final_exec() : _total(0), _missed(0) {
	srand (time(NULL));
}

void lidar_final_exec::set_proj_dir(const std::string& proj) {
    _proj_dir = proj;
}

bool lidar_final_exec::run() {
    //_checkBlock();
	//_checkEquality();
	//_checkRawRandom();
	//_checkEllipsoidicData();
	_checkResamples();
    return true;
}

bool lidar_final_exec::createReport() {
    std::cout << "Report" << std::endl;
	return true;
}

void lidar_final_exec::readFolders() {
	_step = 0;

	_raw = _getRawFolder("FINAL_RAW_STRIP_DATA", _step);
	_getCoordNameList(_raw, "las", _rawList);
	
	_groundEll = _getFolder("FINAL_GROUND_ELL");
	_getCoordNameList(_groundEll, "xyzic", _groundEllList);

	_overgroundEll = _getFolder("FINAL_OVERGROUND_ELL");
	_getCoordNameList(_overgroundEll, "xyzic", _overgroundEllList);

	_groundOrto = _getFolder("FINAL_GROUND_ORTO");
	_getCoordNameList(_groundOrto, "xyzic", _groundOrtoList);

	_overgroundOrto = _getFolder("FINAL_OVERGROUND_ORTO");
	_getCoordNameList(_overgroundOrto, "xyzic", _overgroundOrtoList);
	
	_mds = _getFolder("FINAL_MDS");
	_getCoordNameList(_mds, "asc", _mdsList);

	_mdt = _getFolder("FINAL_MDT");
	_getCoordNameList(_mdt, "asc", _mdtList);

	_intensity = _getFolder("FINAL_INTENSITY");
	_getCoordNameList(_intensity, "tif", _intensityList);
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

void lidar_final_exec::_getCoordNameList(const std::string& fold, const std::string& ext, std::vector<std::string>& list) {
    Poco::Path fPath(fold);
	
	std::vector<std::string> folderContent;
	Poco::File(fPath).list(folderContent);

	std::vector<std::string>::iterator it = folderContent.begin();
	std::vector<std::string>::iterator end = folderContent.end();
	
	for (; it != end; it++) {
		Poco::Path p = Poco::Path(fPath).append(*it);
		if (Poco::toLower(p.getExtension()) == ext) {
			std::string name = p.getBaseName();
			list.push_back(name.substr(name.size() - 8));
		}
	}
}

bool lidar_final_exec::_sortAndCompare(std::vector<std::string>& list1, std::vector<std::string>& list2) {
	std::vector<std::string>::const_iterator it = list1.begin();
	std::vector<std::string>::const_iterator end = list1.end();
	std::sort(list1.begin(), list1.end());
	std::sort(list2.begin(), list2.end());
	return list1 == list2;
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
		stm.prepare("select NAME, AsBinary(GEOM) as GEOM FROM Z_STRIPV");
		CV::Util::Spatialite::Recordset set = stm.recordset();
		if (set.eof()) {
			std::cout << "Nessun dato in Z_STRIPV" << std::endl;
			return;
		}
	
		while (!set.eof()) {
			Blob b = set["GEOM"].toBlob();

			Lidar::Strip::Ptr strip(new Lidar::Strip(b));
			strip->name(set["NAME"].toString());

			str.push_back(strip);
		}
	} catch (const std::exception& ex) {
		std::cout << "Errore durante il reperimento delle strisciate, dati in inseriti" << std::endl;
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
	
	for (; it != end; it++) {
		Poco::Path p = Poco::Path(fPath).append(*it);
		DSM_Factory f;
		if (Poco::toLower(p.getExtension()) != "las" || !f.Open(p.toString(), false, false)) {
			continue;
		}

		DSM* dsm = f.GetDsm();
		for (int i = 0; i < 100; i++, _total++) {
			const NODE& n = dsm->Node(rand() % (dsm->Npt() - 1));

			std::vector<Lidar::Strip::Ptr>::const_iterator sit = strips.begin();
			std::vector<Lidar::Strip::Ptr>::const_iterator send = strips.end();
			bool con = false;
			for (; sit != send; sit++) {
				OGRPoint pt(n.x, n.y);
				con |= (*sit)->geom()->Contains(&pt);
			}
			if (!con) {
				_missed++;
			}
		}
	}
}

void lidar_final_exec::_checkEllipsoidicData() {
	unsigned long m = _checkFolderWithRaw(_groundEll, _groundEllList);
	m += _checkFolderWithRaw(_overgroundEll, _overgroundEllList);
}

unsigned long lidar_final_exec::_checkFolderWithRaw(const std::string& folder, const std::vector<std::string>& data) {
	unsigned long mismatch = 0;
	for (int i = 0; i < 2; i++) {
		const std::string& corner = data.at(rand() % (data.size() - 1));
		std::string path = _fileFromCorner(folder, "xyzic", corner);
		DSM_Factory f;
		f.SetEcho(MyLas::single_pulse);

		File_Mask mask(5, 1, 2, 3, 1, 1);
		f.SetMask(mask);
		bool ret = f.Open(path, false, false);
		
		DSM_Factory s;
		s.Open(_fileFromCorner(_raw, "las", corner));

		for (int j = 0; j < 10; j++) {
			const NODE& n = f.GetDsm()->Node(rand() % (f.GetDsm()->Npt() - 1));
			double z = s.GetDsm()->GetQuota(n.x, n.y);
			if (abs(z - n.z) > 0.1) {
				mismatch++;
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
		gr.Open(groundPath, false, true);

		std::string mdtPath = _fileFromCorner(_mdt, "asc", corner);
		DSM_Factory m;
		m.Open(mdtPath, false, true);

		
		for (int j = 0; j < 10; j++) {
			const NODE& n = gr.GetDsm()->Node(rand() % (gr.GetDsm()->Npt() - 1));
			double z = m.GetDsm()->GetQuota(n.x, n.y);
			if (abs(z - n.z) > 0.1) {
				mismatch++;
			}
		}
	}
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

