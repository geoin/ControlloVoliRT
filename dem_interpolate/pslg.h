/*=============================================================================
!   Filename:  PSLG.H
!
!   Contents:	TIN class
!
!   History:
!			05/05/2004	created
!			20/03/2013	updated
!=================================================================================*/
#ifndef PSLG_H
#define PSLG_H

#include "exports.h"
#include <set>
#include "triangle.h"
#include "dsm.h"
#include <limits>

//#include <share.h>

#ifndef INF
#define INF	1.e30
#endif

#define TR_EPS 1.e-3

static void fn_lst(char* mes) {
}

// TIN CLASS
template <typename ND, typename NT>
class TOOLS_EXPORTS tPSLG: public DSM {
public:
	unsigned int			Org_Nod;	// original num. of nodes
	tPSLG(void): _open(false),
		_npt(0), _ntriangle(0), _nseg(0),
		_nPrev(-5), _lastTri(-1), _nprev(-1), _nSize(0) {
		_trEps = TR_EPS;
	}
	~tPSLG() {
		Release();
	}
	unsigned int Npt(void) const {
		return _npt;
	}
	unsigned int Ntriangle(void) const {
		return _ntriangle;
	}
	unsigned int Nseg(void) const {
		return _nseg;
	}
	const ND& Node(unsigned int i) const {
		return node[i];
	}
	const NT& Triangle(unsigned int i) const {
		return triangle[i];
	}
	SEGMENT& Segment(unsigned int i) {
		return seg[i];
	}
	DSM_Type GetType(void) const { return DSM_TIN; }
	void SetEps(double eps) {
		_trEps = eps;
	}
	void Init() {
		_lastTri = -1;
		_nprev = -1;
	}
	bool InitNode(int count) {
		node.clear();
		_nSize = count;
		node.resize(_nSize);
		_npt = 0;
		return ( !node.empty() );
	}
	bool InitSeg(int count) {
		seg.clear();
		seg.resize(count);
		_nseg = 0;
		return ( !seg.empty() );
	}
	int AddNode(const ND& nd) {
		if ( _nSize == 0 ) {
			InitNode(100);
		} else {
			if ( _npt >= (unsigned int) _nSize ) {
				if ( !_bufferEnlarge((int) (_npt * 0.2)) )
					return _npt;
			}
		}
		node[_npt] = nd;
		setMin(nd.x, nd.y, nd.z);
		setMax(nd.x, nd.y, nd.z);

		return _npt++;
	}
	void RemoveNode(unsigned int i) {
		for (unsigned int j = i; j < _npt - 1; j++) {
			node[j] = node[j + 1];
		}
		_npt--;
	}
	int AddSegment(int n1, int n2) {
		seg[_nseg].p[0] = n1;
		seg[_nseg].p[1] = n2;
		_nseg++;
		return _nseg;
	}
	void RemoveSegment(unsigned int idx) {
		for (unsigned int i = 0; i < _nseg; i++) {
			if ( seg[i].p[0] == idx || seg[i].p[1] == idx ) {
				if ( i < _nseg - 1 ) {
					for (unsigned int j = i; j < _nseg - 1; j++) {
						seg[i] = seg[i + 1];
					}
				}
				_nseg--;
			}
		}
	}
	void SetOpen(void) { _open = true; }

	double GetX(int i) { return node[i].x; }
	double GetY(int i) { return node[i].y; }
	double GetZ(int i) { return node[i].z; }
	double GetTrX(int nTri, int i) const { int p = triangle[nTri].p[i]; return node[p].x; }
	double GetTrY(int nTri, int i) const {  int p = triangle[nTri].p[i]; return node[p].y; }
	double GetTrZ(int nTri, int i) const { int p = triangle[nTri].p[i]; return node[p].z; }
	unsigned int GetTrNode(int nTri, int i) { return triangle[nTri].p[i]; }
	void setMin(double x, double y, double z) {
		if ( _xmin > x ) _xmin = x;
		if ( _ymin > y ) _ymin = y;
		if ( _zmin > z ) _zmin = z;
	}
	void setMax(double x, double y, double z) {
		if ( _xmax < x ) _xmax = x;
		if ( _ymax < y ) _ymax = y;
		if ( _zmax < z ) _zmax = z;
	}
	void Release(void) {
		node.clear();
		triangle.clear();
		seg.clear();
		_open = false;
	}
	void Close(void) {
		if ( _open ) {
			Release();
			Init();
		}
	}
	bool	TriCalc(void) { return _triCalc(); }
	bool	VoroCalc(void) { return _voroCalc(); }

	int FindNearerTriangle(double X, double Y, int trIdx) {
		int tr = FindTriangle(X, Y, trIdx);
		if ( tr < 0 ) tr = _ltri;
		return tr;
	}
	int FindTriangle(double X, double Y, int trIdx = -1) {
		//const long _maxiter = 2500L;
		long _maxiter = (long) (0.01 * _ntriangle);
		if ( _maxiter < 1000 )
			_maxiter = 1000;

		if ( trIdx > (int) _ntriangle ) trIdx = 0;
		if ( _ltri < 0 || _ltri > (int) _ntriangle ) _ltri = 0;
		int _newTri = ( trIdx < 0 ) ? _ltri : trIdx;
		int niter = 0L;
		int triang;
		while ( true ) {
			triang = _trTry(&_newTri, X, Y);//, triangle, node);
			if ( triang < 0 )
				break;
			if ( ++niter > _maxiter )
				return -1;
		}

		if ( triang == -1 ) {
			_ltri = _newTri;
			_newTri = -1;	// external
		}
		_ltri = _newTri;
		return _newTri;
	}
	double GetQuota(double x, double y, double z = Z_NOVAL, double zo = Z_OUT) {
		if ( !_open )
			return z;
		_lastTri = FindTriangle(x, y, _lastTri);
		return ( _lastTri < 0 ) ? z : HeightInterpolation(_lastTri, x, y);
	}
	double HeightInterpolation(int n, double X, double Y) {
		if ( n != _nprev ) {
			Plane(_pln, n); // three points plane
			_nprev = n;
		}
		return -(_pln[0] * X + _pln[1] * Y + _pln[3]) / _pln[2];
	}

	bool ReadFileNod(const std::string& DemName, Progress* Prg = NULL) {
        FILE* fptr = fopen(DemName.c_str(), "r" );
        //FILE* fptr = _fsopen(DemName.c_str(), "r", _SH_DENYWR );
		if ( fptr == NULL )
			return false;

		//Progress* prg = ( Prg == NULL ) ? new Progress : Prg;

        char	val[FILENAME_MAX];
		fgets(val, 80, fptr);
		sscanf(val, "%u", &Org_Nod);
		
		if ( !InitNode(Org_Nod) ) {
			fclose(fptr);
			return false;
		}
		_npt = Org_Nod;
		
		_xmin = _ymin = _zmin = INF;
		_xmax = _ymax = _zmax = -INF;
		
		int _bar = _npt / 100;
		if ( _bar <= 0 )
			_bar = 1;
		
		//prg->Start(_npt);
		bool ko = false;
		for (unsigned int i = 0; i < _npt; i++) {
			//if ( !(i % _bar) )
			//	prg->Set(i);
			if ( fgets(val, 80, fptr) == NULL ) {
				_err_mes = "Error reading nodes at line %d";
				ko = true;
				break;
			}
			unsigned int	k;
			if ( sscanf(val, "%d %lf %lf %lf", &k, &node[i].x, &node[i].y, &node[i].z) < 4 ) {
				_err_mes = "Error reading nodes at line %d";
				ko = true;
				break;
			}
			setMin(node[i].x, node[i].y, node[i].z);
			setMax(node[i].x, node[i].y, node[i].z);
		}
		//prg->Quit();
		//if ( Prg == NULL )
		//	delete prg;

		// Read the break lines
		_nseg = 0;
		if ( !ko && fgets(val, 80, fptr) != NULL ) {
			int ns;
			sscanf(val, "%u", &ns);
			InitSeg(ns);
			_nseg = ns;
			for (unsigned int i = 0; i < _nseg; i++) {
				if ( fgets(val, 80, fptr) == NULL ) {
					_err_mes = "Error reading break-lines at line %d"/*, i)*/;
					ko = true;
					break;
				}
				int k;
				if ( sscanf(val, "%d %u %u", &k, &seg[i].p[0], &seg[i].p[1]) < 3 ) {
					_err_mes = "Error reading break-lines at line %d";
					ko = true;
					break;
				}
			}	
		}
		fclose(fptr);

		if ( ko ) {
			return false;
		}
		_open = true;
		return true;
	}
	bool WriteFileNod(const std::string& DemName, Progress* Prg = NULL) {
		FILE* fptr = fopen(DemName.c_str(), "w");
		if ( fptr == NULL )
			return false;

		Org_Nod = _npt;
		fprintf(fptr, "%u\n", Org_Nod);
		
		//Progress* prg = ( Prg == NULL ) ? new Progress : Prg;

		//prg->Start(Org_Nod);
		// writes nodes nodi
		for (unsigned int i = 0; i < (unsigned int) Org_Nod; i++) {
			//prg->Set(i);
			fprintf(fptr, "%d %.3lf %.3lf %.3lf\n", i, node[i].x, node[i].y, node[i].z);

		}
		//prg->Quit();
		//if ( Prg == NULL )
		//	delete prg;

		// writes break lines
		if ( _nseg != 0 ) {
			fprintf(fptr, "%u\n", _nseg);
			for (unsigned int i = 0; i < _nseg; i++)
				fprintf(fptr, "%d %u %u\n", i, seg[i].p[0], seg[i].p[1]);
		}
		fclose(fptr);
		return true;
	}
	bool Open(const std::string& nome, bool verbose, Progress* prb) {
		_lastTri = -1;
		_nprev = -1;
		return GetDemFromNod(nome.c_str(), verbose, prb);
	}

	bool GetDemFromNod(const char *path, bool verbose, Progress* prg = NULL) {
		if ( _open ) {
			Release();
			Init();
		}
		if ( !ReadFileNod(path, prg) )
			return false;

		Org_Nod = _npt;
		//if ( verbose ) {
		//	_cnl.Init(Fname(path).GetNome().c_str());
		//}
		//Timer t0;
		//t0.StartTime();
		bool retval = _triCalc();
		//std::string elapsed = t0.FormatTime();

		if ( retval ) {
			_open = true;
			//if ( verbose ) {
			//	_cnl.printf("\n");
			//	_cnl.printf("Number of nodes: %ld\n", _npt);
			//	_cnl.printf("Number of Triangles: %ld\n", _ntriangle);
			//	_cnl.printf("Number of segments: %ld\n", _nseg);
			//	_cnl.printf("MinX: %.2lf\tMaxX: %.2lf\n", _xmin, _xmax);
			//	_cnl.printf("MinY: %.2lf\tMaxY: %.2lf\n", _ymin, _ymax);
			//	_cnl.printf("MinZ: %.2lf\tMaxZ: %.2lf\n", _zmin, _zmax);
			//	_cnl.printf("Elapsed time: %s\n", elapsed.c_str());
			//}
			return true;
		} else {
			Release();
			return false;
		}
	}

	//double Surface(int nTri, int type = 1) const {
	//	double surface = triangle[nTri].Surface(this, type);
	//	if ( surface < _trEps )
	//		surface = 0.;
	//	return surface;
	//}
	//double Volume(int nTri, double qrif) const {
	//	return triangle[nTri].Volume(this, qrif);
	//}
	int GetOppositeNode(unsigned int tri, unsigned int nd0, unsigned int nd1) {
		int i0 = _getTriNodeIndex(tri, nd0);
		int i1 = _getTriNodeIndex(tri, nd1);
		if ( i0 < 0 || i1 < 0 )
			return -1; // one of the nodes is not in the triangle
		int i = i0 + i1;
		if ( i == 1 ) i = 2;
		else if ( i == 2 ) i = 1;
		else if ( i == 3 ) i = 0;
		return triangle[tri].p[i];
	}
	int GetAdiacent(unsigned int tri, int nod, std::set<unsigned int>& lst) {
		int id1 = nod;
		int tri1 = tri;
		unsigned int nd = triangle[tri].p[nod];

		int nod1 = (nod + 1) % 3;
		unsigned int nd1 = triangle[tri].p[nod1];
		lst.clear();
		do {
			lst.insert(tri1);
			nd1 = GetOppositeNode(tri1, nd, nd1);
			if ( nd1 < 0 )
				break;
			tri1 = triangle[tri1].n[nod1];
			if ( tri1 < 0  || lst.find(tri1) != lst.end() )
				break;
			nod1 = _getTriNodeIndex(tri1, nd1);
		} while ( tri1 != tri && nod1 >= 0 );

		if ( tri1 < 0 ) {
			nod1 = (nod + 2) % 3;
			nd1 = triangle[tri].p[nod1];
			tri1 = tri;
			do {
				if ( lst.find(tri1) == lst.end() )
					lst.insert(tri1);
				nd1 = GetOppositeNode(tri1, nd, nd1);
				if ( nd1 < 0 )
					break;
				tri1 = triangle[tri1].n[nod1];
				if ( tri1 < 0  || lst.find(tri1) != lst.end() )
					break;
				nod1 = _getTriNodeIndex(tri1, nd1);
			} while ( tri1 != tri && nod1 >= 0 );
		}
		return (int) lst.size();
	}
	//void GetNormal(int n, VecOri* norm) const {
	//	triangle[n].GetNormal(this, norm);
	//}	
	double GetSlope(int n) const {
		return triangle[n].GetSlope(this);
	}
	//double GetExposure(int n) {
	//	VecOri norm;
	//	GetNormal(n, &norm);
	//	return angdir(norm.x, norm.y);
	//}
	size_t GetBorder(std::vector<DPOINT>& vec) {
		vec.clear();
		std::vector<SEGMENT> st;

		for (unsigned int i = 0; i < _ntriangle; i++) {
			int nb = 0;
			for (int j = 0; j < 3; j++) {
				nb += ( triangle[i].n[j] == -1 );
			}
			if ( nb == 0 || nb == 3 )
				continue;
			SEGMENT sm;
			for (int j = 0; j < 3; j++) {
				if ( nb == 1 && triangle[i].n[j] == -1 ) {
					int ind = ( j - 1 >= 0 ) ? j - 1 : 2;
					sm.p[0] = triangle[i].p[ind];
					ind = ( j + 1 < 3 ) ? j + 1 : 0;
					sm.p[1] = triangle[i].p[ind];
					st.push_back(sm);
					break;
				}
				if ( (nb == 2 && triangle[i].n[j] != -1) ) {
					sm.p[0] = triangle[i].p[j];
					int ind = ( j - 1 >= 0 ) ? j - 1 : 2;
					sm.p[1] = triangle[i].p[ind];
					st.push_back(sm);
					ind = ( j + 1 < 3 ) ? j + 1 : 0;
					sm.p[1] = triangle[i].p[ind];
					st.push_back(sm);
					break;
				}
			}
		}
		if ( st.size() == 0 )
			return 0;
		std::vector<unsigned int> vc;
		vc.reserve(st.size());
		vc.push_back(st[0].p[0]);
		vc.push_back(st[0].p[1]);
		st.erase(st.begin());
		while ( st.size() ) {
			unsigned int k = vc[vc.size() - 1];
			for (unsigned int i = 0; i < st.size(); i++) {
				if ( st[i].p[0] == k ) {
					vc.push_back(st[i].p[1]);
					st.erase(st.begin() + i);
					break;
				} else if ( st[i].p[1] == k ) {
					vc.push_back(st[i].p[0]);
					st.erase(st.begin() + i);
					break;
				}
			}
		}
		for (unsigned int i = 0; i < vc.size(); i++) {
			DPOINT p1 = node[vc[i]];
			vec.push_back(p1);
		}
		return vec.size();
	}
private:
	void	Plane(double* equ, int n) {
		equ[0] = (GetTrY(n, 1) - GetTrY(n, 0)) * (GetTrZ(n, 2) - GetTrZ(n, 0)) - (GetTrZ(n, 1) - GetTrZ(n, 0)) * (GetTrY(n, 2) - GetTrY(n, 0));
		equ[1] = (GetTrZ(n, 1) - GetTrZ(n, 0)) * (GetTrX(n, 2) - GetTrX(n, 0)) - (GetTrX(n, 1) - GetTrX(n, 0)) * (GetTrZ(n, 2) - GetTrZ(n, 0));
		equ[2] = (GetTrX(n, 1) - GetTrX(n, 0)) * (GetTrY(n, 2) - GetTrY(n, 0)) - (GetTrY(n, 1) - GetTrY(n, 0)) * (GetTrX(n, 2) - GetTrX(n, 0));
		equ[3] = -(equ[0] *  GetTrX(n, 0) + equ[1] * GetTrY(n, 0) + equ[2] * GetTrZ(n, 0));
	}
	bool	_voroCalc(void) {
		struct triangulateio inData;

		// fill the internal struct for node
		InitStruct(&inData, _npt, 1, _nseg);
		for (unsigned int i = 0, k = 0; i < _npt; i++) {
			inData.pointlist[k++] = node[i].x;
			inData.pointlist[k++] = node[i].y;
			inData.pointattributelist[i] = node[i].z;
		}

		// fill the internal struct for break-lines
		if ( _nseg ) {
			for (unsigned int i = 0, k = 0; i < _nseg; i++) {
				inData.segmentlist[k++] = seg[i].p[0];
				inData.segmentlist[k++] = seg[i].p[1];
			}
		}

		// alloc the output struct
		struct triangulateio outData;
		InitStruct(&outData, 0, 0, 0);

		// alloc the struct for voronoi edges
		struct triangulateio vorData;
		InitStruct(&vorData, 0, 0, 0);
		
		bool ret = false;
        //HWND hw = GetParent(_cnl.GetListHandle().GetHwnd());
		char mes[256];
		if ( Voronoi("Vpcznv", &inData, &outData, &vorData, NULL, mes) ) {
			// add the voronoi vertexes
			if ( vorData.numberofpoints != 0 ) {
				InitNode(vorData.numberofpoints);
				_npt = vorData.numberofpoints;
				for (unsigned int i = 0, k = 0; i < _npt; i++) {
					node[i].set(vorData.pointlist[k], vorData.pointlist[k + 1], vorData.pointattributelist[i]); 
					k += 2;
				}
			}
			if ( _npt && outData.numberoftriangles != 0 ) {
				_ntriangle = outData.numberoftriangles;
				triangle.clear();// = new(_ntriangle * sizeof(NT));
				triangle.resize(_ntriangle);
				for (unsigned int i = 0; i < _ntriangle; i++) {
					int jp = 3 * i;
					triangle[i].p[0] = outData.trianglelist[jp];
					triangle[i].n[0] = outData.neighborlist[jp++];

					triangle[i].p[1] = outData.trianglelist[jp];
					triangle[i].n[1] = outData.neighborlist[jp++];

					triangle[i].p[2] = outData.trianglelist[jp];
					triangle[i].n[2] = outData.neighborlist[jp];
				}
			}

			if ( _npt && vorData.numberofedges != 0 ) {
				InitSeg(vorData.numberofedges);
				_nseg = 0;
				for (unsigned int i = 0, k = 0; i < (unsigned int) vorData.numberofedges; i++) {
					int k1 = vorData.edgelist[k++];
					int k2 = vorData.edgelist[k++];
					if ( k2 < 0 ) {
						ND nd(node[k1].x + vorData.normlist[k-2], node[k1].y + vorData.normlist[k-1]);
						k2 = AddNode(nd);
						//continue;
					}
					seg[_nseg++].Set(k1, k2);
				}
				ret = true;
			}
		}		
		_err_mes = mes;
		ReleaseStruct(&inData);
		ReleaseStruct(&outData);
		ReleaseStruct(&vorData);
		return ret;
	}

	bool	_triCalc(void) {
		struct triangulateio inData;

		// fill the internal struct for node
		InitStruct(&inData, _npt, 1, _nseg);
		for (unsigned int i = 0, k = 0; i < _npt; i++) {
			inData.pointlist[k++] = node[i].x;
			inData.pointlist[k++] = node[i].y;
			inData.pointattributelist[i] = node[i].z;
		}

		// fill the internal struct for break-lines
		if ( _nseg ) {
			for (unsigned int i = 0, k = 0; i < _nseg; i++) {
				inData.segmentlist[k++] = seg[i].p[0];
				inData.segmentlist[k++] = seg[i].p[1];
			}
		}

		// alloc the output struct
		struct triangulateio outData;
		InitStruct(&outData, 0, 0, 0);
		
		bool ret = false;
		//_hl_ = GetParent(_cnl.GetListHandle().GetHwnd());
		char mes[256];
		if ( Triangulate("Vpczn", &inData, &outData, fn_lst, mes) ) {

			// add the new nodes
			if ( _npt != (unsigned int) outData.numberofpoints ) {
				InitNode(outData.numberofpoints);
				_npt = outData.numberofpoints;
				for (unsigned int i = 0, k = 0; i < _npt; i++) {
					node[i].set(outData.pointlist[k], outData.pointlist[k + 1], outData.pointattributelist[i]); 
					k += 2;
				}
			}

			// triangle list
			_ntriangle = outData.numberoftriangles;
			//triangle = new[_ntriangle];// * sizeof(NT));
			triangle.resize(_ntriangle);
			for (unsigned int i = 0; i < _ntriangle; i++) {
				int jp = 3 * i;
				triangle[i].p[0] = outData.trianglelist[jp];
				triangle[i].n[0] = outData.neighborlist[jp++];

				triangle[i].p[1] = outData.trianglelist[jp];
				triangle[i].n[1] = outData.neighborlist[jp++];

				triangle[i].p[2] = outData.trianglelist[jp];
				triangle[i].n[2] = outData.neighborlist[jp];
			}
			ret = true;
		}
		_err_mes = mes;
		ReleaseStruct(&inData);
		ReleaseStruct(&outData);
		return ret;
	}

	int		_trSign(double val) {
		const double thr = _trEps;
		if ( fabs(val) < thr )
			return 0;
		return ( val < 0. ) ? -1 : 1;
	}
	int		_trTry(int* iTri, double xa, double ya) {
		if ( _nPrev != *iTri ) {
			// different from previous
			for (int i = 0; i < 3; i++) {
				int v1 = triangle[*iTri].p[(i + 1) % 3];
				int v2 = triangle[*iTri].p[(i + 2) % 3];
				_rt[i].X2Points(node[v2], node[v1]);
			}

			// sign of an internal point
			_sgn = sign(_rt[0].val(node[triangle[*iTri].p[0]]));
			if ( _sgn == 0 )
				_sgn = sign(_rt[1].val(node[triangle[*iTri].p[1]]));
			_nPrev = *iTri;
		}

		// tpos < 4 outside; tpos = 6 out	_sideSign = 5 on a side
		// tpos = 4 inside if _sideSign = 2
		int tpos = 0;
		int sTot = 0;
		int	tNear[3];
		int _sideSign[3];
		for (int i = 0; i < 3; i++) {
			_sideSign[i] = abs(_trSign(_rt[i].val(xa, ya)) + _sgn);
			if ( _sideSign[i] == 1 )
				sTot++;
			tNear[i] = triangle[*iTri].n[i];
			tpos += _sideSign[i];
		}
		if ( tpos >= 5 || tpos == 4 && sTot > 1 )
			return -2;	// inside
		
		for (int i = 0; i < 3; i++) {
			if ( _sideSign[i] == 0 )
				*iTri = tNear[i];
		}

		// outside
		if ( _sideSign[0] == 0 && *iTri == -1 )
			*iTri = tNear[0];	
		if ( _sideSign[1] == 0 && *iTri == -1 )
			*iTri = tNear[1];
		if ( _sideSign[2] == 0 && *iTri == -1 )
			*iTri = tNear[2];

		if ( *iTri == -1 ) {
			*iTri = _nPrev;
			return -1;
		}
		return *iTri;
	}
	bool _bufferEnlarge(int np, int ns = 0) {
		void* _hg;
		if ( np ) {
			if ( _nSize == 0 ) {
				return InitNode(100);
			}
			_nSize = _npt + np;
			node.resize(_nSize);
		}

		if ( ns ) {
			seg.resize(_nseg + ns);
		}
		return true;
	}
	int _getTriNodeIndex(unsigned int tri, unsigned int nd) {
		for (int j = 0; j < 3; j++) {
			if ( triangle[tri].p[j] == nd )
				return j;
		}
		return -1; // the node does not belong to the triangle
	}

	unsigned int	_npt;		// num. of nodes after triangulation
	unsigned int	_ntriangle;	// number of triangles
	unsigned int	_nseg;		// number of break lines
	long	_lastTri;
	int		_ltri;
	double	_pln[4];
	int		_nprev;
	int		_nPrev, _sgn; // use by _trTry
	RETTA	_rt[3];
	double	_trEps;
	long	_nSize;
	bool	_open;

	std::vector<ND>			node;
	std::vector<NT>			triangle;
	std::vector<SEGMENT>	seg;
};

typedef tPSLG<NODE, TRIANGLE> PSLG;

#endif
