/*
    File: check_ta.h
    Author:  F.Flamigni
    Date: 2013 November 06
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
#ifndef CHECK_TA_H
#define CHECK_TA_H

#include "photo_util/vdp.h"
#include "Poco/Util/Application.h"
#include "docbook/docbook.h"
#include "CVUtil/cvspatialite.h"

class ta_exec {
public:
	enum Check_Type {
		Prj_type = 0,
		fli_type = 1
	};
	typedef std::map<std::string, VDPC> VDP_MAP;
	typedef std::map<std::string, DPOINT> CPT_MAP;
	typedef std::multimap<std::string, std::string> CPT_VDP;
	ta_exec() {}
	~ta_exec();
	bool run(void);
	void set_proj_dir(const std::string& nome);
	void set_vdp_name(const std::string& nome);
	void set_vdp_name2(const std::string& nome);
	void set_obs_name(const std::string& nome);
	//void set_ref_scale(const std::string& nomevalue);

private:
	//bool _read_cam(void);
	//bool _strip_cam(void);
	void _read_vdp(const std::string& nome, VDP_MAP& vdps);
	bool _read_image_pat(VDP_MAP& vdps, const CPT_MAP& pm, CPT_VDP& pts);
	bool _read_cont_pat(CPT_MAP& pm); 
	bool _check_differences(void);
	bool _check_cpt(void);
	bool _read_ref_val(void);
	bool _calc_pts(VDP_MAP& vdps, const CPT_MAP& pm, const CPT_VDP& pts);
	//void _init_document(void);
	Doc_Item _initpg1(void);
	Doc_Item _initpg2(void);
	void _resume(void);
	std::string _get_key(const std::string& val);

	bool _add_point_to_table(Doc_Item tbody, const std::string& cod, const std::string& nome1, const std::string& nome2, const DPOINT& sc);
	bool _add_point_to_table(Doc_Item tbody, const std::string& foto, const VecOri& pt, const VecOri& sc);

	bool _get_photo(void);
	std::string _cam_name;
	std::string _vdp_name;
	std::string _vdp_name_2;
	std::string _obs_name;
	std::string _proj_dir;

	//Camera	_cam_plan; // camera for planned flight
	//std::map<std::string, Camera> _cams;
	std::map<std::string, Camera> _map_strip_cam;
	docbook _dbook;
	Doc_Item _article;
	
	double _T_CP;
	double _T_PR;
	double _T_H;
	double _TP_PA;
	double _TA_PA;
	std::string _refscale;
	std::string _note;

	std::list<std::string> _cpt_out_tol;
	std::list<std::string> _tria_out_tol;
	
	// spatial lite connection
	CV::Util::Spatialite::Connection cnn;
};

class check_ta: public Poco::Util::Application {
public:
	check_ta();

protected:	
	void initialize(Poco::Util::Application& self);
	void uninitialize();
	void reinitialize(Poco::Util::Application& self);
	void defineOptions(Poco::Util::OptionSet& options);
	void displayHelp();
	int main(const std::vector<std::string>& args);
private:
	void handlePrjDir(const std::string & name, const std::string & value);
	void handleRef(const std::string& name, const std::string& value);
	void handleComp(const std::string& name, const std::string& value);
	void handleObs(const std::string& name, const std::string& value);
	//void handleScale(const std::string& name, const std::string& value);
	void handleHelp(const std::string& name, const std::string& value);
	bool _helpRequested;

	ta_exec _tae;
};

#endif
