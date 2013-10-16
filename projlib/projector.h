#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <string>
#include <memory>

#ifndef M_PI
	#define M_PI       3.14159265358979323846
#endif

#ifdef WIN32
    #ifdef PROJECTOR_EXPORTS
        #define PROJECTOR_API __declspec(dllexport)
    #else
        #define PROJECTOR_API __declspec(dllimport)
    #endif
#else
    #define PROJECTOR_API
#endif

//#pragma warning(push)
//#pragma warning(disable:4251)

namespace Projection {
//! degree custom type
/**
*	represent a sessagesima degree
*/
struct PROJECTOR_API degree {
	//! hours
	int hh;
	//! minutes
	int mm;
	//! seconds
	double sec;
};

//! Execute direct and inverse projection of coordinates
class PROJECTOR_API projector {
	friend class coord_transformer;
public:
	enum wk_proj {
		WGS84 = 0,
		UTMED50_FUSE32 = 1,
		UTMED50_FUSE33 = 2,
		UTMED50_FUSE34 = 3,
		GAUSSBOAGA_ROME40_ZONE1 = 4,
		GAUSSBOAGA_ROME40_ZONE2 = 5
	};

	//! default constructor
	projector(): _proj(NULL) {}
	
	//! constructor
	/** 
	*	@param inistr: initialization string in proj format
	*/
	projector(std::string const &inistr): _proj(NULL) {
		init(inistr);
	}

	//! Initialize projector. Eventually clear previous configured projection
	/** 
	*	@param inistr: initialization string in proj format
	*/
	bool init(std::string const &inistr);

	void init_well_known(enum wk_proj wkp){
		init(_wk_proj[wkp]);
	}

	//! Execute direct projection.
	/** 
	*	@parameter pt: point with coordinates in decimal degrees
	*   @return    returns projected point in radiants
	*/
	void project_fwd(double*x, double*y, double* z) const {
		_project(x, y, z, true);
	}
	//! Execute inverse projection.
	/** 
	*	@parameter pt: point with coordinates in radiants
	*   @return    returns projected point in decimal degrees
	*/
	void project_inv(double* x, double* y, double* z) const {
		_project(x, y, z, false);
	}
	std::string const &init_string() const {
		return _ini_string;
	}

	//! convert from sessagesimal degree to radians
	static double deg_to_rad( degree const &deg );
	//! convert from radians to sessagesimal degree
	static degree rad_to_deg( double rad );
	void clear(void) {
		_clear();
	}
private:
	//! reset actual projection
	void _clear();
	
	void _project(double* x, double* y, double* z, bool isdirect) const;
	
	//! underline projection
	void *_proj; 
	//! current init string
	std::string _ini_string;

	static const double	RES60;
	static const double	CONV;
	static const double	CONV1;

	static char *_wk_proj[];
};

			//! Coordinate transformer.
			/**
			*	Trasform from an input coordinate system to an output one.
			*	Try to transform the datum too if configured
			*/
class PROJECTOR_API coord_transformer {
public:
	//! constructor
	/**
	*	projectors parameter are pointer and the class
	*	doesnt owns their lifecycles as object instances
	*/
	coord_transformer(projector const *pjin, projector const *pjout): 
	_proj_in(pjin), _proj_out(pjout) {}
	
	//! Transform a point from input system to output
	//Geoin::GES::point transform(Geoin::GES::point const &pt);
	void transform(double* x, double* y, double* z);
	
	//! get source projector
	projector const *src_proj() const {
		return _proj_in;
	}
	//! set source projector
	void src_proj(projector const *pj){
		_proj_in = pj;
	}

	//! get destination projector
	projector const *dst_proj() const {
		return _proj_out;
	}
	//! get destination projector
	void dst_proj(projector const *pj){
		_proj_out = pj;
	}
private:
	projector const *_proj_in;
	projector const *_proj_out;
};

class PROJECTOR_API ConvMgr {
public:
	enum Format_Type {
		rad = 0,	// radianti
		deg = 1,	// gradi sessadecimali binario
		sec = 2,	// secondi
		grd = 3,	// gradi, minuti secondi formato stringa
		grm = 4		// gradi, minuti formato stringa
	};
	ConvMgr(): _rad(0.) {} // costruttore di default

	ConvMgr(const ConvMgr& cv) { // costruttore di copia
		_rad = cv.rad;
	}
	ConvMgr(const char* mes) { // costruttore da stringa 
		_set(mes);
	}
	ConvMgr(const double val, Format_Type type) { // costruttore da valore numerico e formato
		Set(val, type);
	}
	ConvMgr(const char* val, Format_Type type) { // costruttore da stringa e formato
		Set(val, type);
	}
	void Get(void* crd, Format_Type type, int ndec = 5); // ritorna il valore nella modalita indicata da type
	double GetRad() {
		return _rad;
	}
	void Set(const double val, Format_Type type);
	bool Set(const char* str, Format_Type type);
private:
	double _rad;
	bool _set(const char* str);  // da stringa a radianti
	void _formatSgs(char* mes, Format_Type type, int ndec = 5);  // formatta in stringa
};

}
//#pragma warning(pop)

#endif
