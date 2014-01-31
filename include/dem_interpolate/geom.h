#ifndef GEOM_H
#define GEOM_H

#include <algorithm>
#define  _USE_MATH_DEFINES
#include <math.h>
#include <stdexcept>
#include "exports.h"

#ifndef INF
#define INF	1.e30
#endif

#define DEG_RAD(x) M_PI * (x) / 180
#define RAD_DEG(x) 180 * (x) / M_PI

class DPOINT;
class VecOri;

class TOOLS_EXPORTS RETTA {
public:
	RETTA(): a(0.), b(0.), c(0.) {}
	RETTA(const DPOINT& p1, const DPOINT& p2) ;
	double	a, b, c;
	void X2Points(const DPOINT& p1, const DPOINT& p2);
	void X1PointPar(const DPOINT& p1, const RETTA& rt1);
	RETTA X1PointPar(const DPOINT& p1);
	void X1PointNor(const DPOINT& p1, const RETTA& rt1);
	RETTA X1PointNor(const DPOINT& p1);
	void X1PointHor(DPOINT& p1);
	double distPt(const DPOINT& pt);
	double OridistPt(const DPOINT& pt);
	DPOINT DistOnRetta(const DPOINT& p0, double d);
	double Slope(void);
	//virtual bool Intersez(RETTA& rt, DPOINT *point);
	//bool CircleIntersez(double xc, double yc, double R, DPOINT* pt);
	double val(const DPOINT& p);
	double val(double x, double y) {
		return a * x + b * y + c;
	}
	double valX(double y) {
		return ( a == 0. ) ? -1. : -(c + b * y) / a;
	}
	double valY(double x) {
		return ( b == 0. ) ? -1. : -(c + a * x) / b;
	}
};
class TOOLS_EXPORTS DPOINT { // 3D - point
public:
	enum PT_POS {
		PT_OUT = 0,
		PT_IN1 = 1,
		PT_IN2 = 2,
		PT_ON = 3
	};
	DPOINT(): x(0.), y(0.), z(0.) {}
	DPOINT(const DPOINT& p): x(p.x), y(p.y), z(p.z) {}
	DPOINT(double X, double Y, double Z = 0.): x(X), y(Y), z(Z) {}
	//void init() {
	//	x = y = z = -1;
	//}
	bool operator==(const DPOINT& pt1) {
		return ( fabs(pt1.x - x) < 1.e-6 && fabs(pt1.y - y) < 1.e-6 && fabs(pt1.z - z) < 1.e-6 );
	}
	bool operator!=(const DPOINT& pt1) {
		return ( fabs(pt1.x - x) > 1.e-6 || fabs(pt1.y - y) > 1.e-6 || fabs(pt1.z - z) < 1.e-6 );
	}
	// prodotto scalare
	double dot(const DPOINT& v) {
		return x * v.x + y * v.y + z * v.z;
	}
	// prodotto vettoriale
	DPOINT vec(const DPOINT& v) const {
		DPOINT w;
		w.x = y * v.z - z * v.y;
		w.y = z * v.x - x * v.z;
		w.z = x * v.y - y * v.x;
		return w;
	}
	void Normalize() {
		double len = sqrt(x * x + y * y + z * z);
		if ( len == 0. ) return;
		x /= len;
		y /= len;
		z /= len;
	}
	void set(double X, double Y, double Z = 0.) {
		x = X; y = Y; z = Z;
	}
	void operator=(const DPOINT& p) {
		x = p.x; y = p.y; z = p.z;
	}
	DPOINT operator+(const DPOINT& p) const {
		DPOINT p1(x + p.x, y + p.y, z + p.z);
		return p1;
	}
	void operator+=(const DPOINT& p) {
		x += p.x;
		y += p.y;
		z += p.z;
	}
	DPOINT operator-(const DPOINT& p) const {
		DPOINT p1(p.x - x, p.y - y, p.z - z);
		return p1;
	}
	void operator-=(const DPOINT& p) {
		x -= p.x;
		y -= p.y;
		z -= p.z;
	}
	DPOINT operator*(double d) const {
		DPOINT p1(x * d, y * d, z * d);
		return p1;
	}
	void operator*=(double n) {
		if ( n != 0 ) {
			x *= n;
			y *= n;
			z *= n;
		}
	}
	DPOINT operator/(double n) const {
		DPOINT p1;
		if ( n != 0 ) {
			p1.x = x / n;
			p1.y = y / n;
			p1.z = z / n;
		} else 
			p1 = *this;
		return p1;
	}
	void operator/=(double d) {
		if ( d != 0 ) {
			x /= d;
			y /= d;
			z /= d;
		}
	}
	void operator/=(const DPOINT& p) {
		if ( p.x != 0 )
			x /= p.x;
		if ( p.y != 0 )
			y /= p.y;
		if ( p.z != 0 )
			z /= p.z;
	}
	void operator*=(const DPOINT& p) {
		x *= p.x;
		y *= p.y;
		z *= p.z;
	}
	const double& operator[](int i) const {
		const double* v = &x;
		return v[i];
	}
	double& operator[](int i)  {
		double* v = &x;
		return v[i];
	}
	// assegna le coord alle variabili x, y, z
	void assign(double *X, double* Y, double* Z = NULL) {
		*X = x;
		*Y = y;
		if ( Z != NULL ) *Z = z;
	}
	//bool IsInsideSeg(const DPOINT& p1, const DPOINT& p2);
	// ritorna il punto sulla perp. a p1 this distante d da this
	//DPOINT Posd(const DPOINT& p1, double d) const;
	double dist2D(const DPOINT& ptb) const {
        return hypot(x - ptb.x, y - ptb.y);
	}
	double dist_lat2D(const DPOINT& ptb) const {
		double lon1 = DEG_RAD(x);
		double lat1 = DEG_RAD(y);
		double lon2 = DEG_RAD(ptb.x);
		double lat2 = DEG_RAD(ptb.y);
		double dlat = lat2 - lat1;
		double dlng = lon2 - lon1;
		double a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlng / 2) * sin(dlng / 2);
		double c = 2 * atan2(sqrt(a), sqrt(1 - a));
		double r = 6372.797; // mean radius of Earth in km
		return 1000 * r * c;
	}
	//double dist_lat3D(const DPOINT& ptb) const {
	//	double dp = dist_lat2D(ptb);
	//	double dz = z - ptb.z;
	//	return sqrt(dp * dp + dz * dz);
	//}
	// distanza bidimensionale
	double dist2D(double xp, double yp) const {
        return hypot(x - xp, y - yp);
	}
	// distanza tridimensionale
	double dist3D(const DPOINT& ptb) const {
		double dx = x - ptb.x;
		double dy = y - ptb.y;
		double dz = z - ptb.z;
		return sqrt(dx * dx + dy * dy + dz * dz);
	}
	// ritorna l'angolo rispetto al nord della retta this ptb
	double angdir(const DPOINT& ptb) const {
		double dx = ptb.x - x;
		double dy = ptb.y - y;
		const double eps = 1.e-10;
		double	alfa;
		if ( fabs(dy) > eps ) {
			alfa = atan(dx / dy);
			if ( dy < 0. )
				alfa = ( dx < 0. ) ? -M_PI + alfa : M_PI + alfa;
		} else {
			if ( fabs(dx) < eps )
				alfa = 0.;
			else
				alfa = ( dx > 0. ) ? M_PI / 2. : - M_PI / 2.;
		}
		return alfa;
	}
	double angdir2(const DPOINT& ptb) const {
		double dx = ptb.x - x;
		double dy = ptb.y - y;
		return atan2(dy, dx);
	}
	// ritorna vero se il pt coincide con un estremo
	//bool TouchPoint(const DPOINT& pt, double tol = EPS) const;
	// ritorna la posizione del punto rispetto al segmento
	//int IsPtInSeg(const DPOINT& pA1, const DPOINT& pA2, double tol = EPS) const;

	double x;
	double y;
	double z;
};
class TOOLS_EXPORTS VecOri {
public:
	friend class MatOri;
	VecOri() {
		p[0] = p[1] = p[2] = 0;
	}

	VecOri(const VecOri& v) {
		p[0] = v.p[0]; p[1] = v.p[1]; p[2] = v.p[2];
	}
	VecOri(const double x, const double y, const double z) {
		p[0] = x; p[1] = y; p[2] = z;
	}
	VecOri(const DPOINT& pt) {
		p[0] = pt.x; p[1] = pt.y; p[2] = pt.z;
	}
	void operator=(const VecOri& v) {
		p[0] = v.p[0]; p[1] = v.p[1]; p[2] = v.p[2];
	}
	VecOri operator-(const VecOri& v) {
		return VecOri(p[0] - v.p[0], p[1] - v.p[1], p[2] - v.p[2]);
	}
	VecOri operator+(const VecOri& v) {
		return VecOri(p[0] + v.p[0], p[1] + v.p[1], p[2] + v.p[2]);
	}
	const double& operator[](int i) const {
		return p[i];
	}
	double& operator[](int i) {
		return p[i];
	}
	bool operator==(const VecOri& v) {
		return ( p[0] == v.p[0] && p[1] == v.p[1] && p[2] == v.p[2] );
	}
	// prodotto scalare
	double operator%(const VecOri& v) {
		return p[0] * v.p[0] + p[1] * v.p[1] + p[2] * v.p[2];
	}
	// prodotto vettoriale
	VecOri operator*(const VecOri& v) {
		VecOri w;
		w.p[0] = p[1] * v.p[2] - p[2] * v.p[1];
		w.p[1] = p[2] * v.p[0] - p[0] * v.p[2];
		w.p[2] = p[0] * v.p[1] - p[1] * v.p[0];
		return w;
	}
	// prodotto per costante
	VecOri operator*(const double c) {
		VecOri w;
		w.p[0] = c * p[0];
		w.p[1] = c * p[1];
		w.p[2] = c * p[2];
		return w;
	}
	void Normalize() {
		double len = module();
		if ( len == 0. ) return;
		p[0] /= len;
		p[1] /= len;
		p[2] /= len;
	}
	double module(void) const {
		return sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
	}
    double GetX(void) const { return p[0]; }
    double GetY(void) const { return p[1]; }
    double GetZ(void) const { return p[2]; }
	void Set(double x, double y, double z) {
		p[0] = x;
		p[1] = y;
		p[2] = z;
	}
	void Set(double* pt) {
		Set(pt[0], pt[1], pt[2]);
	}
	void Get(double* pt) {
		pt[0] = p[0];
		pt[1] = p[1];
		pt[2] = p[2];

	}
	void Get(DPOINT& pt) const {
		pt.x = p[0];
		pt.y = p[1];
		pt.z = p[2];
	}
	double* Ptr(void) {
		return &p[0];
	}
protected:
	double p[3];
};
class TOOLS_EXPORTS MatOri {
public:
	MatOri() {
		memset(_mat, 0, sizeof(_mat));
	}
	MatOri(const MatOri& m) {
		mtmat(m._mat);
	}
	MatOri(double om, double fi, double ka) {
		mtor(om, fi, ka);
	}
	MatOri(const double m[3][3]) {
		mtmat(m);
	}
	MatOri(const double* m) {
		for (int i = 0, k = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				_mat[i][j] = m[k++];
			}
		}
	}
	MatOri(const int id) {
		memset(_mat, 0, sizeof(_mat));
		_mat[0][0] = _mat[1][1] = _mat[2][2] = id;
	}
	void operator=(const MatOri& m) {
		mtmat(m._mat);
	}
	void operator=(double m[3][3]) {
		mtmat(m);
	}
	// somma di matrici
	void operator+=(const MatOri& m) {
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				_mat[i][j] = _mat[i][j] + m._mat[i][j];
	}
	MatOri operator+(const MatOri& m) const {
		MatOri mt;
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				mt._mat[i][j] = _mat[i][j] + m._mat[i][j];
		return mt;
	}
	// prodotto matrice vettore
	VecOri operator*(const VecOri& v) const {
		VecOri w;
		for (int i =0; i < 3; i++) {
			w.p[i] = 0;
			for (int j = 0; j < 3; j++)
				w.p[i] += _mat[i][j] * v.p[j];
		}
		return w;

	}
	DPOINT operator*(const DPOINT& p) const {
		DPOINT p1;
		for (int i =0; i < 3; i++) {
			p1[i] = 0;
			for (int j = 0; j < 3; j++)
				p1[i] += _mat[i][j] * p[j];
		}
		return p1;

	}
	// ritorna la riga i come vettore
	VecOri GetRow(int i) {
		VecOri v(_mat[i][0], _mat[i][1], _mat[i][2]);
		return v;
	}
	// ritorna la colonna i come vettore
	VecOri GetCol(int i) const {
		VecOri v(_mat[0][i], _mat[1][i], _mat[2][i]);
		return v;
	}
	// imposta la riga i col vettore v
	void SetRow(VecOri v, int i) {
		_mat[i][0] = v[0];
		_mat[i][1] = v[1];
		_mat[i][2] = v[2];
	}
	// imposta la riga i col vettore v
	void SetCol(VecOri v, int i) {
		_mat[0][i] = v[0];
		_mat[1][i] = v[1];
		_mat[2][i] = v[2];
	}
	// imposta la matrice con vettori riga
	void SetRows(VecOri v0, VecOri v1, VecOri v2) {
		SetRow(v0, 0);
		SetRow(v1, 1);
		SetRow(v2, 2);
	}
	const double* operator[](int i) const {
		return &_mat[i][0];
	}
	double* operator[](int i) {
		return &_mat[i][0];
	}
	// imposta la matrice con vettori colonna
	void SetCols(VecOri v0, VecOri v1, VecOri v2) {
		SetCol(v0, 0);
		SetCol(v1, 1);
		SetCol(v2, 2);
	}
	// prodotto riga colonna
	MatOri operator*(const MatOri& m) const {
		double	_mt[3][3];
	
		for (int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				_mt[i][j] = 0.;
				for (int k = 0; k < 3; k++)
					_mt[i][j] += _mat[i][k] * m._mat[k][j];	// mat * m1
			}
		}
		return _mt;
	}
	void Reset(void) {
		for (int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				_mat[i][j] = 0.;
			}
		}
	}
	void operator *=(const MatOri& m) {
		double	_mt[3][3];
	
		for (int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				_mt[i][j] = 0.;
				for (int k = 0; k < 3; k++)
					_mt[i][j] += _mat[i][k] * m._mat[k][j];	// mat * m1
			}
		}
		mtmat(_mt);
	}
	// prodotto matrice per scalare
	void operator*=(const double c) {
		for (int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				_mat[i][j] *= c;
			}
		}
	}
	//bool operator ==(const MatOri& m) {
	//	return ( MINDIF(_mat[0][0], m._mat[0][0]) && MINDIF(_mat[0][1], m._mat[0][1]) && MINDIF(_mat[0][2], m._mat[0][2]) &&
	//			 MINDIF(_mat[1][0], m._mat[1][0]) && MINDIF(_mat[1][1], m._mat[1][1]) && MINDIF(_mat[1][2], m._mat[1][2]) &&
	//			 MINDIF(_mat[2][0], m._mat[2][0]) && MINDIF(_mat[2][1], m._mat[2][1]) && MINDIF(_mat[2][2], m._mat[2][2]) );
	//}
    MatOri Transpose(void) const {
		MatOri mt;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++)
				mt._mat[i][j] = _mat[j][i];
		}
		return mt;
	}
	MatOri Invert(void) const {
		MatOri mt;
		VecOri v0 = GetCol(0);
		VecOri v1 = GetCol(1);
		VecOri v2 = GetCol(2);
		VecOri v3 = v1 * v2;

		mt.SetRows( v3, v2 * v0, v0 * v1);
		double det = v0 % v3 ;
		if ( fabs(det) < 1.e-12 )
            throw std::runtime_error("non invertibile");
		mt *= (1. / det); 
		return mt;
	}
	void assign(double m[3][3]) {
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				m[i][j] = _mat[i][j];
	}
	void assign(double* m) {
		for (int i = 0, k = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				m[k++] = _mat[i][j];
	}
	void SetTranspose(double m[3][3]) {
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				_mat[i][j] = m[j][i];
	}
	void Rotate(double om, double fi, double ka) {
		MatOri mr(om, fi, ka);
		mr *= *this;
		*this = mr;
	}
	void Rotate(const MatOri& mt) {
		MatOri mr(mt);
		mr *= *this;
		*this = mr;
	}
	void mtor(double om, double fi, double ka) {
		_mat[0][0] = cos(fi) * cos(ka);
		_mat[0][1] = -(cos(om) * sin(ka) + sin(om) * sin(fi) * cos(ka));
		_mat[0][2] = -sin(om) * sin(ka) + cos(om) * sin(fi) * cos(ka);
		
		_mat[1][0] = cos(fi) * sin(ka);
		_mat[1][1] = cos(om) * cos(ka) - sin(om) * sin(fi) * sin(ka);
		_mat[1][2] = sin(om) * cos(ka) + cos(om) * sin(fi) * sin(ka);
		
		_mat[2][0] = -sin(fi);
		_mat[2][1] = -sin(om) * cos(fi);
		_mat[2][2] = cos(om) * cos(fi);
	}
	void ormt(double* om, double* fi, double* ka) {
		*ka = atan(_mat[1][0] / _mat[0][0]);
		*om = atan(-_mat[2][1] / _mat[2][2]);
		*fi = atan((-_mat[2][0] * cos(*ka)) / _mat[0][0]);

		if ( -sin(*fi) * _mat[2][0] < 0 )
			*fi = -*fi;
	
		if ( cos(*fi) * cos(*ka) * _mat[0][0] < 0 )
			*ka = *ka - M_PI;
		if ( cos(*fi) * sin(*ka) * _mat[1][0] < 0 )
			*ka = -*ka;

		if (cos(*fi) * cos(*om) * _mat[2][2] < 0)
			*om = M_PI - *om;
		if (-cos(*fi) * sin(*om) * _mat[2][1] < 0)
			*om = -*om;
	}
private:
	double _mat[3][3];
	void mtmat(const double m[3][3]) {
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				_mat[i][j] = m[i][j];
	}
};
class MBR {
public:
	MBR() {
		Reset();
	}
	MBR(double x0, double y0, double x1, double y1) {
		set(x0, y0, x1, y1);
	}
	MBR(MBR& mbr) {
		set(mbr.GetMinX(), mbr.GetMinY(), mbr.GetMaxX(), mbr.GetMaxY());
	}
	void set(double x0, double y0, double x1, double y1) {
		Xi = x0; Yi = y0;
		Xs = x1, Ys = y1;
	}
	void Reset(void) {
		Xi = INF; Yi = INF;
		Xs = -INF; Ys = -INF;
	}
	void Update(double x, double y) {
		Xi = std::min(Xi, x);
		Yi = std::min(Yi, y);
		Xs = std::max(Xs, x);
		Ys = std::max(Ys, y);
	}
	void Assign(double* xi0, double* yi0, double* xi1, double* yi1) {
		*xi0 = Xi; *yi0 = Yi;
		*xi1 = Xs; *yi1 = Ys;
	}
	void GetMin(double* xi, double* yi) const {
		*xi = Xi; *yi = Yi;
	}
	void GetMax(double* xi, double* yi) const {
		*xi = Xs; *yi = Ys;
	}
	double GetDx() {
		return fabs(Xs - Xi);
	}
	double GetDy() const {
		return fabs(Ys - Yi);
	}
	double GetMinX() const {
		return Xi;
	}
	double GetMinY() const {
		return Yi;
	}
	double GetMaxX() const {
		return Xs;
	}
	double GetMaxY() const {
		return Ys;
	}
	bool IsInside(double x, double y) const {
		if ( x < Xi || x > Xs || y < Yi || y > Ys )
			return false;
		return true;

	}
	bool Disjoint(const MBR& mbr) const {
		if ( Xi > mbr.Xs || Xs < mbr.Xi ||
			Yi > mbr.Ys || Ys < mbr.Yi ) 
			return true;
		return false;
	}
	bool Intersect(const MBR& mbr) {
		if ( Disjoint(mbr) )
			return false;
		Xi = std::max(Xi, mbr.GetMinX());
		Xs = std::min(Xs, mbr.GetMaxX());
		Yi = std::max(Yi, mbr.GetMinY());
		Ys = std::min(Ys, mbr.GetMaxY());
		return true;
	}
	void Extend(MBR& mbr) {
		Xi = std::min(Xi, mbr.GetMinX());
		Yi = std::min(Yi, mbr.GetMinY());
		Xs = std::max(Xs, mbr.GetMaxX());
		Ys = std::max(Ys, mbr.GetMaxY());
	}
	void Inflate(double dx, double dy) {
		Xi -= dx; Xs += dx;
		Yi -= dy; Ys += dy;
		
	}
private:
	double Xi, Yi; // lower limit
	double Xs, Ys; // upper limit
};

#endif
