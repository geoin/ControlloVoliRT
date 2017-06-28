/*
    File: lidar_final_exec.h
    Author:  F.Flamigni
    Date: 2017 June 19
    Comment:

    Disclaimer:
        This file is part of RT_Controllo_Voli.

        Tabula is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        Tabula is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with Tabula.  If not, see <http://www.gnu.org/licenses/>.


        Copyright (C) 2013 Geoin s.r.l.

*/
#ifndef LIDAR_FINAL_EXEC_H
#define LIDAR_FINAL_EXEC_H

#include <iostream>
#include <string>

#include "cv/lidar.h"
#include "proj_api.h"
#include "CVUtil/ogrgeomptr.h"

class vGrid;
//class OGRGeomPtr;

class lidar_final_exec {

public:
    enum DATA_TYPE {
        TILE_GROUND = 0,
        TILE_OVERGROUND = 1
    };

	struct Stats {
		std::string target;
		double mean;
		double stdDev;
		unsigned int count;
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
	bool readReference();

    void set_proj_dir(const std::string&);

	void setTilesPointsPerc(int p) { _tilePP = p; }
    void setTilesFilesPerc(int p) { _tileFP = p; }
//	void setClassPointsPerc(int p) { _classPP = p; }
	void setResampleFilesPerc(int p) { _resFP = p; }
	void setResamplePointsPerc(int p) { _resPP = p; }
    //void setQuotaPointsPerc(int p) { _qPP = p; }

	static void Error(const std::string& operation, const std::exception& e); 
	static void Error(const std::string& operation); 
	static void GetStats(const std::vector<double>& diff, Stats&);

private:
	void _getCoordNameList(const std::string& fold, const std::string& ext, std::vector<std::string>& list, bool complete = true);
	bool _sortAndCompare(std::vector<std::string>& list1, std::vector<std::string>& list2, std::vector<std::string>& diff);
	std::string build_gridname( const std::vector<std::string>& list, const std::string& nome );
	
    std::string _fileFromCorner(const std::string& folder, const std::string& ext, const std::string& corner);
	std::string _getFolder(const std::string& table);
	std::string _getRawFolder(const std::string& table, unsigned int& step);
	void _getStrips(std::vector<CV::Lidar::Strip::Ptr>&);
    bool _isStripUsed( std::map< std::string, std::vector<NODE> >& points,CV:: Lidar::Strip::Ptr sit );

	size_t _getSamplesCount(size_t min, size_t max, size_t size, double perc = 0.1);
    void _checkBlock(const std::string& folder, const std::vector<std::string>& list, DATA_TYPE tiletype);
	void _checkEquality();
    void _checkRawRandom(const std::string& raw, int pulse, std::map< std::string, std::vector<double> >& rawRandomDiff );
    void _checkResamples(const std::string& folder1, const std::vector<std::string>& list1, const std::string& folder2,
                         const std::vector<std::string>& list2, std::vector<Stats>& diff, DATA_TYPE tiletype);
	void _checkQuota(const std::string& folder1, const std::string& folder2, std::vector<Stats>&);
    bool InitIGMIgrid( vGrid& grid );

    CV::Util::Geometry::OGRGeomPtr _get_cartoU();

	void _checkEllipsoidicData();
	void _checkFolderWithRaw(const std::string& folder, const std::vector<std::string>& data, const std::string&);

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

    std::map< std::string, std::vector<double> > groundRandomDiffg, overRandomDiff;

	std::vector<PointCheck> _pc;
	std::vector<Stats> diffMdt, diffMds, statsGround, statsOverGround;

	docbook _dbook;
	Doc_Item _article;
	std::string _note;

	void _reportBlock();
	void _reportEquality();
    void _reportRawRandom(std::map< std::string, std::vector<double> >& rawRandomDiff, DATA_TYPE tiletype);
	void _reportResamples();
	void _reportQuota();
	void _reportEllipsoidic();

    int _tileFP;    // percentuale di file da usare per classificazione
    int _tilePP;    // percentuale di punti da usare in ogni tile per classificazione
    //int _classFP;
    //int _classPP;
    int _resFP; // percentuale di file da usare per ricampionamento e quote
    int _resPP; // percentuale di punti da usare in ciascuna tile per ricampionamento e quote
    //int _qPP;
	double LID_ANG_SCAN, LID_TOL_A;
};

#endif
