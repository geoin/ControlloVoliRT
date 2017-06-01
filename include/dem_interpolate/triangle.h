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

#define TREAL double

#ifdef TRILIBRARY
#define HGLOBAL int
#endif
#include "exports.h"

struct triangulateio {
    TREAL *pointlist;
    TREAL *pointattributelist;
	int *pointmarkerlist;
	int numberofpoints;
	int numberofpointattributes;

	int *trianglelist;
    TREAL *triangleattributelist;
    TREAL *trianglearealist;
	int *neighborlist;
	int numberoftriangles;
	int numberofcorners;
	int numberoftriangleattributes;

	int *segmentlist;
	int *segmentmarkerlist;
	int numberofsegments;

    TREAL *holelist;
	int numberofholes;

    TREAL *regionlist;
	int numberofregions;

	int *edgelist;
	int *edgemarkerlist;
    TREAL *normlist;
	int numberofedges;
};

//#if defined(__cplusplus)
//extern "C" {
//#endif

void ReleaseStruct(struct triangulateio *);
int InitStruct(struct triangulateio * sto, unsigned int npt, int natt, unsigned int nseg);
int Triangulate(struct triangulateio *in, struct triangulateio *out,char* fTrErrMes);
//int Voronoi(char * triswitches, struct triangulateio *in, struct triangulateio *out,
//										struct triangulateio *vor, void (*fLprintf)(char*), char* fTrErrMes);
//#if defined(__cplusplus)
//}
//#endif


#endif
