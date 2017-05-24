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

#if defined(__cplusplus)
extern "C" {
#endif

TOOLS_EXPORTS void ReleaseStruct(struct triangulateio *);
TOOLS_EXPORTS int InitStruct(struct triangulateio * sto, unsigned int npt, int natt, unsigned int nseg);
TOOLS_EXPORTS int Triangulate(const char * triswitches, struct triangulateio *in, struct triangulateio *out,
                                        void (*fLprintf)(char*), char* fTrErrMes);
TOOLS_EXPORTS int Voronoi(char * triswitches, struct triangulateio *in, struct triangulateio *out,
										struct triangulateio *vor, void (*fLprintf)(char*), char* fTrErrMes);
#if defined(__cplusplus)
}
#endif


#endif
