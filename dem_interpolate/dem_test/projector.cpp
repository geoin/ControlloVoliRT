// projector.cpp : Defines the entry point for the DLL application.
//
#define _USE_MATH_DEFINES
#include <string>
#include "projlib/projects.h"
#include "projlib/projector.h"

using namespace std;

namespace Projection {
	const double	projector::RES60=60000.;
	const double	projector::CONV= 180 / PI;
	const double	projector::CONV1= PI / (180 * 3600);

	//! definition of well known projection
	char *Projection::projector::_wk_proj[] = {
		"+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs", // WGS84

		//"+proj=utm +zone=32 +ellps=intl +towgs84=-87.51,-96.61,-124.18,0,0,0,0 +units=m +no_defs", //UTM ED50 FUSE 32
		"+proj=utm +zone=32 +ellps=intl +towgs84=-87,-98,-121,0,0,0,0 +units=m +no_defs", //UTM ED50 FUSE 32
		"+proj=utm +zone=33 +ellps=intl +towgs84=-87,-98,-121,0,0,0,0 +units=m +no_defs", //UTM ED50 FUSE 33
		"+proj=utm +zone=34 +ellps=intl +towgs84=-87,-98,-121,0,0,0,0 +units=m +no_defs",  //UTM ED50 FUSE 34

		// GAUSS-BOAGA Rome40 Zone 1
		//"+proj=tmerc +lat_0=0 +lon_0=9 +k=0.999600 +x_0=1500000 +y_0=0 +ellps=intl +towgs84=-220.42,-66.18,13.04,0,0,0,0 +units=m +no_defs",
		"+proj=tmerc +lat_0=0 +lon_0=9 +k=0.999600 +x_0=1500000 +y_0=0 +ellps=intl +towgs84=-225,-65,9,0,0,0,0 +units=m +no_defs",
		// GAUSS-BOAGA Rome40 Zone 2
		"+proj=tmerc +lat_0=0 +lon_0=15 +k=0.999600 +x_0=2520000 +y_0=0 +ellps=intl +towgs84=-225,-65,9,0,0,0,0 +units=m +no_defs"
	};

	// reset actual projection
	void projector::_clear() {
		if ((PJ *)_proj){ 
			pj_free((PJ *)_proj);
			_proj = NULL;
			_ini_string.clear();
		}
	}

	bool projector::init(string const &inistr) {
		// eventually clear current project
		_clear();
		// initialize projector
		_proj = (PJ *)pj_init_plus(inistr.c_str());
		if ( _proj == NULL ) return false;
		_ini_string = inistr;
		return true;
	}

	void projector::_project(double* x, double* y, double* z, bool isdirect) const {
		projUV xy;
		xy.u = *x;
		xy.v = *y;
		if ( isdirect )
			xy = pj_fwd(xy, (PJ *)_proj);
		else
			xy = pj_inv(xy, (PJ *)_proj);
		*x = xy.u; *y = xy.v;
	}

	double projector::deg_to_rad( degree const &deg ){
		// calculate seconds
		double secs = deg.hh * 3600 + deg.mm * 60 + deg.sec;
		// converts seconds to radians
		return secs * CONV1;
	}

	degree projector::rad_to_deg( double rad ){
		double ddeg, dmin;

		int sign;
		if (rad < 0) 
		{
			rad = -rad;
			sign = -1;
		} 
		else
			sign = 1;
		
		degree deg;
		// decimal degrees
		ddeg = rad * CONV;
		deg.hh = (long)floor(ddeg);

		// decimal minutes
		dmin = (ddeg - deg.hh) * 60;
		deg.mm = (long)floor( dmin );
		
		// decimal seconds
		deg.sec = (dmin - deg.mm) * 60;
		
		// adjust degree sign
		deg.hh = deg.hh * sign;

		return deg;	
	}

	//! Transform a point from input system to output
	void coord_transformer::transform(double* x, double* y, double* z) {
		pj_transform((PJ *)(_proj_in->_proj), (PJ *)(_proj_out->_proj), 1, 0, 
						x, y, z);
	}
	void ConvMgr::Get(void* crd, Format_Type type, int ndec) { // ritorna il valore nella modalita indicata da type
		if ( type == grd || type == grm ) {
			char* val = (char*) crd;
			_formatSgs(val, type, ndec);
		} else {
			double* val = (double*) crd;
			if ( type == rad )
				*val = _rad;
			else if ( type == deg ) {
				*val = _rad * 180. / M_PI;
			} else if ( type == sec ) {
				*val = 3600. * _rad * 180. / M_PI;
			}
		}
	}
	void ConvMgr::Set(const double val, Format_Type type) {
		if ( type == rad )
			_rad = val;
		else if ( type == deg )
			_rad =  val * M_PI / 180;
		else if ( type == sec )
			_rad =  val * M_PI / (180. * 3600.);
	}
	bool ConvMgr::Set(const char* str, Format_Type type) {
		if ( type == rad )
			_rad = atof(str);
		else if ( type == deg )
			_rad =  atof(str) * M_PI / 180;
		else if ( type == sec )
			_rad = (atof(str) / 3600.) * M_PI / 180.;
		else if ( type == grd )
			return _set(str);
		else if ( type == grm ) {
			std::string val = str;
			size_t k1 = val.find_first_of('.');
			if ( k1 == std::string::npos )
				return false;
			double grad, min;
			if ( k1 == 4 ) {
				// latitudine
				grad = atof(val.substr(0, 2).c_str());
				min = atof(val.substr(2, val.size() - 2).c_str());
			} else if ( k1 == 5 ) {
				// longitudine
				grad = atof(val.substr(0, 3).c_str());
				min = atof(val.substr(3, val.size() - 3).c_str());
			} else
				return false;
			_rad = (grad + min / 60) * M_PI / 180;
		}
		return true;
	}
	bool ConvMgr::_set(const char* str) { // da stringa a radianti
		char mes[256];
		strcpy(mes, str);
		char* p1 = strtok(mes, "°, ");
		if ( p1 == NULL )
			return false;
		double deg = atoi(p1);
		p1 = strtok(NULL, "', ");
		if ( p1 == NULL )
			return false;
		deg += atof(p1) / 60.;
		p1 = strtok(NULL, "\"");
		if ( p1 != NULL ) {
			deg += atof(p1) / 3600.;
		}
		_rad =  deg * M_PI / 180;
		p1 = strtok(NULL, "\0");
		if ( p1 != NULL && (!stricmp(p1, "O") || !stricmp(p1, "W") || !stricmp(p1, "S")) )
			_rad = -_rad;
		return true;
	}
	void ConvMgr::_formatSgs(char* mes, Format_Type type, int ndec) { // formatta in stringa
		double deg = _rad * 180. / M_PI;
		int _grd = (int) floor(deg);
		if ( type == grm ) {
			double _min = (deg - _grd) * 60.;
			sprintf(mes, "%2d° %.*lf\'", _grd, ndec, _min);
		} else {
			deg = (deg - _grd) * 60;
			int _min = (int) floor(deg);
			double _sec = (deg - _min) * 60.;
			sprintf(mes, "%2d° %2d\' %.*lf\"", _grd, _min, ndec, _sec);
		}
	}
}


