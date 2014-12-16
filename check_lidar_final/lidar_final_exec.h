#ifndef LIDAR_FINAL_EXEC_H
#define LIDAR_FINAL_EXEC_H

#include <iostream>
#include <string>

#include "cv/lidar.h"
#include "proj_api.h"

class lidar_final_exec {

public:
	struct Stats {
		std::string target;
		double mean;
		double stdDev;
	};

	struct PointCheck {
		std::string target;
		std::string group;
		unsigned long ok;
		unsigned long ko;

		PointCheck() : ok(0), ko(0) {}
	};

	lidar_final_exec();

    bool run();
	void createReport();

	bool openDBConnection();
	void readFolders();

    void set_proj_dir(const std::string&);

	static void Error(const std::string& operation, const std::exception& e); 
	static void Error(const std::string& operation); 
	static void GetStats(const std::vector<double>& diff, Stats&);

private:
	void _getCoordNameList(const std::string& fold, const std::string& ext, std::vector<std::string>& list, bool complete = false);
	bool _sortAndCompare(std::vector<std::string>& list1, std::vector<std::string>& list2, std::vector<std::string>& diff);
	
	std::string _fileFromCorner(const std::string& folder, const std::string& ext, const std::string& corner);
	std::string _getFolder(const std::string& table);
	std::string _getRawFolder(const std::string& table, unsigned int& step);
	void _getStrips(std::vector<CV::Lidar::Strip::Ptr>&);

	size_t _getSamplesCount(size_t min, size_t max, size_t size);
	void _checkBlock();
	void _checkEquality();
	void _checkRawRandom();
	void _checkResamples(const std::string& folder1, const std::vector<std::string>& list1, const std::string& folder2, const std::vector<std::string>& list2, std::vector<Stats>& diff);
	void _checkQuota(const std::string& folder1, const std::string& folder2, std::vector<Stats>&);

	void _checkEllipsoidicData();
	void _checkFolderWithRaw(const std::string& folder, const std::vector<std::string>& data);

	CV::Util::Spatialite::Connection cnn;

    std::string _proj_dir, _stripFolder;
	std::string _gridFile;

	std::string _raw, _mds, _mdt, _intensity, _groundEll, _groundOrto, _overgroundEll, _overgroundOrto; 
	unsigned int _step;

	std::vector<std::string> _rawList, _mdsList, _mdtList, _intensityList, _groundEllList, _groundOrtoList, _overgroundEllList, _overgroundOrtoList;

	bool _coversAll;
	std::vector<std::string> mdtDiff;
	std::vector<std::string> mdsDiff;
	std::vector<std::string> intensityDiff;
	std::vector<std::string> groundEllDiff;
	std::vector<std::string> groundOrtoDiff;
	std::vector<std::string> overGroundEllDiff;
	std::vector<std::string> overGroundOrtoDiff;

	std::map< std::string, std::vector<double> > rawRandomDiff;

	std::vector<PointCheck> _pc;
	std::vector<Stats> diffMdt, diffMds, statsGround, statsOverGround;

	docbook _dbook;
	Doc_Item _article;
	std::string _note;

	void _reportBlock();
	void _reportEquality();
	void _reportRawRandom();
	void _reportResamples();
	void _reportQuota();
	void _reportEllipsoidic();
};

#endif
