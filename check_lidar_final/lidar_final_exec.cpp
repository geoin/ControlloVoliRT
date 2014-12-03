#include "lidar_final_exec.h"

#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/AutoPtr.h"
#include "Poco/NumberParser.h"

#include "CVUtil/ogrgeomptr.h"

#include <assert.h>
#include <algorithm>

#define GEO_DB_NAME "geo.sqlite"

using namespace CV;
using namespace CV::Util::Geometry;

void lidar_final_exec::set_proj_dir(const std::string& proj) {
    _proj_dir = proj;
}

bool lidar_final_exec::run() {
    _checkBlock();
	_checkEquality();
    return true;
}

bool lidar_final_exec::createReport() {
    std::cout << "Report" << std::endl;
	return true;
}

void lidar_final_exec::readFolders() {
	_step = 0;
	_raw = _getRawFolder("FINAL_RAW_STRIP_DATA", _step);

	/*_mds = _getFolder("FINAL_MDS");
	_mdt = _getFolder("FINAL_MDT");
	_intensity = _getFolder("FINAL_INTENSITY");
	_groundEll = _getFolder("FINAL_GROUND_ELL");
	_groundOrto = _getFolder("FINAL_GROUND_ORTO");
	_overgroundEll = _getFolder("FINAL_OVERGROUND_ELL");
	_overgroundOrto = _getFolder("FINAL_OVERGROUND_ORTO");*/
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

void lidar_final_exec::_getCoordNameList(const std::string& fold, std::vector<std::string>& list) {
    Poco::Path fPath(fold);
		
	std::vector<std::string> folderContent;
	Poco::File(fPath).list(folderContent);

	std::vector<std::string>::iterator it = folderContent.begin();
	std::vector<std::string>::iterator end = folderContent.end();
	
	for (; it != end; it++) {
		Poco::Path p = Poco::Path(fPath).append(*it);
		std::string name = p.getBaseName();
		list.push_back(name.substr(name.size() - 8));
	}
}

bool lidar_final_exec::_sortAndCompare(std::vector<std::string>& list1, std::vector<std::string>& list2) {
	std::vector<std::string>::const_iterator it = list1.begin();
	std::vector<std::string>::const_iterator end = list1.end();
	std::sort(list1.begin(), list1.end());
	std::sort(list2.begin(), list2.end());
	return list1 == list2;
}

void lidar_final_exec::_checkBlock() {
	_getCoordNameList(_raw, _rawList);
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
			gp->addPoint(x + 2000, y);
			gp->addPoint(x + 2000, y + 2000);
			gp->addPoint(x, y + 2000);
			gp->closeRings();

			OGRGeomPtr rg_tmp = OGRGeometryFactory::createGeometry(wkbPolygon);
			OGRGeometry* pol_tmp_ = rg_tmp;
			OGRPolygon* pol_tmp = reinterpret_cast<OGRPolygon*>(pol_tmp_);
			pol_tmp->setCoordinateDimension(2);
			pol_tmp->addRing(gp);

			pol = pol->Union(pol_tmp);
		}
	}
	
	/*CV::Util::Spatialite::Statement stm(cnn);
	stm.prepare("select AsBinary(GEOM) as GEOM FROM Z_STRIPV");

	CV::Util::Spatialite::Recordset set = stm.recordset();

	if (set.eof()) {
		throw std::runtime_error("No data in Z_STRIPV");
	}
		
	Lidar::Block block;
	while (!set.eof()) {
		Blob b = set["GEOM"].toBlob();
		Lidar::Strip::Ptr strip(new Lidar::Strip(b));
		block.add(strip);
	}
	
	OGRGeomPtr dif = block.geom()->Difference(pol);
	if (dif->IsEmpty()) {
		return;
	}*/

}

void lidar_final_exec::_checkEquality() {
	if (!_sortAndCompare(_rawList, _rawList)) {
		std::cout << "Diff";
	}
}

void lidar_final_exec::Error(const std::string& operation, const std::exception& e) {
	std::cout << "Error [" << operation << "] : " << e.what() << std::endl;
}

void lidar_final_exec::Error(const std::string& operation) {
	std::cout << "Error [" << operation << "]" << std::endl;
}

