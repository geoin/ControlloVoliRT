/*===============================================================================
!   Filename:  Sun.H
!
!   Contents:	Class for sun height calculation
!
!   History:
!			11/06/2013  created
!			11/06/2013	updated
!=================================================================================*/
#ifndef SUN_H
#define SUN_H

#include "exports.h"
#include <string>
#define  _USE_MATH_DEFINES
#include <math.h>

class TOOLS_EXPORTS Sun {
public:
	Sun(): _latit(0.), _longit(0.), _tzone(0.), _dst(0) {
		_init();
	}
	Sun(double tzone, bool dst): _latit(0.), _longit(0.), _tzone(tzone) {
		_dst = dst ? 1 : 0;
		_init();
	}
	Sun(double lat, double lon, double tzone = 0., bool dst = false): _latit(lat), _longit(lon), _tzone(tzone) {
		_dst = dst ? 1 : 0;
		_init();
	}
	void calc(double lat, double lon, int year, int m, int day, double h) {
		_latit = lat;
		_longit = lon;
		calc(year, m, day, h);
	}
	void calc(int year, int m, int day, double h);
	std::string showhrmn(double dhr);
	// durata del giorno
	double daylen(void) const {
		return _daylen;
	}
	// ora a cui sorge
	double sunrise(void) const {
		return _riset;
	}
	// ora a cui tramonta
	double sunset(void) const {
		return _settm;
	}
	// altezza massima sull'orizzonte
	double altmax(void) const {
		return _altmax;
	}
	// azimut ora impostata
	double azim(void) const {
		return _azim * degs;
	}
	// altezza ora impostata
	double altit(void) const {
		return _altit * degs;
	}
private:
	void _init(void) {
		pi = M_PI;
		tpi = 2 * pi;
		degs = 180.0 / pi;
		rads = pi / 180.0;
		SunDia = 0.53;    
		AirRefr = 34.0/60.0;
	}
	double _FNrange(double x);
	double _FNday (int y, int m, int d, float h);
	double _FNsun(double d);
	double _f0(double lat, double declin);
	double pi;
	double tpi;
	double degs;
	double rads;
	double SunDia;     // Sun radius degrees
	double AirRefr; // athmospheric refraction degrees
	double L, RA, g, delta;
	double _daylen;
	double _latit;
	double _longit;
	double _tzone;
	double _riset;
	double _settm;
	double _altmax;
	double _azim;
	double _altit;
	int	   _dst;
};
#endif
