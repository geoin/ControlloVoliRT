#ifndef LIDAR_RAW_EXEC_H
#define LIDAR_RAW_EXEC_H

#include <iostream>
#include <string>
#include "Poco/Path.h"

#include "cv/lidar.h"


class lidar_raw_exec {
public:
    lidar_raw_exec() : LID_TOL_Z(0.0), LID_TOL_A(0.0),  INTERSECTION_DENSITY(5.0/100.0), LID_ANG_SCAN(0) {}

    void set_proj_dir(const std::string&);
	void setIntersectionDensity(int perc) { INTERSECTION_DENSITY = perc/100.0f; }

	bool init();
    bool run();
	bool report();

	bool openDBConnection();
	bool readReference();

	static void Error(const std::string& operation, const std::exception& e); 
	static void Error(const std::string& operation); 

	struct Stats {
		std::string target;
        long count;
		double mean;
		double stdDev;
	};

private:
	bool _initStripFiles();
	void _traverseFolder(const Poco::Path& fPath);
	bool _initControlPoints();
	bool _initStripsLayer();
    std::string _get_overlapped_cloud(const std::string& cloud);
    CV::Lidar::CloudStrip::Ptr _get_strip_by_name(const std::string& cloud);

	bool _checkDensity();
	bool _checkIntersection();

	void _getStats(const std::vector<double>& diff, Stats&);

	void _control_points_report();
	void _strip_overlaps_report();
	void _density_report();

	void _getIntersectionDiff(CV::Lidar::DSMHandler&, std::vector<DPOINT>&, std::vector<double>&); 
	void _checkControlPoints(const std::string&, CV::Lidar::DSMHandler&);

    std::string _proj_dir;

	//bool _initBlocks();
	//CV::Lidar::Block::Ptr _block;
	
	CV::Util::Spatialite::Connection cnn;

	std::map<std::string, Poco::Path> _cloudStripList;
	std::vector<CV::Lidar::CloudStrip::Ptr> _strips;
	std::multimap<std::string, Stats> _statList;
	
	std::vector<CV::Lidar::ControlPoint::Ptr> _controlVal;
	std::map< std::string, std::vector<double> > _controlInfoList;

	docbook _dbook;
	Doc_Item _article;
	std::string _note;

	double LID_TOL_Z, LID_TOL_A;
	double INTERSECTION_DENSITY;
    double LID_ANG_SCAN;
};

#endif
