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

class DSM_Factory;
class OGRPolygon;
class OGRGeometry;
class CV::Util::Spatialite::QueryField;
class CV::Util::Spatialite::BindField;

class photo_exec {
public:
	enum Check_Type {
		Prj_type = 0,
		fli_type = 1
	};
	photo_exec(): _df(NULL) {}
	~photo_exec();
	bool run(void);
	void set_cam_name(const std::string& nome);
	void set_vdp_name(const std::string& nome);
	void set_dem_name(const std::string& nome);
	void set_proj_dir(const std::string& nome);
	void set_checkType(Check_Type t);
private:
	bool _read_cam(void);
	bool _read_vdp(void);
	bool _read_dem(void);

	bool _process_models(void);
	bool _process_strips(void);
	bool _process_photos(void);
	bool _process_block(void);
	void _final_report(void);
	void _get_elong(CV::Util::Geometry::OGRGeomPtr fv, double ka, double* d1, double* d2);
	bool _get_carto(std::vector<CV::Util::Geometry::OGRGeomPtr>& blocks);
	CV::Util::Geometry::OGRGeomPtr _get_dif(const OGRGeometry* cart, std::vector<CV::Util::Geometry::OGRGeomPtr>& blocks);

	void _uncovered(std::vector<CV::Util::Geometry::OGRGeomPtr>& vs);

	void _init_document(void);
	bool _foto_report(void);
	bool _model_report(void);
	bool _strip_report(void);
	bool _prj_report(void);

	bool _read_ref_val(void);

	bool _get_photo(void);
	std::string _cam_name;
	std::string _vdp_name;
	std::string _dem_name;

	Check_Type _type;

	std::string _db_name;
	std::string _proj_dir;

	// spatial lite connection
	CV::Util::Spatialite::Connection cnn;

	docbook _dbook;
	Doc_Item _article;

	std::map<std::string, VDP> _vdps;
	Camera	_cam;
	DSM_Factory* _df;

	double _GSD, _MAX_GSD;
	double _MODEL_OVERLAP, _MODEL_OVERLAP_RANGE, _MODEL_OVERLAP_T;
	double _STRIP_OVERLAP, _STRIP_OVERLAP_RANGE;
	double _MAX_STRIP_LENGTH;
	double _MAX_HEADING_DIFF;
	double _MAX_ANG;
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
	void defineProperty(const std::string& def);
	int main(const std::vector<std::string>& args);
	void printProperties(const std::string& base);
private:
	void handleCam(const std::string & name, const std::string & value);
	void handlePcent(const std::string & name, const std::string & value);
	void handleDtm(const std::string & name, const std::string & value);
	void handleCarto(const std::string & name, const std::string & value);
	void handleFlight(const std::string & name, const std::string & value);
	void handlePrj(const std::string & name, const std::string & value);
	void handlePline(const std::string & name, const std::string & value);
	void handleFline(const std::string & name, const std::string & value);
	void handleHelp(const std::string& name, const std::string& value);
	void handleDefine(const std::string& name, const std::string& value);
	void handleConfig(const std::string& name, const std::string& value);
	bool _helpRequested;

	photo_exec _phe;
};

#endif
