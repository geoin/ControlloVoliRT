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
	void set_out_folder(const std::string& nome);
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
	void _get_elong(OGRPolygon* fv, double ka, double* d1, double* d2);
	OGRGeometry* _get_carto(void);
	OGRGeometry* GetGeom(CV::Util::Spatialite::QueryField& rs);
	void SetGeom(CV::Util::Spatialite::BindField& bf, const OGRGeometry* og);




	bool _read_ref_val(void);
	std::string _get_strip(const std::string& nome);
	std::string _get_nome(const std::string& nome);

	bool _get_photo(void);
	std::string _cam_name;
	std::string _vdp_name;
	std::string _dem_name;
	std::string _out_folder;

	Check_Type _type;

	std::string _db_name;
	std::string _proj_dir;

	// spatial lite connection
	CV::Util::Spatialite::Connection cnn;

	std::map<std::string, VDP> _vdps;
	Camera	_cam;
	DSM_Factory* _df;
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
