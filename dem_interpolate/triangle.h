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

_declspec (dllexport) void ReleaseStruct(struct triangulateio *);
_declspec (dllexport) int InitStruct(struct triangulateio * sto, int npt, int natt, int nseg);
_declspec (dllexport) int Triangulate(char * triswitches, struct triangulateio *in, struct triangulateio *out,
										void (*fLprintf)(char*), char* fTrErrMes);
_declspec (dllexport) int Voronoi(char * triswitches, struct triangulateio *in, struct triangulateio *out,
										struct triangulateio *vor, void (*fLprintf)(char*), char* fTrErrMes);
#if defined(__cplusplus)
}
#endif


#endif