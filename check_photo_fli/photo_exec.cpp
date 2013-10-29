/*
    File: photo_exec.cpp
    Author:  F.Flamigni
    Date: 2013 October 29
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
#include "check_photo.h"
#include "photo_util/dsm.h"
#include "Poco/Util/XMLConfiguration.h"
#include "Poco/stringtokenizer.h"
#include "Poco/AutoPtr.h"
#include "Poco/sharedPtr.h"
#include <fstream>

#include <QGis-Lisboa/core/qgsproviderregistry.h>
#include <QGis-Lisboa/core/qgsvectorlayer.h>
#include "QGis-Lisboa/core/qgsgeometry.h"
#include <QGis-Lisboa/core/qgsvectorfilewriter.h>

using Poco::Util::XMLConfiguration;
using Poco::AutoPtr;
using Poco::SharedPtr;

photo_exec::~photo_exec() 
{
	if ( _df != NULL )
		delete _df;
}

bool photo_exec::_read_cam()
{
	AutoPtr<XMLConfiguration> pConf;
	try {
		pConf = new XMLConfiguration(_cam_name);
		_cam.foc = atof(pConf->getString("FOC").c_str());
		_cam.dimx = atof(pConf->getString("DIMX").c_str());
		_cam.dimy = atof(pConf->getString("DIMY").c_str());
		_cam.dpix = atof(pConf->getString("DPIX").c_str());
		_cam.xp = atof(pConf->getString("XP", "0").c_str());
		_cam.yp = atof(pConf->getString("YP", "0").c_str());
	} catch (...) {
		return false;
	}
	return true;
}
bool photo_exec::_read_vdp()
{
	std::ifstream fvdp(_vdp_name.c_str(), std::ifstream::in);
	if ( !fvdp.is_open() )
		return false;

	char buf[256];
	while ( fvdp.getline(buf, 256) ) {
		Poco::StringTokenizer tok(buf, " \t", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
		if ( tok.count() != 7 )
			continue;
		if ( atof(tok[1].c_str()) == 0. )
			continue;
		VDP vdp(_cam, tok[0]);
		vdp.Init(DPOINT(atof(tok[1].c_str()), atof(tok[2].c_str()), atof(tok[3].c_str())), atof(tok[4].c_str()), atof(tok[5].c_str()), atof(tok[6].c_str()));
		_vdps[tok[0]] = vdp;
	}
	return true;

}
bool photo_exec::_read_dem()
{
	_df = new DSM_Factory;
	if ( !_df->Open(_dem_name, false) )
		return false;
	DSM* ds = _df->GetDsm();
	unsigned int n = ds->Npt();
	return true;
}
std::string photo_exec::_get_strip(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() != 2 )
		return "";
	return tok[0];
}
std::string photo_exec::_get_nome(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() != 2 )
		return "";
	return tok[1];
}
void photo_exec::set_vdp_name(const std::string& nome)
{
	_vdp_name = nome;
}
void photo_exec::set_dem_name(const std::string& nome)
{
	_dem_name = nome;
}
void photo_exec::set_cam_name(const std::string& nome)
{
	_cam_name = nome;
}
void photo_exec::set_out_folder(const std::string& nome)
{
	_out_folder = nome;
}
//void photo_exec::_get_side(QgsGeometry* fv, double* d1, double* d2) 
//{
//	fv->
//	if ( !fv.IsChiusaP() )
//		return;
//	int np = fv.GetCount();
//	int idx = -1;
//	for (int i = 0; i < np - 1; i++) {
//		int j = i + 1;
//		int k = i == 0 ? np - 2 : i - 1;
//		double alfa = fabs(angdir(fv[j].p[0], fv[j].p[1], fv[i].p[0], fv[i].p[1]) - angdir(fv[i].p[0], fv[i].p[1], fv[k].p[0], fv[k].p[1]));
//		if ( fabs(alfa) > M_PI / 4 ) {
//			idx = i;
//			break;
//		}
//	}
//	FVDATA fvp;
//	if ( idx != 0 ) {
//		int kk = idx;
//		do {
//			fvp.Add(&fv[kk++]);
//			if ( kk >= np -1 ) 
//				kk = 0;
//			if ( kk == idx )
//				fvp.Add(&fv[kk]);
//		} while ( kk != idx );
//	} else
//		fvp = fv;
//
//	double dmax = -1.e30;
//	double dmin = 1.e30;
//	double dl = 0.;
//	for (int i = 1; i < fvp.GetCount(); i++) {
//		double d = fvp[i].dist2D(fvp[i - 1]);
//		if ( dl == 0. )
//			dl += d;
//		else {
//			double alfa = fabs(angdir(fvp[i].p[0], fvp[i].p[1], fvp[i - 1].p[0], fvp[i - 1].p[1]) - angdir(fvp[i - 1].p[0], fvp[i - 1].p[1], fvp[i - 2].p[0], fvp[i - 2].p[1]));
//			if ( fabs(alfa) < M_PI / 4 )
//				dl += d;
//			else {
//				dmin = min(dmin, dl);
//				dmax = max(dmax, dl);
//				dl = d;
//			}
//		}
//	}
//	if ( dl != 0. ) {
//		dmin = min(dmin, dl);
//		dmax = max(dmax, dl);
//	}
//	*d1 = dmin;
//	*d2 = dmax;
//}
bool photo_exec::_process_photos()
{
	DSM* ds = _df->GetDsm();

	QgsFieldMap fields;
	fields[0] = QgsField("STRIP", QVariant::String);
	fields[1] = QgsField("NOME", QVariant::String);
	fields[2] = QgsField("DIMPIX", QVariant::Double);

	Poco::Path pth(_out_folder, "Foto.shp");
	QgsVectorFileWriter writer(pth.toString().c_str(), "CP1250", fields, QGis::WKBPolygon, 0, "ESRI Shapefile");

	QgsFeature fet;

	std::map<std::string, VDP>::iterator it;
	for (it = _vdps.begin(); it != _vdps.end(); it++) {
		VDP& vdp = it->second;
		fet.addAttribute(0, QVariant(_get_strip(it->first).c_str()));
		fet.addAttribute(1, QVariant(_get_nome(it->first).c_str()));
		QgsPolygon polig;
		QgsPolyline line;
		double dt = 0.;

		DPOINT Pc(vdp.Pc.GetX()	, vdp.Pc.GetY(), vdp.Pc.GetZ());
		for ( int i = 0; i < 5; i++) {
			float x = ( i == 0 || i == 3 ) ? 0 : (i != 4) ? (float) vdp.dimx() : (float) vdp.dimx() / 2.;
			float y = ( i == 0 || i == 1 ) ? 0 : (i != 4) ? (float) vdp.dimy() : (float) vdp.dimy() / 2.;
			DPOINT pd, pt;
			vdp.GetRay(x, y, &pd);
			if ( !ds->RayIntersect(Pc, pd, pt) ) {
				if ( !ds->IsInside(pt.z) ) {
					break;
				}
			}
			dt += vdp.Pc.GetZ() - pt.z;
			if ( i != 4 )
				line.push_back(QgsPoint(pt.x, pt.y));
		}
		//calcola del GSD medio
		dt = vdp.pix() * dt / (5 * vdp.foc());
		fet.addAttribute(2, QVariant(dt));
		polig.push_back(line);
		fet.setGeometry(QgsGeometry::fromPolygon(polig));
		_vfoto.push_back(fet);
		writer.addFeature(fet);
	}
	_process_models();
	_process_strips();
	return true;
}
// costruisce i modelli a partire da fotogrammi consecutivi della stessa strisciata
bool photo_exec::_process_models()
{
	QgsFieldMap fields;
	fields[0] = QgsField("STRIP", QVariant::String);
	fields[1] = QgsField("NOME_LEFT", QVariant::String);
	fields[2] = QgsField("NOME_RIGHT", QVariant::String);

	Poco::Path pth(_out_folder, "Models.shp");
	QgsVectorFileWriter writer(pth.toString().c_str(), "CP1250", fields, QGis::WKBPolygon, 0, "ESRI Shapefile");

	std::string str0;
	//SharedPtr<QgsGeometry> g0;
	QgsGeometry* g0 = NULL;
	std::string f0, f1;
	// scandisce tutti i fotogrammi
	for (size_t i = 0; i < _vfoto.size(); i++) {
		QgsFeature& fet = _vfoto[i];
		//SharedPtr<QgsGeometry> g1(fet.geometry());
		QgsGeometry* g1 = fet.geometry();
		const QgsAttributeMap& am = fet.attributeMap();
		std::string str = am[0].toByteArray();
		std::string f1 = am[1].toByteArray();
		if ( str != str0 ) {
			// inizia una nuova strip
			str0 = str;
		} else {
			QgsFeature ft;
			// fa l'intersezione del modello attuale (g1) col precedente (g0)
			ft.setGeometry(g1->intersection(g0));
			ft.addAttribute(0, QVariant(str0.c_str()));
			ft.addAttribute(1, QVariant(f0.c_str()));
			ft.addAttribute(2, QVariant(f1.c_str()));
			writer.addFeature(ft);
		}
		g0 = g1;
		f0 = f1;
	}
	return true;
}
// costruisce le strisciate unendo tutte le foto di una stessa strip
bool photo_exec::_process_strips()
{
	QgsFieldMap fields;
	fields[0] = QgsField("STRIP", QVariant::String);
	fields[1] = QgsField("N_IMGS", QVariant::Int);

	Poco::Path pth(_out_folder, "Strips.shp");
	QgsVectorFileWriter writer(pth.toString().c_str(), "CP1250", fields, QGis::WKBPolygon, 0, "ESRI Shapefile");

	std::string str0;
	int nimg = 0;

	//SharedPtr<QgsGeometry> g0;
	QgsGeometry* g0;
	for (size_t i = 0; i < _vfoto.size(); i++) {
		QgsFeature& fet = _vfoto[i];
		//SharedPtr<QgsGeometry> g1 = fet.geometry();
		QgsGeometry* g1 = fet.geometry();
		const QgsAttributeMap& am = fet.attributeMap();
		std::string str = am[0].toByteArray();
		if ( str != str0 ) {
			if ( !str0.empty() ) {
				QgsFeature ft;
				ft.setGeometry(g0);
				ft.addAttribute(0, QVariant(str0.c_str()));
				ft.addAttribute(1, QVariant(nimg));
				writer.addFeature(ft);
			}
			str0 = str;
			nimg = 0;
			g0 = g1;
		} else {
			g0 = g1->combine(g0);
		}
		nimg++;
	}
	if ( nimg ) {
		QgsFeature ft;
		ft.setGeometry(g0);
		ft.addAttribute(0, QVariant(str0.c_str()));
		ft.addAttribute(1, QVariant(nimg));
		writer.addFeature(ft);
	}
	return true;

}
bool photo_exec::run()
{
	QgsProviderRegistry::instance("C:\\OSGeo4W\\apps\\qgis\\plugins");

	if ( !_read_cam() )
		return false;
	if ( !_read_vdp() )
		return false;
	if ( !_read_dem() )
		return false;
	if ( !_process_photos() )
		return false;
	return true;
}
