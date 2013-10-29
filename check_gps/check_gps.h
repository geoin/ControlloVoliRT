/* 
	File: check_gps.h
	Author:  F.Flamigni
	Date: 2013 October 22
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
#ifndef CHECK_GPS_H
#define CHECK_GPS_H

#include "gps.h"
#include "Poco/Util/Application.h"
#include <spatialite/sqlite.h>

class gps_exec {
public:
	enum gps_type {
		rover_type = 0,
		base_type = 1
	};
	gps_exec() {}
	bool run(void);
	void data_analyze(void);

	void set_out_folder(const std::string& nome);
	void set_proj_dir(const std::string& nome){ _proj_dir.assign(nome); }

private:
	std::string _getnome(const std::string& nome, gps_type type);
	std::string _hathanaka(const std::string& nome);
	std::vector<std::string> _rawConv(const std::string& nome);
	bool _single_track(const std::string& mission, std::vector< Poco::SharedPtr<vGPS> >& vvg, MBR* mbr);
	bool _record_base_file(const std::vector<DPOINT>& basi, const std::vector<std::string>& vs_base);
	bool _mission_process(const std::string& folder);
	bool _init(void);
	
	void foo(const std::string& table);

	std::string _rover_name;
	std::string _sigla_base;
	std::vector<std::string> _vs_base;
	GPS_OPT _gps_opt;

	std::string _out_folder;
	std::string _db_name;
	std::string _proj_dir;

	sqlite3 *db_handle;
	sqlite3_stmt *stmt;
};

class check_gps: public Poco::Util::Application {
public:
	check_gps();
	gps_exec _gps;
protected:	
	void initialize(Poco::Util::Application& self);
	void uninitialize(void);
	void reinitialize(Poco::Util::Application& self);
	void defineOptions(Poco::Util::OptionSet& options);
	void displayHelp(void);
	void defineProperty(const std::string& def);
	int main(const std::vector<std::string>& args);
	void printProperties(const std::string& base);
private:
	void _handlePrj(const std::string & name, const std::string & value);
    void _handleConfigFile(const std::string & name, const std::string & value);
    void _handleConfigType(const std::string & name, const std::string & value);
	void _handleHelp(const std::string& name, const std::string& value);
	bool _helpRequested;
};
#endif
