/*
    File: vdp.cpp
    Author:  F.Flamigni
    Date: 2013 October 15
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

#include "vdp.h"


VDP::VDP(): exec(0), om(0.), fi(0.), ka(0.)
{
}
VDP::VDP(const Camera& cam, const std::string& Nome): _ior(cam), nome(Nome), exec(0), om(0.), fi(0.), ka(0.)
{
}
VDP::VDP(const VDP& v): _ior(v._ior), nome(v.nome), exec(0), om(0.), fi(0.), ka(0.)
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
    nome = vdp.nome;
	
	exec = vdp.exec;								
	_ior = vdp._ior;
}

void VDP::Reset()
{
	_ior.Reset();
	exec = 0;
	Pc.set(0., 0., 0.);
	mat.Reset();
	om = fi = ka = 0.;
}
void VDP::Init(const DPOINT& p, double pom, double pfi, double pka)
{
	Pc = p;//VecOri(p.x, p.y, p.z);
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
Collimation VDP::Ter_Img(const DPOINT& pt) const
{
	// trasforma le coord (X, Y, Z) nel sistema fotocamera
    //VecOri v(pt);
	DPOINT dp = mat * pt;

	// proietta le coord sul piano del sensore
    Collimation cl(-_ior.foc() * dp[0] / dp[2], -_ior.foc() * dp[1] / dp[2]);
	// trasforma da coord fotografiche a coord immagine
    return _ior.LastraImg(cl);
}
Collimation VDP::Img_Las(const Collimation& ci) const
{
    return _ior.ImgLastra(ci);
}
Collimation VDP::Las_Img(const Collimation& cl) const
{
    return _ior.LastraImg(cl);
}
// Calcola il vettore che proietta un punto immagine
void VDP::GetRay(const Collimation& ci, DPOINT* pt) const
{
	// da coord immagine a coord fotografiche
    Collimation cl = _ior.ImgLastra(ci);

	// traspone (equivalente ad invertire) la matrice
	MatOri m = mat.Transpose(); // m è adesso dal sistema fotografico al sistema terreno
    VecOri v(cl.xi, cl.yi, -_ior.foc()); // coordinate fotografiche del punto immagine
	VecOri v1 = m * v; // coordinate del punto immagine nel sistema terreno
	// il centro di presa ha coordinate 0, 0, 0
	v1.Get(*pt); // assegna a pt il vettore v1
}
// da immagine a terreno, conoscendo la quota (Z) del punto
DPOINT VDP::Img_Ter(const Collimation& ci, double Z) const
{
	// da immagine a coord. fotografiche
    Collimation cl = _ior.ImgLastra(ci);
	
    VecOri v(cl.xi, cl.yi, -_ior.foc());
	MatOri m1 = mat.Transpose();
	VecOri v1 = m1 * v; // coord del punto fotografico nel sistema terreno
	double N = (Z - Pc.z ) / v1[2];
    DPOINT p(Pc.x + N * v1[0], Pc.y + N * v1[1], Z);
    return p;
}


/*----------------------------------------------------------------------------
!@  Img_TerA
!@	From image to ground with residual parallax
!
!----------------------------------------------------------------------------*/
double VDP::Img_TerA(const VDP& vdp2, const Collimation& ci1, const Collimation& ci2, DPOINT& pt) const
{
    Collimation cl1 = _ior.ImgLastra(ci1);
    Collimation cl2 = vdp2._ior.ImgLastra(ci2);

    VecOri v1(cl1.xi, cl1.yi, -_ior.foc());
	MatOri m1 = mat.Transpose();
	v1 = m1 * v1;

    VecOri v2(cl2.xi, cl2.yi, -vdp2._ior.foc());
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
	
    pt.z = (Pc.x - vdp2.Pc.x + tx2 * vdp2.Pc.z - tx1 * Pc.z) / (tx2 - tx1);
    pt.x = Pc.x + tx1 * (pt.z - Pc.z);
    pt.y = Pc.y + ty1 * (pt.z - Pc.z);

	// ritrasforma il punto sul secondo fotogramma per calcolare l'eventuale parallasse
    Collimation cc = vdp2.Ter_Img(pt);
	
    double Px = (double) (ci2.yi - cc.yi); // parallasse
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

	xc[0] = _dimx / 2 + camera.xp / camera.dpix;
	xc[1] = 1 / _dpix;
	xc[2] = 0.;
	yc[0] = _dimy / 2 + camera.yp / camera.dpix;
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
Collimation IOR::ImgLastra(const Collimation& ci) const
{
    double den = zci[0] * ci.xi + zci[1] * ci.yi + 1.;
    Collimation cl;
    cl.xi = (xci[0] + xci[1] * ci.xi + xci[2] * ci.yi) / den;
    cl.yi = (yci[0] + yci[1] * ci.xi + yci[2] * ci.yi) / den;
    return cl;
}
/*----------------------------------------------------------------------------
!@  LastraImg
!@	Plate to image
!
!----------------------------------------------------------------------------*/
Collimation IOR::LastraImg(const Collimation& cl) const
{
    double den = zc[0] * cl.xi + zc[1] * cl.yi + 1.;
    Collimation ci;
    ci.xi = (float) ((xc[0] + xc[1] * cl.xi + xc[2] * cl.yi) / den);
    ci.yi = (float) ((yc[0] + yc[1] * cl.xi + yc[2] * cl.yi) / den);
    return ci;
}

