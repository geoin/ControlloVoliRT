/*
    File: check_photo.h
    Author:  F.Flamigni
    Date: 2013 October 29
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
#ifndef CHECK_PHOTO_H
#define CHECK_PHOTO_H

#include "photo_util/vdp.h"
#include "Poco/Util/Application.h"
#include "CVUtil/cvspatialite.h"
#include "CVUtil/ogrgeomptr.h"
#include "docbook/docbook.h"
#include "common/util.h"

class DSM_Factory;
class OGRPolygon;
class OGRGeometry;
class CV::Util::Spatialite::QueryField;
class CV::Util::Spatialite::BindField;

class GSD {
public:
	GSD() {}
	GSD(const DPOINT& p, const std::string& f, double dp): pt(p), foto(f), dpix(dp) { }
	DPOINT pt;
	std::string foto;
	double dpix;
};
typedef struct feature {
	std::string strip;
	std::string time;
	std::string time2;
	std::string date;
	std::string mission;
	int nsat;
	int nbasi;
	double pdop;
	double dist;
	CV::Util::Geometry::OGRGeomPtr pt;
} feature;
typedef std::map<std::string, feature> end_point_axis_info;

class photo_exec {
public:
	enum Check_Type {
		Prj_type = 0,
		fli_type = 1
	};
	photo_exec(): _df(NULL), _type(fli_type) {}
	~photo_exec();
	bool run(void);
	void set_proj_dir(const std::string& nome);
	void set_checkType(Check_Type t);
	void set_ref_scale(const std::string& nome);
private:
	//bool _read_cam(void);
	bool _read_vdp(std::map<std::string, VDP>& vdps);
	bool _calc_vdp(std::map<std::string, VDP>& vdps);
	void _assi_from_vdp(std::map<std::string, VDP>& vdps);
	void process_end_point_axis_info(const Blob& pt, end_point_axis_info& epai);
	void update_strips(std::vector<feature>& ft);
	bool select_mission(end_point_axis_info& ep1, end_point_axis_info& ep2, feature& f, double len);

	//bool _strip_cam(void); // strip camera association
	bool _read_dem(void);
	std::string _get_key(const std::string& val);

	void _process_models(void);
	void _process_strips(void);
	void _process_photos(void);
	void _process_gsd(std::vector<GSD>& vgsd);
	void _process_block(void);
	void _update_assi_volo(void);


	void _get_elong(CV::Util::Geometry::OGRGeomPtr fv, double ka, double* d1, double* d2);
	bool _get_carto(CV::Util::Geometry::OGRGeomPtr& blk);
	CV::Util::Geometry::OGRGeomPtr _get_dif(const OGRGeometry* cart, std::vector<CV::Util::Geometry::OGRGeomPtr>& blocks);

	bool _uncovered(CV::Util::Geometry::OGRGeomPtr& vs);

	void _init_document(void);
	void _final_report(void);
	bool _foto_report(void);
	bool _model_report(void);
	bool _strip_report(void);
	bool _prj_report(void);
	void _gps_report(void);

	bool _read_ref_val(void);

	bool _get_photo(void);
	//std::string _cam_name;
	std::string _vdp_name;
	std::string _vdp_name_proj;
	std::string _dem_name;
	CV::Util::Geometry::OGRGeomPtr _carto;

	Check_Type _type;

	std::string _db_name;
	std::string _proj_dir;

	// spatial lite connection
	CV::Util::Spatialite::Connection cnn;

	docbook _dbook;
	Doc_Item _article;

	std::map<std::string, VDP> _vdps; // map photo name  - photo attitude
	std::map<std::string, VDP> _vdps_plan; // same but for planned
	//std::map<std::string, Camera> _cams;
	std::map<std::string, Camera> _map_strip_cam;
	//Camera	_cam_plan; // camera for planned flight
	std::vector<std::string> _useless_models;
	DSM_Factory* _df;

	double _GSD, _MAX_GSD;
	double _MODEL_OVERLAP, _MODEL_OVERLAP_RANGE, _MODEL_OVERLAP_T;
	double _STRIP_OVERLAP, _STRIP_OVERLAP_RANGE;
	double _MAX_STRIP_LENGTH;
	double _MAX_HEADING_DIFF;
	double _MAX_ANG;
	// reference values
	double _MAX_PDOP;
	int _MIN_SAT;
	int _MAX_DIST;
	double _MIN_SAT_ANG;
	int _NBASI;
	double _MIN_ANG_SOL;
	double _MAX_GPS_GAP;

	std::string _refscale;
	std::string _note;
};

class check_photo: public Poco::Util::Application {
public:
	check_photo();

protected:	
	void initialize(Poco::Util::Application& self);
	void uninitialize();
	void reinitialize(Poco::Util::Application& self);
	void defineOptions(Poco::Util::OptionSet& options);
	void displayHelp();
	int main(const std::vector<std::string>& args);
private:
	void handleHelp(const std::string& name, const std::string& value);
	void handleFlight(const std::string& name, const std::string& value);
	void handleProject(const std::string& name, const std::string & value);
	void handlePrjDir(const std::string& name, const std::string & value);
	void handleScale(const std::string& name, const std::string & value);
	bool _helpRequested;

	photo_exec _phe;
};

#endif
