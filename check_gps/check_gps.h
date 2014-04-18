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
#include "CVUtil/cvspatialite.h"
#include "CVUtil/ogrgeomptr.h"
#include "docbook/docbook.h"

typedef struct {
    std::string data;
    DPOINT pos;
    int nsat;
    double pdop;
    double rms;
    int id_base;
    double dist;
} GRX;

class gps_exec {
public:
	enum gps_type {
		rover_type = 0,
		base_type = 1
	};
	enum Check_Type {
		phot_type = 0,
		lid_type = 1
	};
	gps_exec() {}
	~gps_exec();
	bool run(void);
	void set_proj_dir(const std::string& nome);
	void set_checkType(Check_Type t);


private:
	std::string _getnome(const std::string& nome, gps_type type);
	void _add_column(const std::string& col_name) ;
	void _createGPSMissionsTables();

	// compensa le tracce relative alle singole basi producendo una traccia unica
	bool _single_track(const std::string& mission, std::vector< Poco::SharedPtr<vGPS> >& vvg, MBR* mbr);
	// registra i dati relativi alle basi utilizzate per la singola missione
	bool _record_base_file(const std::vector<DPOINT>& basi, const std::vector<std::string>& vs_base);
	// processa la singola missione
	bool _mission_process(const std::string& folder);

	bool _read_ref_val(void);
	
	// calculate the gps track
	bool _create_gps_track(void);
	// Update the flight lines with gps data
	void _update_assi_volo(void);

	// check with the limit parameters
	void _final_report(void);
	void _init_document(void);
	std::string _get_key(const std::string& val);

	std::string _rover_name;
	std::string _sigla_base;
	std::vector<std::string> _vs_base;
	GPS_OPT _gps_opt;

	std::string _proj_dir;
	Check_Type _type;

	// spatial lite connection
	CV::Util::Spatialite::Connection cnn;
	
	docbook _dbook;
	Doc_Item _article;

	// reference values
	double _MAX_PDOP;
	int _MIN_SAT;
	int _MAX_DIST;
	double _MIN_SAT_ANG;
	int _NBASI;
	double _MIN_ANG_SOL;
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
private:
	void _handlePrjDir(const std::string & name, const std::string & value);
	void _handleHelp(const std::string& name, const std::string& value);
	void _handlePhoto(const std::string& name, const std::string& value);
	void _handleLidar(const std::string& name, const std::string& value);
	bool _helpRequested;
};
#endif
