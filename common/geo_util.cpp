#include "common/util.h"
#include "Poco/StringTokenizer.h"

#include <dem_interpolate/pslg.h>

using namespace CV::Util::Geometry;
using namespace CV::Util::Spatialite;

char* get_typestring(const OGRGeomPtr geom) 
{
	OGRwkbGeometryType ty = geom->getGeometryType();
	switch ( ty ) {
		case wkbPoint:
			return "POINT";
		case wkbLineString:
			return "LINESTRING";
		case wkbPolygon:
			return "POLYGON";
		case wkbMultiPoint:
			return "MULTIPOINT";
		case wkbMultiLineString:
			return "MULTILINESTRING";
		case wkbMultiPolygon:
			return "MULTIPOLYGON";
		case wkbGeometryCollection:
			return "GEOMETRYCOLLECTION";
		default:
			return "GEOMETRY";
	}
	return "GEOMETRY";
}

// get minor and major axis of a pseudo rectangular feature
void get_elong(OGRGeomPtr fv0, double ka, double* d1, double* d2)
{
	OGRPoint po;
	if ( fv0->Centroid(&po) != OGRERR_NONE )
		return;
	OGRGeometry* fv = fv0;
    OGRLinearRing* geom = ((OGRPolygon*) fv)->getExteriorRing();
	double xm = 1.e20, ym = 1.e20;
	double xM = -1.e20, yM = -1.e20;

    for (int i = 0; i < geom->getNumPoints(); i++) {
        double x = geom->getX(i) - po.getX();
        double y = geom->getY(i) - po.getY();
		double x1 = x * cos(ka) + y * sin(ka);
		double y1 = -x * sin(ka) + y * cos(ka);
		xm = std::min(xm, x1);
		ym = std::min(ym, y1);
		xM = std::max(xM, x1);
		yM = std::max(yM, y1);
	}
	double l1 = fabs(xM - xm);
	double l2 = fabs(yM - ym);
	if ( l1 > l2 ) {
		*d1 = l2;
		*d2 = l1;
	} else {
		*d1 = l1;
		*d2 = l2;
	}
}

void get_ellipse_elong(OGRGeomPtr fv0, double& d1, double& d2)
{
	OGRGeometry* fv = fv0;
    OGRLinearRing* geom = ((OGRPolygon*) fv)->getExteriorRing();

	InertialEllipse el;

    for (int i = 0; i < geom->getNumPoints(); i++) {
        double x = geom->getX(i);
        double y = geom->getY(i);
		el.push(x, y);
	}

	el.compute();
	
	el.getAxisLen(d1, d2);
}

void get_ellipse_elong(OGRGeomPtr fv0, double& d1, double& d2, double& t)
{ 
	OGRGeometry* fv = fv0;
    OGRLinearRing* geom = ((OGRPolygon*) fv)->getExteriorRing();

	InertialEllipse el;

    for (int i = 0; i < geom->getNumPoints(); i++) {
        double x = geom->getX(i);
        double y = geom->getY(i);
		el.push(x, y);
	}

	el.compute();
	
	el.getAxisLen(d1, d2, t);
}

void add_column(Connection& cnn, const std::string& table, const std::string& col_name) 
{
	std::stringstream sql;
	sql << "ALTER TABLE " << table << " ADD COLUMN " << col_name;
	try {
		cnn.execute_immediate(sql.str());
	} catch(const std::exception&) {
	}
}
std::string _set_ref_scale(const std::string& nome)
{
	if ( nome  == "1000" )
		return "RefScale_1000";
	else if ( nome  == "2000" )
		return "RefScale_2000";
	else if ( nome  == "5000" )
		return "RefScale_5000";
	else if ( nome  == "10000" )
		return "RefScale_10000";
	return "";
}
bool GetProjData(Connection& cnn, std::string& note, std::string& scale, int& datum)
{
	std::stringstream sql;
	sql << "SELECT NOTE, SCALE, DATUM from PROJECT";
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	std::string head = rs[0];
	if ( rs.eof() )
		return false;
    note = rs[0].toString();
	scale = _set_ref_scale(rs[1]);
	datum = rs[2].toInt();
	return true;
}
void read_planned_cam(Connection& cnn, Camera& cam)
{
	std::stringstream sql;
	sql << "SELECT * from " << Z_CAMERA << " where PLANNING = 1";
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	if ( rs.eof() )
		throw std::runtime_error("fotocamera di progetto non assegnata");

	cam.foc = rs["FOC"];
	cam.dimx = rs["DIMX"];
	cam.dimy = rs["DIMY"];
	cam.dpix = rs["DPIX"];
	cam.xp = rs["XP"];
	cam.yp = rs["YP"];
    cam.serial = rs["SERIAL_NUMBER"].toString();
    cam.id = rs["ID"].toString();
}

void read_cams(Connection& cnn, std::map<std::string, Camera>& map_strip_cam)
{
	std::stringstream sql;
	sql << "SELECT * from " << Z_CAMERA;
	Statement stm(cnn);
	stm.prepare(sql.str());
	Recordset rs = stm.recordset();
	
	std::map<std::string, Camera> cams;
	Camera cam_plan;

	// mappa delle camere usate compresa quella di progetto
	while ( !rs.eof() ) {
		Camera cam;
		cam.foc = rs["FOC"];
		cam.dimx = rs["DIMX"];
		cam.dimy = rs["DIMY"];
		cam.dpix = rs["DPIX"];
		cam.xp = rs["XP"];
		cam.yp = rs["YP"];
        cam.serial = rs["SERIAL_NUMBER"].toString();
        cam.id = rs["ID"].toString();
		int plan = rs["PLANNING"];
		cam.planning = plan == 1;
		rs.next();
		cams[cam.id] = cam;
		if ( cam.planning )
			cams["progetto"] = cam;
	}
	// get the camera - mission association
	std::string table = "MISSION";
	std::stringstream sql1;
	sql1 << "SELECT ID_CAMERA, NAME from " << table;
	stm.prepare(sql1.str());
	rs = stm.recordset();

	std::map<std::string, std::string> map_mission_cam;

	while ( !rs.eof() ) {
        map_mission_cam[ rs["NAME"].toString() ] = rs["ID_CAMERA"].toString(); // mission name camera id
		rs.next();
	}

	// get the mission associated to each strip
	table = std::string(ASSI_VOLO) + "V";
	std::stringstream sql2;
	sql2 << "SELECT A_VOL_CS, MISSION from " << table;
	stm = Statement(cnn);
	stm.prepare(sql2.str());
	rs = stm.recordset();
	while ( !rs.eof() ) {
		std::string strip = rs["A_VOL_CS"]; // strip name - mission name
		std::string mission = rs["MISSION"];
		std::string cam_id = map_mission_cam[mission]; // camera id for mission
		if ( cams.find(cam_id) != cams.end() )
			map_strip_cam[strip] = cams[cam_id];
		else
			map_strip_cam[strip] = cams["progetto"];
		rs.next();
	}
}
std::string get_strip(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() < 2 ) {
		throw std::runtime_error("Nome non valido: " + nome);
	}
	
	Poco::StringTokenizer::Iterator it = tok.begin();
	Poco::StringTokenizer::Iterator end = tok.end();
	
	std::stringstream out;
	out << (*it);

	for (it++; it != end - 1; it++) {
		out << "_" << *it;
	}
	return out.str();
}
std::string get_nome(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() < 2 ) {
		throw std::runtime_error("Nome non valido: " + nome);
	}
	return tok[tok.count() - 1];
}
