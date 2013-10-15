/*===============================================================================
!   Filename:  Sun.H
!
!   Contents:	Class for sun height calculation
!
!   History:
!			11/06/2013  created
!			11/06/2013	updated
!=================================================================================*/

#include "sun.h"
#include <sstream>

void Sun::calc(int year, int m, int day, double h)
{
	double UT = h - _tzone - _dst;	// universal time
	double jd = _FNday((int) year, (int) m, (int) day, (float) UT);

	//   Use FNsun to find the ecliptic longitude of the Sun
	double lambda = _FNsun(jd);

	//   Obliquity of the ecliptic
	double obliq = 23.4393 * rads - 3.563E-7 * rads * jd;

	// Sidereal time at Greenwich meridian
	double GMST0 = L * degs / 15.0 + 12.0;	// hours
	double SIDTIME = GMST0 + UT + _longit / 15.0;

	// Hour Angle
	double ha = 15.0 * SIDTIME - RA;	// degrees
	ha = _FNrange(rads * ha);
	double x = cos(ha) * cos(delta);
	double y = sin(ha) * cos(delta);
	double z = sin(delta);
	double xhor = x * sin(_latit * rads) - z * cos(_latit * rads);
	double yhor = y;
	double zhor = x * cos(_latit * rads) + z * sin(_latit * rads);
	_azim = atan2(yhor, xhor) + pi;
	_azim = _FNrange(_azim);
	_altit = asin(zhor);

	// delta = asin(sin(obliq) * sin(lambda));
	double alpha = atan2(cos(obliq) * sin(lambda), cos(lambda));
	//   Find the Equation of Time in minutes
	double equation = 1440 - (L - alpha) * degs * 4;

	ha = _f0(_latit, delta);

	// Conversion of angle to hours and minutes
	_daylen = degs * ha / 7.5;
	if ( _daylen < 0.0001 ) {
		_daylen = 0.0;
	}
	
	// arctic winter
	_riset = 12.0 - 12.0 * ha / pi + _tzone + _dst - _longit / 15.0 + equation / 60.0;
	_settm = 12.0 + 12.0 * ha / pi + _tzone + _dst - _longit / 15.0 + equation / 60.0;
	double noont = _riset + 12.0 * ha / pi;
	_altmax = 90.0 + delta * degs - _latit; 
	if ( _altmax > 90.0 )
		_altmax = 180.0 - _altmax; //to express as degrees from the N horizon

	noont -= 24 * ( noont > 24 );

	if ( _riset > 24.0) 
		_riset -= 24.0;
	if ( _settm > 24.0) 
		_settm -= 24.0;
}

//   Get the days to J2000 h is UT in decimal hours only works between 1901 to 2099
double Sun::_FNday (int y, int m, int d, float h)
{
	int luku = - 7 * (y + (m + 9) / 12) / 4 + 275 * m / 9  + d;
	// type casting necessary on PC DOS and TClite to avoid overflow
	luku += (long) y * 367;
	return (double) luku - 730530.0 + h / 24.0;
}
//   returns an angle in the range 0 to 2*pi
double Sun::_FNrange(double x) 
{
    double b = x / tpi;
    double a = tpi * (b - (long) (b));
    if ( a < 0 ) 
		a = tpi + a;
    return a;
}
// Calculating the hour angle
double Sun::_f0(double lat, double declin) 
{
	double dfo = rads * (0.5 * SunDia + AirRefr);
	if ( lat < 0.0 ) 
		dfo = -dfo;	// Southern hemisphere
	double fo = tan(declin + dfo) * tan(lat * rads);
	if ( fo > 0.99999 ) 
		fo = 1.0; // to avoid overflow
	fo = asin(fo) + pi / 2.0;
	return fo;
}
//   Find the ecliptic longitude of the Sun
double Sun::_FNsun(double d)
{
	double w, M, v ,r;
	//   mean longitude of the Sun
	w = 282.9404 + 4.70935E-5 * d;
	M = 356.047 + 0.9856002585 * d;
	// Sun's mean longitude
	L = _FNrange(w * rads + M * rads);

	//   mean anomaly of the Sun
	g = _FNrange(M * rads);

	// eccentricity
	double ecc = 0.016709 - 1.151E-9 * d;

	//   Obliquity of the ecliptic
	double obliq = 23.4393 * rads - 3.563E-7 * rads * d;
	double E = M + degs * ecc * sin(g) * (1.0 + ecc * cos(g));
	E = degs * _FNrange(E * rads);
	double x = cos(E * rads) - ecc;
	double y = sin(E * rads) * sqrt(1.0 - ecc * ecc);
	r = sqrt(x * x + y * y);
	v = atan2(y, x) * degs;
	// longitude of sun
	double lonsun = v + w;
	lonsun -= 360.0 * (lonsun > 360.0);

	// sun's ecliptic rectangular coordinates
	x = r * cos(lonsun * rads);
	y = r * sin(lonsun * rads);
	double yequat = y * cos(obliq);
	double zequat = y * sin(obliq);
	RA = atan2(yequat, x);
	delta = atan2(zequat,sqrt(x * x + yequat * yequat));
	RA *= degs;

	//   Ecliptic longitude of the Sun

	return _FNrange(L + 1.915 * rads * sin(g) + .02 * rads * sin(2 * g));
}
// Display decimal hours in hours and minutes
std::string Sun::showhrmn(double dhr)
{
	int hr, mn;
	hr = (int) dhr;
	mn = (int) (0.5 + (dhr - (double) hr) * 60);
	std::stringstream str;
	if ( hr < 10 ) 
		str << '0';
	str << hr;
	str << ':';
	if ( mn < 10 ) 
		str << '0';
	str << mn;
	return str.str();
}
