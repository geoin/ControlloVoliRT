/*=============================================================================
!   Filename:  DSM.H
!
!   Contents:	DSM class interface
!
!   History:
!			25/11/2011	created
!			20/06/2013	updated
!=================================================================================*/
#ifndef DSM_H
#define DSM_H

#include <string>
#include <set>
#include <vector>

#include "geom.h"
#include "exports.h"

#define Z_NOVAL -9999.
#define Z_OUT -8888.
#define DEM_EXT "nod"

class LASreader;

TOOLS_EXPORTS int sign(double val);

class TOOLS_EXPORTS MyLas {
public:
	enum Lidar_echo {
		all_pulses = 0,
		first_pulse = 1,
		last_pulse = 2,
		intermediate_pulse = 4
	};
	MyLas(): _lasreader(NULL) {}
	~MyLas();
	unsigned int open(const std::string& nome);
	bool get_next_point(DPOINT& p);
	int get_echo(void) const { return _echo; }
	void get_min(double& _xmin, double& _ymin, double& _zmin) const;
	void get_max(double& _xmax, double& _ymax, double& _zmax) const;
private:
	int _echo;
	LASreader* _lasreader;
};

// class for triangle
class TOOLS_EXPORTS TRIANGLE {
public:
	unsigned int	p[3]; // indice nella lista dei nodi
	int				n[3]; // triangoli adiacenti
};
// Break line description
class TOOLS_EXPORTS SEGMENT {
public:
	SEGMENT(void) {}
	SEGMENT(int p1, int p2) {
		p[0] = p1;
		p[1] = p2;
	}
	void Set(int p1, int p2) {
		p[0] = p1;
		p[1] = p2;
	}
	unsigned int p[2];
};
class TOOLS_EXPORTS File_Mask {
public:
	File_Mask(): nf(0), x(0), y(0), z(0), ne(0), ni(0){}
	File_Mask(int nfield, int xi, int yi, int zi, int nec, int nim):
		nf(nfield), x(xi), y(yi), z(zi), ne(nec), ni(nim) {}
	File_Mask(const File_Mask& fm):
		nf(fm.nf), x(fm.x), y(fm.y), z(fm.z), ne(fm.ne), ni(fm.ni) {}
	void operator=(const File_Mask& fm) {
		nf = fm.nf;
		x = fm.x;
		y = fm.y;
		z = fm.z;
		ne = fm.ne;
		ni = fm.ni;
	}
	int nf; // numero di campi dei record validi
	int x, y, z; // indice dei campi delle coordinate
	int ne;	// numero dell'echo
	int ni; // numero complessivo di echi
};

typedef DPOINT NODE;	// class for node

template <typename ND>
class TOOLS_EXPORTS _DSM {
public:
	enum Surface_Type {
		Surface_2D = 0,
		Surface_3D = 1
	};
	enum DSM_Type {
		DSM_UNKN = -1,
		DSM_GRID = 0,
		DSM_TIN = 1
	};
	_DSM(void): _xmin(0.), _ymin(0.), _zmin(0.), _xmax(0.), _ymax(0.), _zmax(0.), _z_noVal(Z_NOVAL) {}
	virtual ~_DSM() {}

	virtual double GetQuota(double x, double y, double z = Z_NOVAL, double zo = Z_OUT) const = 0;
	virtual double GetQm(double x, double y, double dx, double dy, double z = Z_NOVAL, double zo = Z_OUT) const = 0;
	virtual DSM_Type GetType(void) const = 0;
	virtual unsigned int Npt(void) const = 0;
	virtual unsigned int Ntriangle(void) const = 0;
	virtual unsigned int Nseg(void) const = 0;
	virtual double GetX(int i) const  = 0;
	virtual double GetY(int i) const = 0;
	virtual double GetZ(int i) const = 0;
	virtual const ND Node(unsigned int i) const = 0;
	virtual void Node(const DPOINT& p, unsigned int i) = 0;
	virtual const TRIANGLE& Triangle(unsigned int i) const = 0;
	virtual const SEGMENT& Segment(unsigned int i) const = 0;
	virtual int FindTriangle(double X, double Y, int trIdx = -1) const = 0;
	virtual bool Open(const std::string& nome, bool verbose, bool tria = true) = 0;
	virtual void Close(void) = 0;
	virtual int GetAdiacent(unsigned int tri, int nod, std::set<unsigned int>& lst) = 0;
	//virtual size_t GetBorder(std::vector<DPOINT>& vec) = 0;
	virtual bool RayIntersect(const DPOINT& p0, const DPOINT& p1, DPOINT& pt) const {
		// p0 centro proiezione, p1 vettore;

		VecOri P0(p0); // il centro di proiezione
		VecOri u(p1); // il vettore della direzione della retta
		VecOri n(0, 0, 1); // la normale al piano

		double z = pt.z;
		double x = pt.x;
		double y = pt.y;
		if ( z == _z_noVal )
			z = (_zmax + _zmin) / 2;
		double dz0 = 0;
		int nit = 15;
		double dz = 0., dx = 0, dy = 0;
		while ( --nit ) {
			VecOri V0(0, 0, z); // un punto del piano
			double s = n % (V0 - P0) /  (n % u);
			VecOri v = P0 + (u * s);
			double zt = GetQuota(v.GetX(), v.GetY());
			if ( zt == Z_OUT || zt == Z_NOVAL) {
				pt.z = zt;
				return false;
			}

			dz = fabs(zt - z);
			dx = fabs(v.GetX() - x);
			dy = fabs(v.GetY() - y);
			if ( dz < 0.1 ) {
				pt.x = v.GetX();
				pt.y = v.GetY();
				pt.z = z;
				break;
			}
			if ( nit == 1 && dz < dz0 )
				nit++;
			z = zt;
			x = v.GetX();
			y = v.GetY();
			dz0 = dz;
		}
		if ( nit == 0 ) {
			pt.x = x;
			pt.y = y;
			pt.z = z;
			return false;
		}
		return true;
	}
	virtual size_t GetBorder(std::vector<DPOINT>& vec) {
		vec.clear();
		std::vector<SEGMENT> st;

		for (unsigned int i = 0; i < Ntriangle(); i++) {
			int nb = 0;
			for (int j = 0; j < 3; j++) {
				nb += (Triangle(i).n[j]); //( triangle[i].n[j] == -1 );
			}
			if ( nb == 0 || nb == 3 ) // internal or isolated triangle
				continue;
			SEGMENT sm;
			for (int j = 0; j < 3; j++) {
				if ( nb == 1 && Triangle(i).n[j] == -1 ) {
					int ind = ( j - 1 >= 0 ) ? j - 1 : 2;
					sm.p[0] = Triangle(i).p[ind];
					ind = ( j + 1 < 3 ) ? j + 1 : 0;
					sm.p[1] = Triangle(i).p[ind];
					st.push_back(sm); // single border segment
					break;
				}
				if ( (nb == 2 && Triangle(i).n[j] != -1) ) {
					sm.p[0] = Triangle(i).p[j];
					int ind = ( j - 1 >= 0 ) ? j - 1 : 2;
					sm.p[1] = Triangle(i).p[ind];
					st.push_back(sm);
					ind = ( j + 1 < 3 ) ? j + 1 : 0;
					sm.p[1] = Triangle(i).p[ind];
					st.push_back(sm); // double border segment
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
		while ( st.size() ) { // build ordered list of vertexes
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
			DPOINT p1 = Node(vc[i]);
			vec.push_back(p1);
		}
		return vec.size();
	}

	virtual std::string Error_string(void) const { return _err_mes; }
	//virtual void SetEps(double eps) {}
	virtual void SetEcho(int echo) {}
	virtual void SetMask(const File_Mask& fm) {}

	double z_noVal(void) const { return _z_noVal; }
	bool IsValid(double z) const { return z != _z_noVal && z != Z_OUT; }
	bool IsInside(double z) const { return z != Z_OUT; }
	void z_noVal(double z) { _z_noVal = z; }
	double Xmin() const { return _xmin; }
	double Ymin() const { return _ymin; }
	double Zmin() const { return _zmin; }
	double Xmax() const { return _xmax; }
	double Ymax() const { return _ymax; }
	double Zmax() const { return _zmax; }
	void Xmin(double val) { _xmin = val; }
	void Ymin(double val) { _ymin = val; }
	void Zmin(double val) { _zmin = val; }
	void Xmax(double val) { _xmin = val; }
	void Ymax(double val) { _ymin = val; }
	void Zmax(double val) { _zmin = val; }

	virtual void getBB(DPOINT&, DPOINT&, DPOINT&, DPOINT&) {}
	virtual void getMajorAxis(DPOINT&, DPOINT&) {}
protected:
	std::string	_err_mes;
	double	_xmin, _ymin, _zmin;
	double	_xmax, _ymax, _zmax;
	double _z_noVal;
};

typedef _DSM<NODE> DSM;

class TOOLS_EXPORTS DSM_Grid: public DSM {
public:
	DSM_Grid(): _pos(0) {}
	double GetQuota(double x, double y, double z = Z_NOVAL, double zo = Z_OUT) const;
	double GetQm(double x, double y, double dx, double dy, double z = Z_NOVAL, double zo = Z_OUT) const;

	DSM_Type GetType(void) const { return DSM_GRID; }

	unsigned int Npt(void) const;
	unsigned int Ntriangle(void) const;
	unsigned int Nseg(void) const;
	const NODE Node(unsigned int i) const;
	const TRIANGLE& Triangle(unsigned int i) const;
	const SEGMENT& Segment(unsigned int i) const;
	void Node(const DPOINT& p, unsigned int i);
	double GetX(int i) const  { return _getX(i); }
	double GetY(int i) const { return _getY(i); }
	double GetZ(int i) const { return quote[i]; }

	int FindTriangle(double X, double Y, int trIdx = -1) const;
	bool Open(const std::string& nome, bool verbose, bool = false);
	bool Save(const std::string& nome);
	void Set(double xmin, double ymin, int nx, int ny, double stepx, double _z0);
	long nx(void) const { return _nx; }
	long ny(void) const { return _ny; }
	double stx(void) const { return _stx; }
	double sty(void) const { return _sty; }
	void Close(void);
	double Surface(int nTri, int type = 1) const;
	double Volume(int nTri, double qrif) const;
	int GetAdiacent(unsigned int tri, int nod, std::set<unsigned int>& lst);
	//void GetNormal(int n, VecOri* norm) const;
	//size_t GetBorder(std::vector<DPOINT>& vec);
	bool GetProperties(const std::string& nome);
	void AddQuote(float z);
	bool Merge(DSM_Grid& dsm1, DSM& dsm2, const std::string& nome);
	bool Diff(DSM_Grid& dsm1, DSM& dsm2, const std::string& nome);
	void Filter(void);

private:
	long _indexX(double X) const;
	long _indexY(double Y) const;
	long _indexX(long i) const;
	long _indexY(long i) const;
	double _getX(long i) const;
	double _getY(long i) const;
	long _index(double X, double Y) const;
	bool _check_tri(int tri);
	void _inc_tri(int ix, int iy, int icx, int icy, std::set<unsigned int>& lst);

	double				_stx, _sty;
	double				_x0, _y0, _z0;
	long				_nx, _ny;
	mutable TRIANGLE	_tr;
	mutable NODE		_nd;
	SEGMENT				_sg;
    long long				_pos;
	std::vector<float> quote;
};

class TOOLS_EXPORTS DSM_Factory {
public:
	DSM_Factory(void): _dsm(NULL), _eps(1.e-9), _lidar_echo(0) {}
	~DSM_Factory(void) {
		Close();
	}
	bool Open(const std::string nome, bool verbose = false, bool tria = true);
	//void SetEps(double eps) {
	//	_eps = eps;
	//	if ( _dsm != NULL )
	//		_dsm->SetEps(_eps);
	//}
	void SetEcho(int echo) {
		_lidar_echo = echo;
	}
	void SetMask(const File_Mask& fma) {
		fm = fma;
	}
	void Close(void);
	static DSM::DSM_Type GetType(const std::string& nome);
	DSM* GetDsm(void) { return _dsm; }
	std::string err_mes(void) const {
		if ( _dsm != NULL )
			return _dsm->Error_string();
		return "";
	}
private:
    int _lidar_echo;
    double _eps;
	File_Mask fm;
    DSM* _dsm;
};
#endif
