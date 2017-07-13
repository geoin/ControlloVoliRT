#include "exports.h"
#include "common/geom.h"

RETTA::RETTA(const DPOINT& p1, const DPOINT& p2) 
{
	X2Points(p1, p2);
}
void RETTA::X2Points(const DPOINT& p1, const DPOINT& p2)
{
	a = p2.y - p1.y;
	b = p1.x - p2.x;
	c = -a * p1.x - b * p1.y;
}

//void RETTA::X1PointHor(DPOINT& p1)
//{
//	a = 0.;
//	b = 1.;
//	c = -p1.y;
//}
void RETTA::X1PointPar(const DPOINT& p1, const RETTA& line)
{
	a = line.a;
	b = line.b;
	c = -a * p1.x - b * p1.y;
}
RETTA RETTA::X1PointPar(const DPOINT& p1)
{
	RETTA rt1;
	rt1.a = a;
	rt1.b = b;
    rt1.c = -rt1.a * p1.x - rt1.b * p1.y;
	return rt1;
}
void RETTA::X1PointNor(const DPOINT& p1, const RETTA& rtA)
{
	a = rtA.b;
	b = -rtA.a;
    c = -a * p1.x - b * p1.y;
}
bool RETTA::Intersez(RETTA& rt, DPOINT *point) const
{
    double den = a * rt.b - rt.a * b;
    if ( fabs(den ) < 1.e-4 )
        return false;	// parallel

    point->x = (-c * rt.b + rt.c * b) / den;
    point->y = (-a * rt.c + rt.a * c) / den;
    return true;
}
double RETTA::val(const DPOINT& p) 
{
	return a * p.x + b * p.y + c;
}

bool SEGMENTO::IsPtIn(DPOINT* pt) const
{
    double teta = p1.angdir(p2);
    double dist = p1.dist2D(p2);
    double yprj = (pt->x - p1.x) * sin(teta) + (pt->y - p1.y) * cos(teta);
    if ( fabs(yprj) < EPS_TH ) yprj = 0.;
    if ( fabs(yprj - dist ) < EPS_TH) yprj = dist;
    if ( yprj < 0. || yprj > dist )
        return false;
    return true;
}
//RETTA RETTA::X1PointNor(const DPOINT& p1)
//{
//	RETTA rt1;
//	rt1.a = b;
//	rt1.b = -a;
//	rt1.c = -rt1.a * p1.x - rt1.b * p1.y;
//	return rt1;
//}
bool LineSegIntersez(const DPOINT& pA1, const DPOINT& pA2, RETTA& rtB, DPOINT& point)
{
    RETTA	rtA;
    rtA.X2Points(pA1, pA2);
//    RETTA	rtB;
//    rtB.X2Points(pB1, pB2);

    if ( !rtA.Intersez(rtB, &point) )
        return false;

    if ( point.x >= (std::min)(pA1.x, pA2.x) &&
        point.x <= (std::max)(pA1.x, pA2.x) &&
        point.y >= (std::min)(pA1.y, pA2.y) &&
        point.y <= (std::max)(pA1.y, pA2.y) )
        return true;
    if ( fabs(point.x - (std::min)(pA1.x, pA2.x)) < EPS_TH &&
        point.y >= (std::min)(pA1.y, pA2.y) &&
        point.y <= (std::max)(pA1.y, pA2.y))
        return true;
    if ( fabs(point.x - (std::max)(pA1.x, pA2.x)) < EPS_TH &&
        point.y >= (std::min)(pA1.y, pA2.y) &&
        point.y <= (std::max)(pA1.y, pA2.y))
        return true;
    if ( fabs(point.y - (std::min)(pA1.y, pA2.y)) < EPS_TH &&
        point.x >= (std::min)(pA1.x, pA2.x) &&
        point.x <= (std::max)(pA1.x, pA2.x))
        return true;
    if ( fabs(point.y - (std::max)(pA1.y, pA2.y)) < EPS_TH &&
        point.x >= (std::min)(pA1.x, pA2.x) &&
        point.x <= (std::max)(pA1.x, pA2.x))
        return true;

    return false;
}
TOOLS_EXPORTS int FeatureIntersectR(const std::vector<DPOINT>& pol, const DPOINT& p1, const DPOINT& p2, std::vector<DPOINT>& pt)
{
    std::vector<DPOINT> p0 = pol;

    pt.clear();
    int icount = 0;
    RETTA rt(p1, p2);
    for (size_t i = 0; i < pol.size() - 1; i++) {
        DPOINT ppt;
        if ( LineSegIntersez(p0[i], p0[i + 1], rt, ppt) ) {
            pt.push_back(ppt);
            icount++;
        }
    }

    // sort the points on the line
    bool sortx = ( fabs(p2.x - p1.x) > fabs(p2.y - p2.y) );
    for (size_t i = 0; i < pt.size(); i++ ) {
        for (size_t j = i + 1; j < pt.size(); j++ ) {
            bool test = sortx ? ( pt[i].x > pt[j].x ) : ( pt[i].y > pt[j].y );
            if ( test ) {
                DPOINT dum = pt[i];
                pt[i] = pt[j];
                pt[j] = dum;
            }
        }
    }
    return icount;
}
TOOLS_EXPORTS int sign(double val)
{
	return ( val == 0. ) ? 0 : ( val < 0. ) ? -1 : 1;
}
