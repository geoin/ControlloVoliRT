/*==============================================================================
!   Filename:  GPS.H
!
!   Contents:	interface for rinex manager
!
!   History:
!			28/12/2011	created
!			07/12/2012	updated
!================================================================================*/
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
};
typedef std::vector<GPS_DATA> vGPS;
bool RinexPost(const std::string& rover, const std::string& out, vGPS* data);//, Abort* ab = NULL);
bool RinexPost(const std::string& rover, const std::string& base, const std::string& out, MBR* mbr, vGPS* data, GPS_OPT* gps = NULL);//, Abort* ab = NULL);
int Crx2Rnx(const char* crx);
int Raw2Rnx(const char* crx, const char* ext, std::vector<std::string>& vs);

#endif
