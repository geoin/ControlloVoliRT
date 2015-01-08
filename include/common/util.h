#ifndef CV_COMMON_UTIL_H
#define CV_COMMON_UTIL_H

#include "docbook/docbook.h"
#include "CVUtil/ogrgeomptr.h"
#include "CVUtil/cvspatialite.h"
#include "photo_util/vdp.h"

enum CHECK_TYPE {
	less_ty = 0,
	great_ty = 1,
	abs_less_ty = 2,
	between_ty =3
};

#define STRIP_NAME "A_VOL_CS"
#define SRID 32632 // SRID UTM32 wgs84
#define SRIDGEO 4326 // SRID lat lon wgs84
#define GPS_TABLE_NAME "GPS"
#define Z_UNCOVER "Z_UNCOVER"
#define ASSI_VOLO "AVOLO"
#define Z_FOTO "Z_FOTO"
#define Z_MODEL "Z_MODEL"
#define Z_STRIP "Z_STRIP"
#define Z_STR_OVL "Z_STR_OVL"
#define DB_NAME "geo.sqlite"
#define Z_BLOCK "Z_BLOCK"
#define Z_CAMERA "Camera"

typedef std::vector<unsigned char> Blob;

bool print_item(Doc_Item& row, Poco::XML::AttributesImpl& attr, double val, CHECK_TYPE ty, double tol1, double tol2 = 0);
char* get_typestring(const CV::Util::Geometry::OGRGeomPtr geom);
void get_elong(const CV::Util::Geometry::OGRGeomPtr fv0, double ka, double* d1, double* d2);
void get_ellipse_elong(const CV::Util::Geometry::OGRGeomPtr fv0, double& d1, double& d2);
void get_ellipse_elong(const CV::Util::Geometry::OGRGeomPtr fv0, double& d1, double& d2, double& theta);
void add_column(CV::Util::Spatialite::Connection& cnn, const std::string& table, const std::string& col_name);
void init_document(docbook& dbook, const std::string& nome, const std::string& title, const std::string& note);
bool GetProjData(CV::Util::Spatialite::Connection& cnn, std::string& note, std::string& scale);
void read_cams(CV::Util::Spatialite::Connection& cnn, std::map<std::string, Camera>& map_strip_cam);
void read_planned_cam(CV::Util::Spatialite::Connection& cnn, Camera& cam);

std::string get_strip(const std::string& nome);
std::string get_nome(const std::string& nome);

#endif

