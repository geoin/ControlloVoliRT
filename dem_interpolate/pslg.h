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
#include <Poco/String.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include "fastparser.h"
#include <nanoflann.hpp>

#include <cstdio>
#include <cmath>

#ifndef INF
#define INF	1.e30
#endif

#define TR_EPS 1.e-10
#if !defined(UINT)
    #define UINT unsigned int
#endif

template <typename T>
struct PointCloud
{
    struct Point
    {
        T  x,y,z;
    };

    std::vector<Point>  pts;

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const { return pts.size(); }

    // Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
    inline T kdtree_distance(const T *p1, const size_t idx_p2,size_t /*size*/) const
    {
        const T d0=p1[0]-pts[idx_p2].x;
        const T d1=p1[1]-pts[idx_p2].y;
        const T d2=p1[2]-pts[idx_p2].z;
        return d0*d0+d1*d1+d2*d2;
    }

    // Returns the dim'th component of the idx'th point in the class:
    // Since this is inlined and the "dim" argument is typically an immediate value, the
    //  "if/else's" are actually solved at compile time.
    inline T kdtree_get_pt(const size_t idx, int dim) const
    {
        if (dim==0) return pts[idx].x;
        else if (dim==1) return pts[idx].y;
        else return pts[idx].z;
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }

};
template <typename T>
struct PointCloud2D
{
    struct Point
    {
        T  x,y;
    };

    std::vector<Point>  pts;

    // Must return the number of data points
    inline size_t kdtree_get_point_count() const { return pts.size(); }

    // Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
    inline T kdtree_distance(const T *p1, const size_t idx_p2,size_t /*size*/) const
    {
        const T d0=p1[0]-pts[idx_p2].x;
        const T d1=p1[1]-pts[idx_p2].y;
        return d0*d0+d1*d1;
    }

    // Returns the dim'th component of the idx'th point in the class:
    // Since this is inlined and the "dim" argument is typically an immediate value, the
    //  "if/else's" are actually solved at compile time.
    inline T kdtree_get_pt(const size_t idx, int dim) const
    {
        if (dim==0) return pts[idx].x;
        else return pts[idx].y;
    }

    // Optional bounding-box computation: return false to default to a standard bbox computation loop.
    //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
    //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
    template <class BBOX>
    bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }

};

typedef nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<float, PointCloud<float> >,
    PointCloud<float>,
    3 /* dim */
> my_kd_tree_t;

typedef nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<float, PointCloud2D<float> >,
    PointCloud2D<float>,
    2 /* dim */
> my_kd_tree_t2;

static void fn_lst(char* mes) {
}

class InertialEllipse {
public:
	InertialEllipse() 
		: _n(0), Ixx(0.0), Iyy(0.0), Ixy(0.0), Mx(0.0), My(0.0), M2x(0.0), M2y(0.0), Mxy(0.0)
	{}

	inline void add(const double& x, const double& y) {
		size_t n = _n + 1;

		double vc = double(n - 1) / double(n);
		Mx  = vc * Mx + x / n;
		My  = vc * My + y / n;
		M2x = vc * M2x + x * x / n;
		M2y = vc * M2y + y * y / n;
		Mxy = vc * Mxy + x * y / n;

		_n++;
	}

	void push(const double& x, const double& y) {
		add(x, y);
		_points.push_back(DPOINT(x, y));
	}

	inline void compute() {
		Ixx = M2x - Mx * Mx;
		Iyy = M2y - My * My;
		//Ixy = Mxy - Mx * My;
		Ixy = (double(_n) / double(_n - 1)) * (Mxy - Mx * My);
        Theta = atan2(2 * Ixy, Ixx - Iyy) / 2;

        double c = sqrt(pow(Ixx - Iyy, 2.) + 4 * pow(Ixy, 2.));
        ra = sqrt(2.) * sqrt(Ixx + Iyy + c);
        rb = sqrt(2.) * sqrt(Ixx + Iyy - c);
	}

	inline void getAxisLen(double& l1, double& l2, double& theta) {
		double tmin = 1e30;
		double tmax = -1e30;

		double tminB = 1e30;
		double tmaxB = -1e30;
		
		theta = atan2(2 * Ixy, Ixx - Iyy) / 2;
		double vi = cos(theta);
		double vj = sin(theta);

		double n = _points.size(); 
		for (int i = 0; i < n; i++) {
			DPOINT p = _points.at(i);
			double ti = (p.x - Mx) * vi + (p.y - My) * vj;
			tmin = std::min(tmin, ti);
			tmax = std::max(tmax, ti);
			
			ti = -(p.x - Mx) * vj + (p.y - My) * vi;
			tminB = std::min(tminB, ti);
			tmaxB = std::max(tmaxB, ti);
		}

		l1 = std::abs(tmax)  + std::abs(tmin);
		l2 = std::abs(tmaxB)  + std::abs(tminB);
	}

	
	inline void getAxisLen(double& l1, double& l2) {
		double t_;
		getAxisLen(l1, l2, t_);
	}

//	inline void getMajorAxis(DPOINT& p1, DPOINT& p2) {
//		double tmin = 1e30;
//		double tmax = -1e30;

//		double tminB = 1e30;
//		double tmaxB = -1e30;
		
//		double theta = atan2(2 * Ixy, Ixx - Iyy) / 2;
//		double vi = cos(theta);
//		double vj = sin(theta);

//		double n = _points.size();
//		for (int i = 0; i < n; i++) {
//			DPOINT p = _points.at(i);
//			double ti = (p.x - Mx) * vi + (p.y - My) * vj;
//			tmin = std::min(tmin, ti);
//			tmax = std::max(tmax, ti);
			
//			ti = -(p.x - Mx) * vj + (p.y - My) * vi;
//			tminB = std::min(tminB, ti);
//			tmaxB = std::max(tmaxB, ti);
//		}
		
//		p1.set(Mx + tmin * vi, My + tmin * vj);
//		p2.set(Mx + tmax * vi, My + tmax * vj);
//	}

	inline void getMajorAxis(DSM* dsm) {
		double tmin = 1e30;
		double tmax = -1e30;

		double tminB = 1e30;
		double tmaxB = -1e30;
		
		double theta = atan2(2 * Ixy, Ixx - Iyy) / 2;
		double vi = cos(theta);
		double vj = sin(theta);

		double n = dsm->Npt(); 
		for (int i = 0; i < n; i++) {
			DPOINT p = dsm->Node(i);
			double ti = (p.x - Mx) * vi + (p.y - My) * vj;
			tmin = std::min(tmin, ti);
			tmax = std::max(tmax, ti);

			ti = -(p.x - Mx) * vj + (p.y - My) * vi;
			tminB = std::min(tminB, ti);
			tmaxB = std::max(tmaxB, ti);
		}
		
        FirstPt.set(Mx + tmin * vi, My + tmin * vj);
        LastPt.set(Mx + tmax * vi, My + tmax * vj);

    // vertici strip
        A = DPOINT(FirstPt.x - tminB * vj, + FirstPt.y + tminB * vi);
        B = DPOINT(FirstPt.x - tmaxB * vj, + FirstPt.y + tmaxB * vi);
        C = DPOINT(LastPt.x - tmaxB * vj, + LastPt.y + tmaxB * vi);
        D = DPOINT(LastPt.x - tminB * vj, + LastPt.y + tminB * vi);
    }
    inline void calculateStrip(DSM* dsm) {
        double minX = INF, minY = INF;
        double maxX = -INF, maxY = -INF;
        MatOri mr( 0, 0, Theta );
        DPOINT tr( Mx, My );
        DPOINT ti;
        for( UINT i = 0; i < dsm->Npt(); i++ ) {
            const DPOINT& pt = dsm->Node( i );
            ti = mr * (pt - tr);
            minX = std::min( minX, ti.x );
            maxX = std::max( maxX, ti.x );

            minY = std::min( minY, ti.y );
            maxY = std::max( maxY, ti.y );
        }

        double thr( 0.75 );

        double tminX = minX * thr;
        double tmaxX = maxX * thr;
        double tminY = minY * thr;
        double tmaxY = maxY * thr;

        A = DPOINT( INF, -INF );
        B = DPOINT( -INF, -INF );
        C = DPOINT( -INF, INF );
        D = DPOINT( INF, INF );

        for( UINT i = 0; i < dsm->Npt(); i++ ) {
            const DPOINT& pt = dsm->Node( i );
            ti = mr * (pt - tr);

            if ( ti.y > tmaxY ) {
                if (  ti.x < tminX ) {
                    A.x = std::min( A.x, ti.x );
                    A.y = std::max( A.y, ti.y );
                }
                if( ti.x > tmaxX ) {
                    B.x = std::max( B.x, ti.x );
                    B.y = std::max( B.y, ti.y );
                }
            } else if ( ti.y < tminY ) {
                if( ti.x < tminX ) {
                    D.x = std::min( D.x, ti.x );
                    D.y = std::min( D.y, ti.y );
                }
                if( ti.x > tmaxX ) {
                    C.x = std::max( C.x, ti.x );
                    C.y = std::min( C.y, ti.y );
                }
            }
        }
        if ( A.x == INF) A.x = minX;
        if ( A.y == -INF ) A.y = maxY;
        if ( B.x == -INF ) B.x = maxX;
        if ( B.y == -INF ) B.y = maxY;
        if( C.x == -INF ) C.x = maxX;
        if ( C.y == INF ) C.y = minY;
        if ( D.x == INF ) D.x = minX;
        if ( D.y == INF ) D.y = minY;

        MatOri mr1 = mr.Invert();
        A = mr1 * A + tr;
        B = mr1 * B + tr;
        C = mr1 * C + tr;
        D = mr1 * D + tr;

        FirstPt = DPOINT( (A.x + D.x) / 2, (A.y + D.y) / 2 );
        LastPt = DPOINT( (B.x + C.x) / 2, (B.y + C.y) / 2 );
    }
    void extreme_points(DPOINT& p1, DPOINT& p2)
    {
        double vi = cos(Theta);
        double vj = sin(Theta);

        p1 = DPOINT(Mx - ra * vi, My - ra * vj, 0);
        p2 = DPOINT(Mx + ra * vi, My + ra * vj, 0);
    }
	double Mx, My;

	double Ixx, Iyy, Ixy; // momenti d'inerzia	
	double M2x, M2y, Mxy;
    double Theta;
    double ra, rb;

    DPOINT A, B, C, D;      // corners of strip footprint
    DPOINT FirstPt, LastPt; // ending point of axes

private:
	size_t _n; // numero punti processati
	std::vector<DPOINT> _points;
};

class TOOLS_EXPORTS Bbox {
public:
	Bbox() {}
	//box(double x0, double y0, double dx, double dy): _xo(x0 - dx), _y0(y0 - dy), _x1(x0 + dx), _y1(y0 + dy) {}
	Bbox(double x0, double y0, double x1, double y1): _x0(x0), _y0(y0), _x1(x1), _y1(y1) {}
	bool is_inside(double x, double y) const {
		return x >= _x0 && x <= _x1 && y >= _y0 && y <= _y1;
	}
private:
	double _x0, _x1;
	double _y0, _y1;
};

// TIN CLASS
template <typename ND, typename NT>
class TOOLS_EXPORTS tPSLG: public DSM {
public:
	unsigned int			Org_Nod;	// original num. of nodes
	tPSLG(double eps = TR_EPS): _open(false),
		_npt(0), _ntriangle(0), _nseg(0),
        _nPrev(-5), _lastTri(-1), _nprev(-1), myIndex(nullptr), myIndex2(nullptr),
		_nSize(0), _p_off(), _p_scale(1., 1., 1.),
		_echo(0), _trEps(TR_EPS) {}
	virtual ~tPSLG() {
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
	const ND Node(unsigned int i) const {
		return _denormalize(node[i]);
	}
	void Node(const DPOINT& p, unsigned int i) {
		node[i].x = p.x;
		node[i].y = p.y;
		node[i].z = p.z;
		_normalize(i);
	}
	const NT& Triangle(unsigned int i) const {
		return triangle[i];
	}
	const SEGMENT& Segment(unsigned int i) const {
		return seg[i];
	}
	void _set_off_scale(void) {
		_p_off = DPOINT(_xmin, _ymin, 0);
		double dx = _xmax - _xmin;
		double dy = _ymax - _ymin;
		dx = std::max(dx, dy);
		_p_scale = DPOINT(dx, dx, dx);
	}
	void _normalize(const ND& nd, int k) {
		node[k] = nd;
		_normalize(k);
	}
	void _normalize(int k) {
		node[k] -= _p_off;
		node[k] /= _p_scale;
	}
    ND _normalize(const ND& nod) {
        ND nd = nod;
        nd -= _p_off;
        nd /= _p_scale;
        return nd;
    }

	ND _denormalize(const ND& nd) const {
		ND nd1 = nd;
		nd1 *= _p_scale;
		nd1 += _p_off;
		return nd1;
	}
	double GetZ(int i) const {
		return node[i].z * _p_scale.z + _p_off.z;
	}
	double GetX(int i) const {
		return node[i].x * _p_scale.x + _p_off.x;
	}
	double GetY(int i) const {
		return node[i].y * _p_scale.y + _p_off.y;
	}
	DSM_Type GetType(void) const { return DSM_TIN; }
	void SetEps(double eps) {
		_trEps = eps;
	}
	void SetEcho(int echo) {
		_echo = echo;
//		if ( _echo == 3 )
//			_echo = 4; // per gli echi intermedi
	}
	void SetAngle( int angle ) {
		_angle = angle;
	}
	void SetMask(const File_Mask& fm) {
		_fm = fm;
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
		_normalize(nd, _npt);
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

	double GetX(int i) { return node[i].x * _p_scale.x + _p_off.x; }
	double GetY(int i) { return node[i].y * _p_scale.y + _p_off.y; }
	double GetZ(int i) { return node[i].z * _p_scale.z + _p_off.z; }
	double GetTrX(int nTri, int i) const {
		return GetX(triangle[nTri].p[i]);
	}
	double GetTrY(int nTri, int i) const {
		return GetY(triangle[nTri].p[i]);
	}
	double GetTrZ(int nTri, int i) const {
		return GetZ(triangle[nTri].p[i]);
	}
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
        cloud.pts.clear();
        cloud2.pts.clear();
		_open = false;
        if ( myIndex )
            delete myIndex;
        myIndex2 = nullptr;
        if ( myIndex2 )
            delete myIndex2;
        myIndex2 = nullptr;
	}
	void Close(void) {
		if ( _open ) {
			Release();
			Init();
		}
	}
	bool	TriCalc(void) { return _triCalc(); }
	bool	VoroCalc(void) { return _voroCalc(); }

	int FindNearerTriangle(double X, double Y, int trIdx) const {
		int tr = FindTriangle(X, Y, trIdx);
		if ( tr < 0 ) tr = _ltri;
		return tr;
	}
	int FindTriangle(double X, double Y, int trIdx = -1) const {
		X = (X -_p_off.x) / _p_scale.x; 
		Y = (Y -_p_off.y) / _p_scale.y; 
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
    bool isInnerTri(int itri) const {
        return triangle[itri].n[0] >= 0 && triangle[itri].n[1] >= 0 && triangle[itri].n[2] >= 0;
    }

    void CreateIndex() {
        for( unsigned int i = 0; i < Npt(); i++ ) {
            const NODE& nd = node[i];
            PointCloud<float>::Point p;
            p.x = nd.x;
            p.y = nd.y;
            p.z = nd.z;

            cloud.pts.push_back( p );
        }
        myIndex = new  my_kd_tree_t( 3 /*dim*/, cloud, nanoflann::KDTreeSingleIndexAdaptorParams( 100 /* max leaf */ ) );
        myIndex->buildIndex();
    }
    void CreateIndex2() {
        for( unsigned int i = 0; i < Npt(); i++ ) {
            const NODE& nd = node[i];
            PointCloud2D<float>::Point p;
            p.x = nd.x;
            p.y = nd.y;

            cloud2.pts.push_back( p );
        }
        myIndex2 = new  my_kd_tree_t2( 2 /*dim*/, cloud2, nanoflann::KDTreeSingleIndexAdaptorParams( 100 /* max leaf */ ) );
        myIndex2->buildIndex();
    }
    double getPoint(const NODE& nd) {
        if ( myIndex == nullptr )
            return -1;
        NODE n1 = _normalize(nd);

        float query_pt[3] = { float(n1.x), float(n1.y), float(n1.z) };

        const size_t num_results = 1;
        size_t ret_index;
        float out_dist_sqr;
        nanoflann::KNNResultSet<float> resultSet( num_results );
        resultSet.init( &ret_index, &out_dist_sqr );
        myIndex->findNeighbors( resultSet, &query_pt[0], nanoflann::SearchParams( 10 ) );
        if ( ret_index > 0 )
            return out_dist_sqr;
        return -1;
        //std::cout << "Index search " << ret_index << " in " << tm.FormatTime() <<  " Dist " << out_dist_sqr << std::endl;
    }

    bool getPoints(const NODE& nd, double radius, std::vector<int>& indexes) {
        if ( myIndex == nullptr )
            return -1;
        NODE n1 = _normalize(nd);
        float rd = pow(radius / _p_scale[0], 2);

        float query_pt[3] = { float(n1.x), float(n1.y), float(n1.z) };

        nanoflann::SearchParams params;

        std::vector<std::pair<size_t, float > > IndicesDists;
        size_t kk = myIndex->radiusSearch( &query_pt[0], rd, IndicesDists, params );
        //std::cout << "Radius search " << kk << " in " << tm.FormatTime() << std::endl;

        indexes.clear();
        for( size_t i = 0; i < IndicesDists.size(); i++ ) {
            indexes.push_back(IndicesDists[i].first);
            //std::cout << "Index " << IndicesDists[i].first << " Dist " << IndicesDists[i].second << std::endl;
        }
        return !IndicesDists.empty();
    }
    bool getPoints2(const NODE& nd, double radius, std::vector<int>& indexes) {
        if ( myIndex2 == nullptr )
            return -1;
        NODE n1 = _normalize(nd);
        float rd = pow(radius / _p_scale[0], 2);

        float query_pt[3] = { float(n1.x), float(n1.y), float(n1.z) };

        nanoflann::SearchParams params;

        std::vector<std::pair<size_t, float > > IndicesDists;
        size_t kk = myIndex2->radiusSearch( &query_pt[0], rd, IndicesDists, params );
        //std::cout << "Radius search " << kk << " in " << tm.FormatTime() << std::endl;

        indexes.clear();
        for( size_t i = 0; i < IndicesDists.size(); i++ ) {
            indexes.push_back(IndicesDists[i].first);
            //std::cout << "Index " << IndicesDists[i].first << " Dist " << IndicesDists[i].second << std::endl;
        }
        return !IndicesDists.empty();
    }

	double GetQuota(double x, double y, double z = Z_NOVAL, double zo = Z_OUT) const {
		if ( !_open )
			return z;
		_lastTri = FindTriangle(x, y, _lastTri);
        if ( _lastTri < 0)
            return Z_OUT;
       if ( !isInnerTri(_lastTri) )
           return Z_WEAK;
       Triangolo t;
       Get_triangle(t, _lastTri);
       double ff = t.AspectRatio();
       if ( ff < 0.3 )
           return Z_WEAK;
        return HeightInterpolation(_lastTri, x, y);
	}
	void _trIn(std::set<unsigned int>& tr, std::set<unsigned int>& nd, long k, const Bbox& b) const {
		tr.insert(k);
		bool ins = false;
		for ( int i = 0; i < 3; i++) {
			ND n = node[triangle[k].p[i]];
			if ( b.is_inside(n.x, n.y) ) {
				nd.insert(triangle[k].p[i]);
				ins = true;
			}
		}
		if ( ins ) {
			for ( int i = 0; i < 3; i++) {
				if ( tr.find(k) == tr.end()  && triangle[k].n[i] > 0 )
					_trIn(tr, nd, triangle[k].n[i], b);
			}

		}
	}

	double GetQm(double x, double y, double dx, double dy, double z = Z_NOVAL, double zo = Z_OUT) const {
		if ( !_open )
			return z;
		_lastTri = FindTriangle(x, y, _lastTri);
		if ( _lastTri < 0 ) 
			return z;
		Bbox b((x - _p_off.x - dx) / _p_scale.x, (y - _p_off.y - dy) / _p_scale.y, (x - _p_off.x + dx) / _p_scale.x, (y - _p_off.y + dy) / _p_scale.y);

		std::set<unsigned int> tr;
		std::set<unsigned int> nd;
		_trIn(tr, nd, _lastTri, b);
		if ( nd.empty() )
			return GetQuota(x, y, z, zo);
		std::set<unsigned int>::const_iterator it;
		double qt = 0.;
		for (it = nd.begin(); it != nd.end(); it++) {
			qt += node[*it].z;
		}
		return qt / nd.size();
	}

	double HeightInterpolation(int n, double X, double Y) const {
		if ( n != _nprev ) {
			Plane(_pln, n); // three points plane
			_nprev = n;
		}
		return -(_pln[0] * X + _pln[1] * Y + _pln[3]) / _pln[2];
	}

	bool ReadFileNod(const std::string& DemName) {
        FILE* fptr = fopen(DemName.c_str(), "r" );
		if ( fptr == NULL )
			return false;

        char	val[FILENAME_MAX];
		fgets(val, 80, fptr);
		sscanf(val, "%u", &Org_Nod);
		
		if ( !InitNode(Org_Nod) ) {
			fclose(fptr);
			return false;
		}
		//_npt = Org_Nod;
		
		_xmin = _ymin = _zmin = INF;
		_xmax = _ymax = _zmax = -INF;
		
		//int _bar = _npt / 100;
		//if ( _bar <= 0 )
		//	_bar = 1;
		
		//prg->Start(_npt);
		bool ko = false;
		for (unsigned int i = 0; i < Org_Nod; i++) {
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
		_set_off_scale();
		for (unsigned int i = 0; i < Org_Nod; i++) {
			_normalize(i);
		}

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
	bool WriteFileNod(const std::string& DemName) {
		FILE* fptr = fopen(DemName.c_str(), "w");
		if ( fptr == NULL )
			return false;

		Org_Nod = _npt;
		fprintf(fptr, "%u\n", Org_Nod);
		
		//Progress* prg = ( Prg == NULL ) ? new Progress : Prg;

		//prg->Start(Org_Nod);
		// writes nodes nodi
		for (unsigned int i = 0; i < (unsigned int) Org_Nod; i++) {
			ND nd = _denormalize(node[i]);
			fprintf(fptr, "%d %.3lf %.3lf %.3lf\n", i, nd.x, nd.y, nd.z);

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
	bool Open(const std::string& nome, bool verbose, bool tria) {
		_lastTri = -1;
		_nprev = -1;
		std::string ext = Poco::Path(nome).getExtension();
		if ( !Poco::icompare(ext, "las") )
			return GetDemFromLas(nome, verbose, tria);
		if ( !Poco::icompare(ext, "nod") )
			return GetDemFromNod(nome, verbose, tria);

		return GetDemFromASCII(nome, verbose, tria);
	}
	bool GetDemFromASCII(const std::string& nome, bool verbose, bool tria) {
	
		if ( _open ) {
			Release();
			Init();
		}
		_xmin = _ymin = _zmin = INF;
		_xmax = _ymax = _zmax = -INF;

		FILE* fptr = fopen(nome.c_str(), "r" );
		if ( fptr == NULL )
			return false;
		std::vector<char> buffer;
		buffer.resize(65535);
		setbuf(fptr, &buffer[0]);

        // search for the first record to find the average record size
		FastParser tok;
		char	mes[FILENAME_MAX];
		bool found = false;
		while ( !found ) {
			if ( fgets(mes, 80, fptr) == NULL ) {
				fclose(fptr);
				return false;
			}
			if ( tok.Parse(mes) == _fm.nf )
			found = true;
		}
		fseek(fptr, 0, 0);

		Poco::File f(nome);
		unsigned long sz = f.getSize();
		size_t szr = strlen(mes);
		long nrec = (long) (sz / szr); // average record count

        if ( _fm.ne < 0 || _fm.ni < 0)
            _echo = MyLas::all_pulses;

		node.clear();
		node.reserve(nrec);
		
		int nprec = -1;
		while ( fgets(mes, 80, fptr) ) {
			if ( tok.Parse(mes) != _fm.nf )
				continue;
			if ( _echo != 0 ) {
				// selezionagli impulsi
				int echo = MyLas::all_pulses;
				int ne = tok.GetInt(_fm.ne - 1); // echo attuale
				int nt = tok.GetInt(_fm.ni - 1); // numero complessivo di echi
				if ( ne == 1 || ne == nprec )
					echo |= MyLas::first_pulse;
				if ( ne == nt )
					echo |= MyLas::last_pulse;
				if ( echo == MyLas::all_pulses )
					echo = MyLas::intermediate_pulse;
				if (nt == 1) {
					echo |= MyLas::single_pulse;
				}
				nprec = ne;
				if ( !(_echo & echo) ) // 1 primo impulso 2 ultimo 3 intermedio 0 tutti
					continue;
			}

			ND n;
			n.x = tok.GetFloat(_fm.x-1);
			n.y = tok.GetFloat(_fm.y-1);
			n.z = tok.GetFloat(_fm.z-1);
			setMin(n.x, n.y, n.z);
			setMax(n.x, n.y, n.z);
			node.push_back(n);
		}
        fclose(fptr);
		if (  node.empty() )
			return false;
		Org_Nod = (unsigned int) node.size();
		_nSize = Org_Nod;
		
		_set_off_scale();
		for (unsigned int i = 0; i < Org_Nod; i++)
			_normalize(i);

		_npt = Org_Nod;
		_open = true;

		if (!tria) {
			return true;
		}

		bool retval = _triCalc();
		if (!retval) {
			Release();
		}
		return retval;
	}
	bool GetDemFromLas(const std::string& nome, bool verbose, bool tria) {
		if ( _open ) {
			Release();
			Init();
		}
		MyLas ml;
		if ( ( Org_Nod = ml.open(nome)) == 0 )
			return false;
		if ( !InitNode(Org_Nod) )
			return false;

        std::cout << "Las " << nome << " " << Org_Nod << " punti" << std::endl;

		//bool ret = true;
		Org_Nod = 0;

        _xmin = _ymin = _zmin = INF;
        _xmax = _ymax = _zmax = -INF;

		DPOINT pt;

        while ( ml.get_next_point(pt) ) {
            if ( !ml.isvalid() )
                continue;

            if ( _angle != 0 ) {
                int angle = ml.get_angle();
                if( angle < -_angle || angle > _angle )
                    continue;
            }

            int act_echo = ml.get_echo();
            if ( act_echo == 0 )
                std::cout << "PUNTO ANOMALO\n";

            if ( act_echo & MyLas::first_pulse )
                _count_first++;
            if ( act_echo & MyLas::last_pulse )
                _count_last++;
            if ( act_echo == MyLas::single_pulse )
                _count_single++;
            if ( act_echo == MyLas::intermediate_pulse )
                _count_inter++;

            if ( _count_single > _count_first ) {
                std::cout << "!!! " << act_echo << " fi " << _count_first << " si " <<_count_single << std::endl;
            }

            if ( _echo != 0 && ( (_echo & act_echo) != _echo ) ) // 1 primo impulso 2 ultimo 3 intermedio 0 tutti
				continue;


			node[Org_Nod].x = pt.x;
			node[Org_Nod].y = pt.y;
			node[Org_Nod].z = pt.z;
            Org_Nod++;

            setMin(pt.x, pt.y, pt.z);
            setMax(pt.x, pt.y, pt.z);

			_ie.add(pt.x, pt.y);
		}
        if ( Org_Nod == 0 )
            return false;
        node.resize(Org_Nod);
        _npt = Org_Nod;
        _open = true;

        std::cout << " !!! Read " << Org_Nod << " punti con angolo: " << _angle << " echo " << _echo << std::endl;
        _set_off_scale();
        for (unsigned int i = 0; i < Org_Nod; i++)
            _normalize(i);

		_ie.compute();

		if (!tria) {
			return true;
		}

		bool retval = _triCalc();
		if (!retval) {
			Release();
		}

        std::cout << " N. meshes " << _ntriangle << std::endl;
		return retval;
	}

    virtual void calculateStrip(std::vector<DPOINT>& bb, DPOINT& firstPt, DPOINT& lastPt) {
        _ie.calculateStrip(this);
        bb.clear();
        bb.push_back(_ie.A);
        bb.push_back(_ie.B);
        bb.push_back(_ie.C);
        bb.push_back(_ie.D);
        firstPt = _ie.FirstPt;
        lastPt = _ie.LastPt;
    }

	virtual void getMajorAxis(DPOINT& p1, DPOINT& p2) {
        if (_ie.FirstPt == DPOINT() && _ie.LastPt == DPOINT()) {
			_ie.getMajorAxis(this);
		}

        p1 = _ie.FirstPt;
        p2 = _ie.LastPt;
	} 

	virtual void getBB(DPOINT& p1, DPOINT& p2, DPOINT& p3, DPOINT& p4) {
        if (_ie.FirstPt == DPOINT() && _ie.LastPt == DPOINT()) {
			_ie.getMajorAxis(this); 
		}

        p1 = _ie.A;
        p2 = _ie.B;
        p3 = _ie.C;
        p4 = _ie.D;
	} 

	bool GetDemFromNod(const std::string& path, bool verbose, bool tria) {
		if ( _open ) {
			Release();
			Init();
		}
		if ( !ReadFileNod(path) )
			return false;

		_npt = Org_Nod;
		bool retval = _triCalc();

		if ( retval ) {
			_open = true;
			return true;
		} else {
			Release();
			return false;
		}
	}
    bool Triacalc() {
        return _triCalc();
    }

   DSM* SubCloud(const DPOINT& p0, const DPOINT& p1);
//{
//        PSLG* dsm = new PSLG;
//        dsm->_xmin = dsm->_ymin = dsm->_zmin = INF;
//        dsm->_xmax = dsm->_ymax = dsm->_zmax = -INF;
//		dsm->_npt = 0;
//		dsm->_nSize = 0;
//        if ( dsm == nullptr )
//            return dsm;

//        for( size_t i = 0; i < node.size(); i++) {
//            const NODE& nd = Node(i);
//            if ( nd.x > p0.x && nd.x < p1.x && nd.y > p0.y && nd.y < p1.y ) {
//                dsm->node.push_back(nd);
//                dsm->setMin(nd.x, nd.y, nd.z);
//                dsm->setMax(nd.x, nd.y, nd.z);
//            }
//        }

//        if ( !dsm->node.empty() ) {
//            dsm->Org_Nod = dsm->node.size();
//			dsm->_npt = dsm->Org_Nod;
//            dsm->_set_off_scale();
//            for (unsigned int i = 0; i < dsm->Org_Nod; i++)
//                dsm->_normalize(i);
//            if ( !dsm->Triacalc()) {
//                delete dsm;
//                return nullptr;
//            }
//            return dsm;
//        }
//        delete dsm;
//        return nullptr;
//    }

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

        for (UINT i = 0; i < (UINT) triangle.size(); i++) {
            int nb = 0;
            for (int j = 0; j < 3; j++) {
                nb += ( triangle[i].n[j] == -1 );
            }
            // scarta triangoli interni ed isolati
            if ( nb == 0 || nb == 3 )
                continue;
            SEGMENT sm;
            for (int j = 0; j < 3; j++) {
                // un solo lato di bordo
                if ( triangle[i].n[j] == -1 ) {
                    int ind = ( j - 1 >= 0 ) ? j - 1 : 2;
                    sm.p[0] = triangle[i].p[ind];
                    ind = ( j + 1 < 3 ) ? j + 1 : 0;
                    sm.p[1] = triangle[i].p[ind];
                    st.push_back(sm);

                }
            }
        }

        if ( st.size() == 0 )
            return 0;
        std::vector<UINT> vc;
        vc.reserve(st.size());
        vc.push_back(st[0].p[0]);
        vc.push_back(st[0].p[1]);
        st.erase(st.begin());
        while ( st.size() ) {
            UINT k = vc[vc.size() - 1];
            bool found = false;
            for (UINT i = 0; i < st.size(); i++) {
                if ( st[i].p[0] == k ) {
                    vc.push_back(st[i].p[1]);
                    st.erase(st.begin() + i);
                    found = true;
                    break;
                } else if ( st[i].p[1] == k ) {
                    vc.push_back(st[i].p[0]);
                    st.erase(st.begin() + i);
                    found = true;
                    break;
                }
            }
            if( !found ) {
                if( k == vc[0] ) {
                    for( UINT i = 0; i < vc.size(); i++ ) {
                        DPOINT p1 = Node( vc[i] );
                        vec.push_back( p1 );
                    }
                    vec.push_back( DPOINT() );
                    vc.clear();
                    if( !st.empty() ) {
                        vc.push_back( st[0].p[0] );
                        vc.push_back( st[0].p[1] );
                        st.erase( st.begin() );
                    }
                }
            }
        }
        for (UINT i = 0; i < vc.size(); i++) {
            DPOINT p1 = Node(vc[i]);
            vec.push_back(p1);
        }
        return vec.size();
    }

private:
	void	Plane(double* equ, int n) const {
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

		char mes[256];
        if ( Triangulate(&inData, &outData, mes) ) {

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

		_open = ret;
		return ret;
	}

	int		_trSign(double val) const {
		const double thr = _trEps;
		if ( fabs(val) < thr )
			return 0;
		return ( val < 0. ) ? -1 : 1;
	}
	int		_trTry(int* iTri, double xa, double ya) const {
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
		//void* _hg;
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
	mutable long	_lastTri;
	mutable int		_ltri;
	mutable double	_pln[4];
	mutable int		_nprev;
	mutable int		_nPrev, _sgn; // use by _trTry
	mutable RETTA	_rt[3];
	double	_trEps;
	int		_echo;
	int		_angle;
	File_Mask _fm;
	long	_nSize;
	bool	_open;
	DPOINT	_p_scale;
	DPOINT	_p_off;
	mutable ND		_nd;

	std::vector<ND>			node;
	std::vector<NT>			triangle;
	std::vector<SEGMENT>	seg;

	InertialEllipse _ie;
    my_kd_tree_t* myIndex;
    my_kd_tree_t2* myIndex2;
    PointCloud<float> cloud;
    PointCloud2D<float> cloud2;
};
typedef tPSLG<NODE, TRIANGLE> PSLG;

template <typename ND, typename NT> inline DSM* tPSLG<ND, NT>::SubCloud(const DPOINT& p0, const DPOINT& p1) {
    PSLG* dsm = new PSLG;
    dsm->_xmin = dsm->_ymin = dsm->_zmin = INF;
    dsm->_xmax = dsm->_ymax = dsm->_zmax = -INF;
    dsm->_npt = 0;
    dsm->_nSize = 0;
    if ( dsm == nullptr )
        return dsm;

    for( size_t i = 0; i < node.size(); i++) {
        const NODE& nd = Node(i);
        if ( nd.x > p0.x && nd.x < p1.x && nd.y > p0.y && nd.y < p1.y ) {
            dsm->node.push_back(nd);
            dsm->setMin(nd.x, nd.y, nd.z);
            dsm->setMax(nd.x, nd.y, nd.z);
        }
    }

    if ( !dsm->node.empty() ) {
        dsm->Org_Nod = dsm->node.size();
        dsm->_npt = dsm->Org_Nod;
        dsm->_set_off_scale();
        for (unsigned int i = 0; i < dsm->Org_Nod; i++)
            dsm->_normalize(i);
        if ( !dsm->Triacalc()) {
            delete dsm;
            return nullptr;
        }
        return dsm;
    }
    delete dsm;
    return nullptr;
}

#endif
