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


// classe definizione fotocamera
class TOOLS_EXPORTS Camera {
public:
	Camera(): foc(100.), xp(0.), yp(0.), dimx(0), dimy(0), dpix(0.) {}
	Camera(double Foc, double dp, double dx, double dy): foc(Foc), dpix(dp), dimx(dx), dimy(dy) {
		xp = dimx / 2.;
		yp = dimy / 2.;
	}
	Camera(double Foc, double dp, double dx, double dy, double Xp, double Yp): foc(Foc), dpix(dp), dimx(dx), dimy(dy) {
		xp = dimx / 2. + Xp;
		yp = dimy / 2. + Yp;
	}

	double	foc;		// Focal lenght (mm)
	double	xp, yp;		// digital principal point offset respect center (mm)
	double	dpix;		// digital camera pixel size (mm)
	double	dimx, dimy;	// digital camera sensor size (mm)
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
	void	ImgLastra(float xi, float yi, double *xl, double *yl);
	// trasformazione da coord. fotografiche a immagine
	void	LastraImg(double xl, double yl, float *xi, float *yi) const;
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
	VecOri	Pc;
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
	void	Img_Las(float xi, float yi, double *xl, double *yl);
	void	Las_Img(double xl, double yl, float *xi, float *yi) const;

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
	void Ter_Img(double X, double Y, double Z, float *x, float *y) const;
	void Ter_Img(DPOINT *pt, float *x, float *y) const;
	// ritorna il vettore dal centro di presa verso il pixel xi, yi
	void GetRay(float x, float y, DPOINT* pt);
	void Img_Ter(float x, float y, double* X, double* Y, double Z);
	//double Img_Ternp(VDP& _vdp_, float xl, float yl, float xr, float yr, double* X, double* Y, double *Z);
	//double Img_Ternp(VDP& _vdp_, float x1, float y1, float x2, float y2, DPOINT& pt);
	double Img_TerA(VDP& _vdp_, float x1, float y1, float x2, float y2, double *X, double *Y, double *Z);
private:
	IOR		_ior;
};
class TOOLS_EXPORTS Collimation {
public:
	Collimation(): xi(0), yi(0) {}
	Collimation(float x, float y): xi(x), yi(y) {}
	Collimation(const Collimation& c): xi(c.xi), yi(c.yi) {}
	float xi;
	float yi;
};

class TOOLS_EXPORTS VDPC: public VDP {
public:
	VDPC(): VDP() {}
    VDPC(const VDPC& v): VDP(v) {}
	VDPC(const Camera& cam, const std::string& Nome): VDP(cam, Nome) {}
	//~VDP();
    //Collimation& operator[](const std::string& cod) const {
      //  return cl[cod];
    //}
	Collimation& operator[](const std::string& cod) {
		return cl[cod];
	}
	std::map<std::string, Collimation> cl;
};
#endif
