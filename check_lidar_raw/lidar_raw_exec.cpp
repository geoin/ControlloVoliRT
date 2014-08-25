#include "lidar_raw_exec.h"

#include "Poco/File.h"

#include <sstream>
#include <numeric>
#include <math.h>
#include <cmath>

#define GEO_DB_NAME "geo.sqlite"

using namespace CV;

void lidar_raw_exec::Error(const std::string& operation, const std::exception& e) {
	std::cout << "Error [" << operation << "] : " << e.what() << std::endl;
}

void lidar_raw_exec::set_proj_dir(const std::string& proj) {
    _proj_dir = proj;
}

bool lidar_raw_exec::openDBConnection() {
	try {
		Poco::Path geodb(_proj_dir, GEO_DB_NAME);
		cnn.open(geodb.toString());
		return true;
	} catch (const std::exception& e) {
		Error("opening connection", e);
		return false;
	}
}

bool lidar_raw_exec::init() {
	if (!_initStripFiles()) {
		return false;
	}

	if (!_initControlPoints()) {
		return false;
	}

	if (!_initStripsLayer()) {
		return false;
	}

	return true;
}

bool lidar_raw_exec::run() {
	try {
		_checkDensity();
		_checkIntersection();
			
		return true;
	} catch (const std::exception& e) {
		Error("run", e);
		return false;
	}
}

bool lidar_raw_exec::_initControlPoints() {
	try {
		CV::Util::Spatialite::Statement stm(cnn);

		std::stringstream query;
		query << "select Z_QUOTA, NAME, AsBinary(GEOM) as GEOM FROM RAW_CONTROL_CLOUD";
		stm.prepare(query.str());

		CV::Util::Spatialite::Recordset set = stm.recordset();

		if (set.eof()) {
			throw std::runtime_error("No data in RAW_CONTROL_CLOUD");
		}
		
		while (!set.eof()) {
			Blob b = set["GEOM"].toBlob();
			Lidar::ControlPoint::Ptr point(new Lidar::ControlPoint(b, set["Z_QUOTA"].toDouble()));
			point->name(set["NAME"].toString());
			_controlVal.push_back(point);

			set.next();
		}
		return true;
	} catch (const std::exception& e) {
		Error("fetching control points", e);
		return false;
	}
}

bool lidar_raw_exec::_initStripFiles() {
	try {
		std::stringstream query;
		query << "select FOLDER from STRIP_RAW_DATA LIMIT 1"; //only one record should be here
		
		CV::Util::Spatialite::Statement stm(cnn);
		stm.prepare(query.str());
		CV::Util::Spatialite::Recordset set = stm.recordset();

		if (set.eof()) {
			throw std::runtime_error("No data in STRIP_RAW_DATA");
		}

		std::string folder = set["FOLDER"].toString();
		Poco::Path fPath(folder);
		
		std::vector<std::string> folderContent;
		Poco::File(fPath).list(folderContent);

		std::vector<std::string>::iterator it = folderContent.begin();
		std::vector<std::string>::iterator end = folderContent.end();
		for (; it != end; it++) {
			Poco::Path p = Poco::Path(fPath).append(*it);
			std::string name = p.getBaseName();
			_cloudStripList.insert(std::pair<std::string, Poco::Path>(name, p));
		}

		stm.reset();
		return true;
	} catch (const std::exception& e) {
		Error("fetching strips", e);
		return false;
	}
}

bool lidar_raw_exec::_initStripsLayer() {
	try {
		CV::Util::Spatialite::Statement stm(cnn);

		std::stringstream query;
		query << "select Z_STRIP_CS, Z_STRIP_YAW, Z_MISSION, Z_STRIP_LENGTH, AsBinary(GEOM) as GEOM FROM Z_STRIPV";
		stm.prepare(query.str());

		CV::Util::Spatialite::Recordset set = stm.recordset();

		if (set.eof()) {
			throw std::runtime_error("No data in Z_STRIPV");
		}
		
		while (!set.eof()) {
			Blob b = set["GEOM"].toBlob();
			Lidar::Strip::Ptr strip(new Lidar::Strip(b));
			strip->yaw(set["Z_STRIP_YAW"].toDouble());
			strip->missionName(set["Z_MISSION"].toString());
			strip->name(set["Z_STRIP_CS"].toString());

			Lidar::CloudStrip::Ptr cloud(new Lidar::CloudStrip(strip));
			std::map<std::string, Poco::Path>::iterator el = _cloudStripList.find(cloud->name());
			if (el == _cloudStripList.end()) {
				throw std::runtime_error("Strip " + cloud->name() + " missing");
			}	

			cloud->cloudPath(el->second.toString());
			_strips.push_back(cloud);

			set.next();
		}
		return true;
	} catch (const std::exception& e) {
		Error("fetching strips layer", e);
		return false;
	}
}

bool lidar_raw_exec::_checkDensity() {
	bool ret = _strips.size() == _cloudStripList.size();
	/*if (!ret) { 
		throw std::runtime_error("Layer strips number different from cloud files");
	}*/

	std::vector<CV::Lidar::CloudStrip::Ptr>::iterator it = _strips.begin();
	std::vector<CV::Lidar::CloudStrip::Ptr>::iterator end = _strips.end();

	for (; it != end; it++) {
		Lidar::CloudStrip::Ptr cloudStrip = *it;
		cloudStrip->computeDensity();
	}
	
	return ret;
}

bool lidar_raw_exec::_checkIntersection() {
	std::vector<Lidar::CloudStrip::Ptr>::iterator it = _strips.begin();
	std::vector<Lidar::CloudStrip::Ptr>::iterator end = _strips.end();
	
	for (; it != end; it++) {
		Lidar::CloudStrip::Ptr cloud = *it;
		Lidar::Strip::Ptr source = cloud->strip();
		std::vector<Lidar::CloudStrip::Ptr>::const_iterator next = it;
		for (next++; next != end; next++) {
			Lidar::CloudStrip::Ptr cloudTarget = *next;
			Lidar::Strip::Ptr target = cloudTarget->strip();
			if (source->isParallel(target) && source->intersect(target)) {
				Lidar::Strip::Intersection::Ptr intersection = source->intersection(target);

				std::vector<double> diff; 
				Lidar::DSMHandler srcDsm(cloud->dsm());
				unsigned int count = srcDsm->Npt();
				for (unsigned int i = 0; i < count; ++i) {
					DPOINT pt = srcDsm->Node(i);
					if (intersection->contains(pt)) {
						Lidar::DSMHandler targetDsm(cloudTarget->dsm());
						double zTrg = targetDsm->GetQuota(pt.x, pt.y);

						double dZ = pt.z - zTrg; 
						diff.push_back(dZ);
					}
				}
				if (!diff.size()) {
					continue;
				}
				
				Stats s = { target->name(), 0.0, 0.0 };
				_getStats(diff, s);

				_statList.insert(std::pair<std::string, Stats>(source->name(), s));
			}
		}
	}
	return true;
}

void lidar_raw_exec::_getStats(const std::vector<double>& diff, Stats& s) {
	//TODO
	double size = static_cast<double>(diff.size());
	double mean = std::accumulate(diff.begin(), diff.end(), 0.0) / size;

	std::vector<double>::const_iterator j = diff.begin();
	std::vector<double>::const_iterator end = diff.end();
	double mm = 0.0;
	for (; j != end; j++) {
		double val = *j;
		mm += std::pow(val - mean, 2.0);
	}

	s.mean = mean;
	s.stdDev = std::sqrt(mm / size);
}
