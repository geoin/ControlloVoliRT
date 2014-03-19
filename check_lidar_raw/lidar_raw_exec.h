#ifndef LIDAR_RAW_EXEC_H
#define LIDAR_RAW_EXEC_H

#include <iostream>
#include <string>
#include "Poco/Path.h"

#include "cv/lidar.h"

class lidar_raw_exec {
public:
    void set_proj_dir(const std::string&);

	bool init();
    bool run();
	bool openDBConnection();

	static void Error(const std::string& operation, const std::exception& e); 

	struct Stats {
		std::string target;
		double mean;
		double stdDev;
	};

private:
	bool _initStripFiles();
	bool _initControlPoints();
	bool _initStripsLayer();

	bool _checkDensity();
	bool _checkIntersection();

	void _getStats(const std::vector<double>& diff, Stats&);

    std::string _proj_dir;
	
	CV::Util::Spatialite::Connection cnn;

	std::map<std::string, Poco::Path> _cloudStripList;
	std::vector<CV::Lidar::ControlPoint::Ptr> _controlVal;
	std::vector<CV::Lidar::CloudStrip::Ptr> _strips;
	std::multimap<std::string, Stats> _statList;
};

#endif
