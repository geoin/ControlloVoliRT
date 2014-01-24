#include "exports.h"
#include "dem_interpolate/dsm.h"

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
double RETTA::val(const DPOINT& p) 
{
	return a * p.x + b * p.y + c;
}
//RETTA RETTA::X1PointNor(const DPOINT& p1)
//{
//	RETTA rt1;
//	rt1.a = b;
//	rt1.b = -a;
//	rt1.c = -rt1.a * p1.x - rt1.b * p1.y;
//	return rt1;
//}

TOOLS_EXPORTS int sign(double val)
{
	return ( val == 0. ) ? 0 : ( val < 0. ) ? -1 : 1;
}
