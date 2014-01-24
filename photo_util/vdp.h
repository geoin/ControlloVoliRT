/*===============================================================================
!   Filename:  vdp.H
!
!   Contents:	Class for space resection data
!
!   History:
!			01/04/2004  created
!			27/03/2011	updated
!=================================================================================*/
#ifndef VDP_H
#define VDP_H

#include "exports.h"
#include "dem_interpolate/geom.h"
#include <string>
#include <map>

class TOOLS_EXPORTS Collimation {
public:
    Collimation(): xi(0), yi(0) {}
    Collimation(double x, double y): xi(x), yi(y) {}
    Collimation(const Collimation& c): xi(c.xi), yi(c.yi) {}
    double xi;
    double yi;
};

// classe definizione fotocamera
class TOOLS_EXPORTS Camera {
public:
	Camera(): foc(100.), xp(0.), yp(0.), dimx(0), dimy(0), dpix(0.), planning(true) {}
	Camera(double Foc, double dp, double dx, double dy): foc(Foc), dpix(dp), dimx(dx), dimy(dy), planning(true) {
		xp = dimx / 2.;
		yp = dimy / 2.;
	}
	Camera(double Foc, double dp, double dx, double dy, double Xp, double Yp): foc(Foc), dpix(dp), dimx(dx), dimy(dy), planning(true) {
		xp = dimx / 2. + Xp;
		yp = dimy / 2. + Yp;
	}

	double	foc;		// Focal lenght (mm)
	double	xp, yp;		// digital principal point offset respect center (mm)
	double	dpix;		// digital camera pixel size (mm)
	double	dimx, dimy;	// digital camera sensor size (mm)
	std::string id, serial, model, descr;
	bool planning;
private:
};		

// classe definizione orientamento interno. Trasformazione da coord. immagine (pixel) a coord fotografiche (mm) riferite al pp
class TOOLS_EXPORTS IOR {
public:
	IOR() {
		Reset();
	}
	// costruttore basato su una fotocamera
	IOR(const Camera& cam);
	~IOR() {
	}
	void	Init(const Camera& cam);
	// trasformazione da immagine a coord. fotografiche
    Collimation	ImgLastra(const Collimation& ci)const;
	// trasformazione da coord. fotografiche a immagine
    Collimation	LastraImg(const Collimation& cl) const;
	void	Reset(void);
	long	dimx(void) const {
		return (long) (_dimx);
	}
	long	dimy(void) const {
		return (long) (_dimy);
	}
	double foc(void) const {
		return _foc;
	}
	double pix(void) const {
		return _dpix;
	}
private:
	void	_inverte(void);

	double	_foc;					// focal length (mm)
	double	xc[3], yc[3], zc[2];	// interior parameters: trasformation from photo coord.(mm) to image coord. (pixel)
	double	xci[3], yci[3], zci[2];	// reverse transformation parameters from image to photo
	int		exec;					// orientation type
	double	_dimx, _dimy;			// image size in pixel
	double	_dpix;					// pixel size (mm)
};

class TOOLS_EXPORTS VDP {
public:
	VDP();
    VDP(const VDP& v);
	VDP(const Camera& cam, const std::string& Nome);
	~VDP();
	DPOINT	Pc;
	double	om, fi, ka;							// photo angles
	MatOri mat;
	int		exec;								// vdp execution flag
	std::string nome;

	void operator=(const VDP& vdp);
	void InitIor(const Camera& cam) {
		_ior.Init(cam);
	}
	void Init(const DPOINT& p, double pom, double pfi, double pka);
	bool IsValid(void) const {
		return exec != 0;
	}

	void Reset(void);
    Collimation	Img_Las(const Collimation& ci) const;
    Collimation Las_Img(const Collimation& cl) const;

	long	dimx(void) const {
		return _ior.dimx();
	}
	long	dimy(void) const {
		return _ior.dimy();
	}
	double foc(void) const {
		return _ior.foc();
	}
	double pix(void) const {
		return _ior.pix();
	}
    Collimation Ter_Img(const DPOINT& pt) const;
	// ritorna il vettore dal centro di presa verso il pixel xi, yi
    void GetRay(const Collimation& ci, DPOINT* pt) const;
    DPOINT Img_Ter(const Collimation& ci, double z) const;
    double Img_TerA(const VDP& _vdp_, const Collimation& ci1, const Collimation& ci2, DPOINT& pt) const;
private:
	IOR		_ior;
};

class TOOLS_EXPORTS VDPC: public VDP {
public:
	VDPC(): VDP() {}
    VDPC(const VDPC& v): VDP(v) {}
	VDPC(const Camera& cam, const std::string& Nome): VDP(cam, Nome) {}
	//~VDP();
    const Collimation& operator[](const std::string& cod) const {
        std::map<std::string, Collimation>::const_iterator it = cl.find(cod);
        return it->second;
    }
	Collimation& operator[](const std::string& cod) {
		return cl[cod];
	}
	std::map<std::string, Collimation> cl;
};
#endif
