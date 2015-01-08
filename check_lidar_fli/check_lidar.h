/*
    File: check_lidar.h
    Author:  F.Flamigni
    Date: 2013 November 22
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
#ifndef CHECK_LIDAR_H
#define CHECK_LIDAR_H

//#include "photo_util/vdp.h"
#include "Poco/Util/Application.h"
#include "docbook/docbook.h"
#include "CVUtil/cvspatialite.h"
#include "CVUtil/ogrgeomptr.h"
#include "dem_interpolate/geom.h"

#include "cv/lidar.h"

class DSM_Factory;

/*
class Lidar {
public:
	double fov;
	double ifov;
	double freq;
	double scan;
};*/

struct AxisVertex {
	CV::GPS::Sample::Ptr sample;
	double dist;
	std::string time2;

	AxisVertex() : dist(0.0f) {}
};


class lidar_exec {
public:
	enum Check_Type {
		PRJ_TYPE = 0,
		FLY_TYPE = 1
	};

	lidar_exec() : STRIP_OVERLAP(0), STRIP_OVERLAP_RANGE(0), MAX_STRIP_LENGTH(0), PT_DENSITY(0), LID_ANG_SCAN(0) { }
	~lidar_exec();

	bool run(void);
	void set_proj_dir(const std::string& nome);
	void set_checkType(Check_Type t);

private:
	void _createAvolov();
	void _buildAxis();
	void _process_strips();
	void _traverseFolder(const Poco::Path&, CV::Util::Spatialite::Statement&);
	void _createStripTable(); 

	void _process_block();
	void _get_dif();

	void _get_overlaps(const std::map<std::string, CV::Lidar::Strip::Ptr>&);

	bool _read_lidar();
	bool _read_strip_folder();
	bool _read_lidar_from_mission();

	void _compare_axis();
	void _get_planned_axis(std::vector<CV::Lidar::Axis::Ptr>&);
	void _compare_axis_report(std::map<CV::Lidar::Axis::Ptr, CV::Lidar::Axis::Ptr>&);

	bool _read_dem(void);

	CV::Util::Spatialite::Recordset _read_control_points();
	bool _check_sample_cloud();
	bool _read_cloud();

	std::string _get_strip(const std::string& nome);
	bool _check_differences(void);
	bool _check_cpt(void);
	bool _read_ref_val(void);
	void _init_document(void);
	Doc_Item _initpg1(void);
	Doc_Item _initpg2(void);

	void _update_assi_volo();
	std::string _findLasByName(const std::string& las);

	void _final_report();
    void _strips_comp_report();
	void _strip_report();
    void _control_points_report();
	void _gps_report();

	bool _add_point_to_table(Doc_Item tbody, const std::string& cod, const std::string& nome1, const std::string& nome2, const DPOINT& sc);
	bool _add_point_to_table(Doc_Item tbody, const std::string& foto, const VecOri& pt, const VecOri& sc);

	//bool _get_photo(void);

	double read_proj_speed();
	void process_end_point_axis_info(const Blob& pt, std::map<std::string, AxisVertex>& epi);
	bool select_mission(std::map<std::string, AxisVertex>& ep1, std::map<std::string, AxisVertex>& ep2, CV::GPS::Sample::Ptr& f, double len);
	void update_strips(std::vector<CV::GPS::Sample::Ptr>& ft);

	std::string _proj_dir;
	// spatial lite connection
	CV::Util::Spatialite::Connection cnn;

	Check_Type _type;
	
	DSM_Factory* _df;

	Poco::SharedPtr<DSM_Factory> _sampleCloudFactory;
	CV::Lidar::Sensor::Ptr _lidar;
	std::map<std::string, CV::Lidar::Sensor::Ptr> _lidarsList;
	std::map<std::string, CV::Lidar::Strip::Ptr> _strips;
	std::map<std::string, CV::Lidar::Strip::Ptr> _invalidStrips;
	std::vector<CV::Lidar::ControlPoint::Ptr> _controlVal;

	docbook _dbook;
	Doc_Item _article;
	std::string _dem_name;
	std::string _note;

    int STRIP_OVERLAP, STRIP_OVERLAP_RANGE, MAX_STRIP_LENGTH, MAX_PDOP, MIN_SAT, MIN_SAT_ANG, NBASI;
	double LID_ANG_SCAN, LID_TOL_A, PT_DENSITY, MAX_DIST;

	void _findReferenceColumns();
	std::string _quotaCol, _stripNameCol;

	std::string _cloudsFolder;
};

class check_lidar: public Poco::Util::Application {
public:
    check_lidar();

protected:	
	void initialize(Poco::Util::Application& self);
	void uninitialize();
	void reinitialize(Poco::Util::Application& self);
	void defineOptions(Poco::Util::OptionSet& options);
	void displayHelp();
	void defineProperty(const std::string& def);
	int main(const std::vector<std::string>& args);
	void printProperties(const std::string& base);
private:
	void handleFlight(const std::string& name, const std::string& value);
	void handleProject(const std::string& name, const std::string& value);
	void handlePrjDir(const std::string & name, const std::string & value);
	void handleScale(const std::string & name, const std::string & value);

	void handleHelp(const std::string& name, const std::string& value);
	bool _helpRequested;

	lidar_exec _lix;
};

#endif
