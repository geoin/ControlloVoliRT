#include "common/util.h"
#include "Poco/stringtokenizer.h"

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
	OGRLinearRing* or = ((OGRPolygon*) fv)->getExteriorRing();
	double xm = 1.e20, ym = 1.e20;
	double xM = -1.e20, yM = -1.e20;

	for (int i = 0; i < or->getNumPoints(); i++) {
		double x = or->getX(i) - po.getX();
		double y = or->getY(i) - po.getY();
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

void add_column(Connection& cnn, const std::string& table, const std::string& col_name) 
{
	std::stringstream sql;
	sql << "ALTER TABLE " << table << " ADD COLUMN " << col_name;
	try {
		cnn.execute_immediate(sql.str());
	}
		catch(std::exception e) {
	}
}

std::string get_strip(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() != 2 )
		return "";
	return tok[0];
}
std::string get_nome(const std::string& nome)
{
	Poco::StringTokenizer tok(nome, "_", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
	if ( tok.count() != 2 )
		return "";
	return tok[1];
}