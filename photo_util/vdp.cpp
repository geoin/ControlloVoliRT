/*=============================================================================
!   Filename:  ORI_VDP.CPP
!
!   Contents:	Space resection and DLT
!
!   History:
!			16/01/2004	created
!			01/11/2012	updated
!=============================================================================*/
#include "vdp.h"


VDP::VDP(): exec(0), om(0.), fi(0.), ka(0.)
{
}
VDP::VDP(const Camera& cam, const std::string& Nome): _ior(cam), nome(Nome)
{
}
VDP::~VDP()
{
}
void VDP::operator=(const VDP& vdp)
{
	Pc = vdp.Pc;
	om = vdp.om; fi = vdp.fi; ka = vdp.ka;							
	mat = vdp.mat;
	
	exec = vdp.exec;								
	_ior = vdp._ior;
}

void VDP::Reset()
{
	_ior.Reset();
	exec = 0;
	Pc.Set(0., 0., 0.);
	mat.Reset();
	om = fi = ka = 0.;
}
void VDP::Init(const DPOINT& p, double pom, double pfi, double pka)
{
	Pc = VecOri(p.x, p.y, p.z);
	om = DEG_RAD(pom);	// omega positivo antiorario
	fi = -DEG_RAD(pfi); // per convenzione internamente fi e ka sono positivi se orari
	ka = -DEG_RAD(pka);
	mat = MatOri(om, fi, ka);
}

/*----------------------------------------------------------------------------
!@  Ter_img
!@	From ground to image
!
!----------------------------------------------------------------------------*/
void VDP::Ter_Img(DPOINT *pt, float *x, float *y) const
{
	Ter_Img(pt->x, pt->y, pt->z, x, y);
}
void VDP::Ter_Img(double X, double Y, double Z, float *x, float *y) const
{
	// trasforma le coord (X, Y, Z) nel sistema fotocamera
	VecOri v = VecOri(X, Y, Z) - Pc;
	VecOri dp = mat * v;

	// proietta le coord sul piano del sensore
	double lsx = -_ior.foc() * dp[0] / dp[2];
	double lsy = -_ior.foc() * dp[1] / dp[2];
	// trasforma da coord fotografiche a coord immagine
	_ior.LastraImg(lsx, lsy, x, y);
}
void VDP::Img_Las(float xi, float yi, double *xl, double *yl)
{
	_ior.ImgLastra(xi, yi, xl, yl);
}
void VDP::Las_Img(double xl, double yl, float *xi, float *yi) const
{
	_ior.LastraImg(xl, yl, xi, yi);
}
// Calcola il vettore che proietta un punto immagine
void VDP::GetRay(float x, float y, DPOINT* pt)
{
	// da coord immagine a coord fotografiche
	double lx, ly;
	_ior.ImgLastra(x, y, &lx, &ly);

	// traspone (equivalente ad invertire) la matrice
	MatOri m = mat.Transpose(); // m è adesso dal sistema fotografico al sistema terreno
	VecOri v(lx, ly, -_ior.foc()); // coordinate fotografiche del punto immagine
	VecOri v1 = m * v; // coordinate del punto immagine nel sistema terreno
	// il centro di presa ha coordinate 0, 0, 0
	v1.Get(*pt); // assegna a pt il vettore v1
}
// da immagine a terreno, conoscendo la quota (Z) del punto
void VDP::Img_Ter(float x, float y, double* X, double* Y, double Z)
{
	// da immagine a coord. fotografiche
	double lsx, lsy;
	_ior.ImgLastra(x, y, &lsx, &lsy);
	
	VecOri v(lsx, lsy, -_ior.foc());
	MatOri m1 = mat.Transpose();
	VecOri v1 = m1 * v; // coord del punto fotografico nel sistema terreno
	double N = (Z - Pc.GetZ() ) / v1[2];
	*X = Pc.GetX() + N * v1[0];
	*Y = Pc.GetY() + N * v1[1];
}

/*----------------------------------------------------------------------------
!@  Img_TerAnp
!@	Image to ground, no parallax
!
!----------------------------------------------------------------------------*/
//double VDP::Img_Ternp(VDP& vdp2, float x1, float y1, float x2, float y2, DPOINT& pt)
//{
//	return Img_Ternp(vdp2, x1, y1, x2, y2, &pt.x, &pt.y, &pt.z);
//}
//double VDP::Img_Ternp(VDP& vdp2, float x1, float y1, float x2, float y2, double *X, double *Y, double *Z)
//{
//	float	xc, yc;
//
//	double lx1, ly1;
//	ior.ImgLastra(x1, y1, &lx1, &ly1);
//
//	double lx2, ly2;
//	vdp2.ior.ImgLastra(x2, y2, &lx2, &ly2);
//	
//	VecOri v1(lx1, ly1, -ior.foc);
//	MatOri m1 = mat.Transpose();
//	v1 = m1 * v1;
//
//	VecOri v2(lx2, ly2, -vdp2.ior.foc);
//	MatOri m2 = vdp2.mat.Transpose();
//	v2 = m2 * v2;
//
//	double tx1 = v1[0] / v1[2];
//	double ty1 = v1[1] / v1[2];
//	double tx2 = v2[0] / v2[2];
//	double ty2 = v2[1] / v2[2];
//
//	LinSys	lSys1;
//	lSys1.init(3);
//	lSys1.reset();
//
//	lSys1.SetPar(0, -1.);
//	lSys1.SetPar(1, 0.);
//	lSys1.SetPar(2, tx1);
//	lSys1.SetPar(3, tx1 * Zc - Xc);
//	lSys1.mtcno();
//	lSys1.SetPar(0, 0);
//	lSys1.SetPar(1, -1.);
//	lSys1.SetPar(2, ty1);
//	lSys1.SetPar(3, ty1 * Zc - Yc);
//	lSys1.mtcno();
//	lSys1.SetPar(0, -1.);
//	lSys1.SetPar(1, 0.);
//	lSys1.SetPar(2, tx2);
//	lSys1.SetPar(3, tx2 * vdp2.Zc - vdp2.Xc);
//	lSys1.mtcno();
//	lSys1.SetPar(0, 0);
//	lSys1.SetPar(1, -1.);
//	lSys1.SetPar(2, ty2);
//	lSys1.SetPar(3, ty2 * vdp2.Zc - vdp2.Yc);
//	lSys1.mtcno();
//	lSys1.Solve();
//	*X = lSys1.GetRes(0);
//	*Y = lSys1.GetRes(1);
//	*Z = lSys1.GetRes(2);
//
//	vdp2.Ter_Img(*X, *Y, *Z, &xc, &yc);
//	double Px = (double) (y2 - yc);
//	return Px;
//}


/*----------------------------------------------------------------------------
!@  Img_TerA
!@	From image to ground with residual parallax
!
!----------------------------------------------------------------------------*/
double VDP::Img_TerA(VDP& vdp2, float x1, float y1, float x2, float y2, double *X, double *Y, double *Z)
{
	float	xc, yc;

	double lx1, ly1;
	_ior.ImgLastra(x1, y1, &lx1, &ly1);
	double lx2, ly2;
	vdp2._ior.ImgLastra(x2, y2, &lx2, &ly2);

	VecOri v1(lx1, ly1, -_ior.foc());
	MatOri m1 = mat.Transpose();
	v1 = m1 * v1;

	VecOri v2(lx2, ly2, -vdp2._ior.foc());
	MatOri m2 = vdp2.mat.Transpose();
	v2 = m2 * v2;

		double tx1 = v1[0] / v1[2];
	double ty1 = v1[1] / v1[2];
	double tx2 = v2[0] / v2[2];

	// si cerca P=(X, Y, Z) intersezione tra le rette passanti per OL, e V1 e OR e V2
	// l'equazione della retta per OL V1 è: ( V1 è già nel sistema con origine in OL)
	//
	// X - XC1   V1x         Y - XC1   V1y
	// ------_ = --- = tx1 ; ------_ = --- = ty2
	// Z - ZC1   V1z         Z - ZC1   V1z
	//
	// cui va aggiunta una delle due equazioni della retta per OR e V2 ( V2 è già nel sistema con origine in OR)
	//
	// X - XC2   V2x
	// ------_ = --- = tx2
	// Z - ZC2   V2z

	// la cui soluzione è:
	
	*Z = (Pc.GetX() - vdp2.Pc.GetX() + tx2 * vdp2.Pc.GetZ() - tx1 * Pc.GetZ()) / (tx2 - tx1);
	*X = Pc.GetX() + tx1 * (*Z - Pc.GetZ());
	*Y = Pc.GetY() + ty1 * (*Z - Pc.GetZ());

	// ritrasforma il punto sul secondo fotogramma per calcolare l'eventuale parallasse
	vdp2.Ter_Img(*X, *Y, *Z, &xc, &yc);
	
	double Px = (double) (y2 - yc); // parallasse
	return Px; 
}


/*******************************************************************************/
IOR::IOR(const Camera& cam)
{
	Init(cam);
}
void IOR::Init(const Camera& camera)
{
	_dimx = camera.dimx / camera.dpix; // dimensione del sensore in pixel
	_dimy = camera.dimy / camera.dpix;

	_dpix = camera.dpix;
	_foc = camera.foc;

	xc[0] = camera.xp / camera.dpix;
	xc[1] = 1 / _dpix;
	xc[2] = 0.;
	yc[0] = camera.yp / camera.dpix;
	yc[1] = 0.;
	yc[2] = -1 / _dpix;
	zc[0] = 0.;
	zc[1] = 0.;
	exec = 1;
	_inverte();
}
void IOR::Reset()
{
	xc[0] = 0.; xc[1] = 1.; xc[2] = 0.;
	yc[0] = 0.; yc[1] = 0.; yc[2] = 1.;
	zc[0] = zc[1] = 0.;

	xci[0] = 0.; xci[1] = 1.; xci[2] = 0.;
	yci[0] = 0.; yci[1] = 0.; yci[2] = 1.;
	zci[0] = zci[1] = 0.;
	_foc = 100.;

	exec = 0;
	_dimx = _dimy = 0;
	_dpix = 1.;	
}
void IOR::_inverte()
{
	double den = xc[1] * yc[2] - xc[2] * yc[1];
	 
	yci[0] = (yc[1] * xc[0] - xc[1] * yc[0]) / den;
	yci[1] = (zc[0] * yc[0] - yc[1]) / den;
	yci[2] = (xc[1] - zc[0] * xc[0]) / den;
	xci[0] = (xc[2] * yc[0] - yc[2] * xc[0]) / den;
	xci[1] = (yc[2] - zc[1] * yc[0]) / den;
	xci[2] = (zc[1] * xc[0] - xc[2]) / den;
	zci[0] = (zc[1] * yc[1] - zc[0] * yc[2]) / den;
	zci[1] = (xc[2] * zc[0] - xc[1] * zc[1]) / den;
}
void IOR::ImgLastra(float xi, float yi, double *xl, double *yl)
{
	double den = zci[0] * xi + zci[1] * yi + 1.;
	*xl = (xci[0] + xci[1] * xi + xci[2] * yi) / den;
	*yl = (yci[0] + yci[1] * xi + yci[2] * yi) / den;
}
/*----------------------------------------------------------------------------
!@  LastraImg
!@	Plate to image
!
!----------------------------------------------------------------------------*/
void IOR::LastraImg(double xl, double yl, float *xi, float *yi) const
{
	double den = zc[0] * xl + zc[1] * yl + 1.;
	*xi = (float) ((xc[0] + xc[1] * xl + xc[2] * yl) / den);
	*yi = (float) ((yc[0] + yc[1] * xl + yc[2] * yl) / den);
}

