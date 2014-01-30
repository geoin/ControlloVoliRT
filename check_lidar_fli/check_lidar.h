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
#include "geom.h"

class Lidar {
public:
	double fov;
	double ifov;
	double freq;
	double scan;
};

class lidar_exec {
public:
	enum Check_Type {
		Prj_type = 0,
		fli_type = 1
	};
	lidar_exec() {}
	~lidar_exec();
	bool run(void);
	void set_proj_dir(const std::string& nome);
	void set_checkType(Check_Type t);
private:
	void _get_strips(void);
	bool _read_lidar(void);
	std::string _get_strip(const std::string& nome);
	bool _check_differences(void);
	bool _check_cpt(void);
	bool _read_ref_val(void);
	void _init_document(void);
	Doc_Item _initpg1(void);
	Doc_Item _initpg2(void);


	bool _add_point_to_table(Doc_Item tbody, const std::string& cod, const std::string& nome1, const std::string& nome2, const DPOINT& sc);
	bool _add_point_to_table(Doc_Item tbody, const std::string& foto, const VecOri& pt, const VecOri& sc);

	bool _get_photo(void);

	std::string _prj_folder;
	Check_Type _type;

	Lidar	_lidar;
	docbook _dbook;
	Doc_Item _article;
	
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
