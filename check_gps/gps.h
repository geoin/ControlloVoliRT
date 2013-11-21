/* 
	File: gps.h
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
#ifndef GPS_H
#define GPS_H

#include <string>
#include <vector>
#include "dem_interpolate/geom.h"
#include "exports.h"

class GPS_DATA {
public:
	DPOINT pos;
	std::string data;
};
class GPS_OPT {
public:
	enum pos_mode {
		Single = 6,
		Kinematic = 2,
		Static = 3
	};
	enum sol_typ {
		forward = 0,
		backward = 1,
		combined = 2
	};
	enum nav_sys {
		gps = 1,
		gloass = 4,
		gps_glonass = 5
	};
	GPS_OPT(): Position_mode(Kinematic), Solution_type(forward), Nav_sys(gps), frequence(2), max_base_dst(0.) {}
	GPS_OPT(int pos, int sol, int nav, int fre): Position_mode(pos), Solution_type(sol), Nav_sys(nav), frequence(fre), max_base_dst(0.) {}
	GPS_OPT(const GPS_OPT& gp): Position_mode(gp.Position_mode), Solution_type(gp.Solution_type), Nav_sys(gp.Nav_sys), frequence(gp.frequence), max_base_dst(0.) {}
	GPS_OPT operator=(const GPS_OPT& gp) {
		Position_mode = gp.Position_mode;
		Solution_type = gp.Solution_type;
		Nav_sys = gp.Nav_sys;
		frequence = gp.frequence;
		max_base_dst = gp.max_base_dst;
		return *this;
	}
	int Position_mode;
	int Solution_type;
	int Nav_sys;
	int frequence;
	double max_base_dst;
	double min_sat_angle;
};
typedef std::vector<GPS_DATA> vGPS;

bool RinexPost(const std::string& rover, const std::string& base, const std::string& out, MBR* mbr, vGPS* data, GPS_OPT* gps = NULL);
//int Crx2Rnx(const char* crx);
//int Raw2Rnx(const char* crx, const char* ext, std::vector<std::string>& vs);
std::vector<std::string> RawConv(const std::string& nome);
std::string Hathanaka(const std::string& nome);
#endif
