/*=============================================================================
!   Filename:  PROJ_HELPER.H
!
!   Contents:	ProjTransform class declaration
!
!   History:
!			10/08/2005	created
!			13/06/2013	updated
!=============================================================================*/
#ifndef PROJ_HELPER_H
#define PROJ_HELPER_H

#include <math.h>
#include "projector.h"
#include "boost/shared_ptr.hpp"
using namespace Projection;

#define _wgs84_ "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs" // 4326
#define _geocent_ "+proj=geocent +ellps=WGS84 +datum=WGS84 +no_defs" // 4978
#define _utm_ "+proj=utm +ellps=WGS84 +zone=%d +datum=WGS84 +units=m +no_defs" //326%d

//char *ed50 = "+proj=longlat +ellps=intl +towgs84=-87,-98,-121,0,0,0,0 +no_defs";
//char *Rome40 = "+proj=longlat +ellps=intl +towgs84=-225,-65,9,0,0,0,0 +no_defs";

//char const *gb_roma40[] = {
//	"+proj=tmerc +lat_0=0 +lon_0=9 +k=0.999600 +x_0=1500000 +y_0=0 +ellps=intl +towgs84=-225,-65,9,0,0,0,0 +units=m +no_defs",
//	"+proj=tmerc +lat_0=0 +lon_0=15 +k=0.999600 +x_0=2520000 +y_0=0 +ellps=intl +towgs84=-225,-65,9,0,0,0,0 +units=m +no_defs",
//	"+proj=tmerc +lat_0=0 +lon_0=9 +k=0.999600 +x_0=1500000 +y_0=0 +ellps=intl +towgs84=%lf,%lf,%lf,%lf,%lf,%lf,%lf +units=m +no_defs",
//	"+proj=tmerc +lat_0=0 +lon_0=15 +k=0.999600 +x_0=2520000 +y_0=0 +ellps=intl +towgs84=%lf,%lf,%lf,%lf,%lf,%lf,%lf +units=m +no_defs"
//};

#if defined(__cplusplus)

// generic transformation
class ProjTransform {
public:
	ProjTransform(): latin(false), latout(false) {
		//cdt = NULL;
		//cdtr = NULL;
		//latin = false;
		//latout = false;
	}
	ProjTransform(std::string& in, std::string& out) {
		_init(in, out);
	}
	virtual ~ProjTransform() {
		_destroy();
	}
	virtual void transform(double* inx, double *iny, double* inz = NULL) {
		cdt->transform(inx, iny, inz);
	}
	virtual void Rtransform(double* inx, double* iny, double* inz = NULL) {
		cdtr->transform(inx, iny, inz);
	}
	std::string const &GetInString() const {
		return pin.init_string();
	}
	std::string const &GetOutString() const {
		return pout.init_string();
	}
	bool latin, latout;
protected:
	projector pin, pout;
	void _destroy() {
		/*if ( cdt != NULL )
			delete cdt;
		cdt = NULL;
		if ( cdtr != NULL )
			delete cdtr;
		cdtr = NULL;*/
		pin.clear();
		pout.clear();
	}
	void _init(std::string& in, std::string& out) {
		if ( !pin.init(in) ) 
			throw(std::exception("input Transformation error"));
		if ( !pout.init(out) ) 
			throw(std::exception("output Transformation error"));
		cdt.reset(new coord_transformer(&pin, &pout));
		cdtr.reset(new coord_transformer(&pout, &pin));
		latout = ( strstr(out.c_str(), "longlat") ) ? true : false;
		latin = ( strstr(in.c_str(), "longlat") ) ? true : false;
	}
	boost::shared_ptr<coord_transformer> cdt;
	boost::shared_ptr<coord_transformer> cdtr;
};
#endif // __cplusplus

#endif
