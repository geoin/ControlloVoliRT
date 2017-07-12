/*
    File: check_ortho.h
    Author:  F.Flamigni
    Date: 2013 November 12
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
#ifndef CHECK_ORTHO_H
#define CHECK_ORTHO_H

//#include "dem_interpolate/geom.h"
#include "Poco/Util/Application.h"
#include "CVUtil/cvspatialite.h"
#include "CVUtil/ogrgeomptr.h"
#include "docbook/docbook.h"
#include "common/geom.h"

class ortho_exec {
public:

	ortho_exec() {}
	~ortho_exec();
	bool run(void);
	void set_proj_dir(const std::string& nome);
	void set_img_dir(const std::string& nome);

    static bool get_filename_from_ext(const Poco::Path&, const std::string& target, const std::string& ext, Poco::Path&);

	//void set_ref_scale(const std::string& nome);
private:
	bool _process_imgs(void);
	bool _process_borders(void);
	//bool _process_img_border(const std::string& foglio, CV::Util::Geometry::OGRGeomPtr& pol);
	bool _process_img_border(const std::string& foglio, std::vector<DPOINT>& pt);

	bool _read_tfw(const std::string& nome);
	bool _final_report(void);

	bool _read_ref_val(void);
	//void _init_document(void);

	std::string _proj_dir;
	std::string _img_dir;
	std::vector<std::string> _fogli;

	// parametri connessione a sqlite
	CV::Util::Spatialite::Connection cnn;
	
	docbook _dbook;
	Doc_Item _article;
	std::string _refscale;
	std::string _note;
};

class check_ortho: public Poco::Util::Application {
public:
	check_ortho();

protected:	
	void initialize(Poco::Util::Application& self);
	void uninitialize();
	void reinitialize(Poco::Util::Application& self);
	void defineOptions(Poco::Util::OptionSet& options);
	void displayHelp();
	void defineProperty(const std::string& def);
	int main(const std::vector<std::string>& args);
private:
	void handleHelp(const std::string& name, const std::string& value);
	void handlePrjDir(const std::string& name, const std::string & value);
	//void handleScale(const std::string& name, const std::string & value);
	void handleImgDir(const std::string& name, const std::string & value);

	void handleDefine(const std::string& name, const std::string& value);
	void handleConfig(const std::string& name, const std::string& value);
	bool _helpRequested;

	ortho_exec _otx;
};

#endif
