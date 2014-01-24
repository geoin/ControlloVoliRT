/*================================================================================
!   Filename:  TRIANGLE.H
!
!   Contents:	Include for triangulation library
!
!   History:
!			05/04/2004	created
!			13/08/2013	updated
!================================================================================*/
#ifndef TRIANGLE_H
#define TRIANGLE_H

#define REAL double

#ifdef TRILIBRARY
#define HGLOBAL int
#endif
#include "exports.h"

struct triangulateio {
	REAL *pointlist;
	REAL *pointattributelist;
	int *pointmarkerlist;
	int numberofpoints;
	int numberofpointattributes;

	int *trianglelist;
	REAL *triangleattributelist;
	REAL *trianglearealist;
	int *neighborlist;
	int numberoftriangles;
	int numberofcorners;
	int numberoftriangleattributes;

	int *segmentlist;
	int *segmentmarkerlist;
	int numberofsegments;

	REAL *holelist;
	int numberofholes;

	REAL *regionlist;
	int numberofregions;

	int *edgelist;
	int *edgemarkerlist;
	REAL *normlist;
	int numberofedges;
};

#if defined(__cplusplus)
extern "C" {
#endif

TOOLS_EXPORTS void ReleaseStruct(struct triangulateio *);
TOOLS_EXPORTS int InitStruct(struct triangulateio * sto, int npt, int natt, int nseg);
TOOLS_EXPORTS int Triangulate(char * triswitches, struct triangulateio *in, struct triangulateio *out,
										void (*fLprintf)(char*), char* fTrErrMes);
TOOLS_EXPORTS int Voronoi(char * triswitches, struct triangulateio *in, struct triangulateio *out,
										struct triangulateio *vor, void (*fLprintf)(char*), char* fTrErrMes);
#if defined(__cplusplus)
}
#endif


#endif
