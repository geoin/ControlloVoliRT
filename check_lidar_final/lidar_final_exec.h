#ifndef LIDAR_FINAL_EXEC_H
#define LIDAR_FINAL_EXEC_H

#include <iostream>
#include <string>

#include "cv/lidar.h"

class lidar_final_exec {

public:
    bool run();
	bool createReport();

	bool openDBConnection();
	void readFolders();

    void set_proj_dir(const std::string&);

	static void Error(const std::string& operation, const std::exception& e); 
	static void Error(const std::string& operation); 

private:
	void _getCoordNameList(const std::string& fold, std::vector<std::string>& list);
	bool _sortAndCompare(std::vector<std::string>& list1, std::vector<std::string>& list2);

	std::string _getFolder(const std::string& table);
	std::string _getRawFolder(const std::string& table, unsigned int& step);

	void _checkBlock();
	void _checkEquality();

	CV::Util::Spatialite::Connection cnn;

    std::string _proj_dir;

	std::string _raw, _mds, _mdt, _intensity, _groundEll, _groundOrto, _overgroundEll, _overgroundOrto; 
	unsigned int _step;

	std::vector<std::string> _rawList;
};

#endif
