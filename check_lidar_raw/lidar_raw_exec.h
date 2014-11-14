#ifndef LIDAR_RAW_EXEC_H
#define LIDAR_RAW_EXEC_H

#include <iostream>
#include <string>
#include "Poco/Path.h"

#include "cv/lidar.h"

class lidar_raw_exec {
public:
	lidar_raw_exec() : LID_TOL_Z(0.0), PT_DENSITY(0.0) {}

    void set_proj_dir(const std::string&);

	bool init();
    bool run();
	bool report();

	bool openDBConnection();
	bool readReference();

	static void Error(const std::string& operation, const std::exception& e); 
	static void Error(const std::string& operation); 

	struct Stats {
		std::string target;
		double mean;
		double stdDev;
	};

private:
	bool _initStripFiles();
	void _traverseFolder(const Poco::Path& fPath);
	bool _initControlPoints();
	bool _initStripsLayer();
	bool _initBlocks();

	bool _checkDensity();
	bool _checkIntersection();

	void _getStats(const std::vector<double>& diff, Stats&);

	void _control_points_report();
	void _strip_overlaps_report();
	void _density_report();

    std::string _proj_dir;
	CV::Lidar::Block::Ptr _block;
	
	CV::Util::Spatialite::Connection cnn;

	std::map<std::string, Poco::Path> _cloudStripList;
	std::vector<CV::Lidar::ControlPoint::Ptr> _controlVal;
	std::vector<CV::Lidar::CloudStrip::Ptr> _strips;
	std::multimap<std::string, Stats> _statList;

	docbook _dbook;
	Doc_Item _article;
	std::string _note;

	double LID_TOL_Z, PT_DENSITY;
};

#endif
