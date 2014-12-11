#ifndef LIDAR_FINAL_EXEC_H
#define LIDAR_FINAL_EXEC_H

#include <iostream>
#include <string>

#include "cv/lidar.h"
#include "proj_api.h"

class lidar_final_exec {

public:
	lidar_final_exec();

    bool run();
	bool createReport();

	bool openDBConnection();
	void readFolders();

    void set_proj_dir(const std::string&);

	static void Error(const std::string& operation, const std::exception& e); 
	static void Error(const std::string& operation); 

private:
	void _getCoordNameList(const std::string& fold, const std::string& ext, std::vector<std::string>& list, bool complete = false);
	bool _sortAndCompare(std::vector<std::string>& list1, std::vector<std::string>& list2);
	
	std::string _fileFromCorner(const std::string& folder, const std::string& ext, const std::string& corner);
	std::string _getFolder(const std::string& table);
	std::string _getRawFolder(const std::string& table, unsigned int& step);
	void _getStrips(std::vector<CV::Lidar::Strip::Ptr>&);

	void _checkBlock();
	void _checkEquality();
	void _checkRawRandom();
	void _checkResamples();
	void _checkQuota();

	void _checkEllipsoidicData();
	unsigned long _checkFolderWithRaw(const std::string& folder, const std::vector<std::string>& data);

	CV::Util::Spatialite::Connection cnn;

    std::string _proj_dir, _stripFolder;
	std::string _gridFile;

	std::string _raw, _mds, _mdt, _intensity, _groundEll, _groundOrto, _overgroundEll, _overgroundOrto; 
	unsigned int _step;

	std::vector<std::string> _rawList, _mdsList, _mdtList, _intensityList, _groundEllList, _groundOrtoList, _overgroundEllList, _overgroundOrtoList;
};

#endif
