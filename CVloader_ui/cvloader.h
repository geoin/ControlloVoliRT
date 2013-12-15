/*
    File: cvloader.h
    Author:  A.Comparini
    Date: 2013 November 03
    Comment:
        Load files and data for cartography aerial missions to be processed for suitable precision
        and reports creation

    Disclaimer:
        This file is part of CV a framework for verifying aerial missions for cartography.

        CV is free software: you can redistribute it and/or modify
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

#ifndef CVLOADER_H
#define CVLOADER_H

#include <QString>

#define PLANNED_FLIGHT_LAYER_NAME "AVOLOP"
#define FLIGHT_LAYER_NAME "AVOLOV"
#define QUADRO_LAYER_NAME "Quadro_RT"
#define CARTO_LAYER_NAME "carto"
#define CAMERA_FILE "camera.xml"
#define DEM_FILE "dem.asc"
#define ASSETTI_FILE "assettiv.txt"
#define PLANNED_ASSETTI_FILE "assettip.txt"
#define SHAPE_CHAR_SET "CP1252"
#define UTM32_SRID  32632
#define GEOM_COL_NAME "geom"
#define MAX_MSG_LEN 250
#define GEO_DB_NAME "geo.sqlite"
#define CONTORNO_RT "CONTORNO_RT"

class loader {
public:
	loader(): _replace(true){}
	bool load_flight_lines(const QString& nome);
	bool load_planned_flight_lines(const QString& nome);
	bool load_assetti(const QString& nome);
	bool load_planned_assetti(const QString& nome);
	bool load_camera(const QString& nome);
	bool load_carto(const QString& nome);
	bool create_project(void);
	bool load_dem(const QString& nome);
	bool load_quadro(const QString& nome);
	bool load_contorno(const QString& nome);

	bool load_mission(const QString& nome, const QString& mis_name);
	bool load_base(const QString& nome, const QString& mis_name, const QString& base_name);

	void prj_folder(const QString& nome) { _prj_folder = nome; }
private:
	bool _copy_file(const QString& source, const QString& dest) ;
	bool _load_layer(const QString& file_name, const QString& layer_name);

	QString _prj_folder;
	bool _replace;
};
#endif // CVLOADER_H
