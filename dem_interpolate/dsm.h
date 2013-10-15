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

#define Z_NOVAL -99999.
#define Z_OUT -8888.
#define DEM_EXT "nod"

class DSM;
//class DPOINT;
//class VecOri;

TOOLS_EXPORTS int sign(double val);

class TOOLS_EXPORTS TRIANGLE { // class for triangle
public:
	//double Surface(const DSM* dsm, int type = 1) const;
	//double Volume(const DSM* dsm, double qrif) const;
	void GetNormal(const DSM* dsm, VecOri* norm) const;
	//double GetSlope(const DSM* dsm) const;
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


class Progress;
typedef DPOINT NODE;	// class for node

class TOOLS_EXPORTS DSM {
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
	DSM(void): _xmin(0.), _ymin(0.), _zmin(0.), _xmax(0.), _ymax(0.), _zmax(0.), _z_noVal(Z_NOVAL) {}
	virtual double GetQuota(double x, double y, double z = Z_NOVAL, double zo = Z_OUT) = 0;
	virtual DSM_Type GetType(void) const = 0;
	virtual unsigned int Npt(void) const = 0;
	virtual unsigned int Ntriangle(void) const = 0;
	virtual unsigned int Nseg(void) const = 0;
	virtual const NODE& Node(unsigned int i) const = 0;
	virtual const TRIANGLE& Triangle(unsigned int i) const = 0;
	virtual SEGMENT& Segment(unsigned int i) = 0;
	virtual int FindTriangle(double X, double Y, int trIdx = -1) = 0;
	virtual bool Open(const std::string& nome, bool verbose, Progress* prb = NULL) = 0;
	virtual void Close(void) = 0;
	//virtual double Surface(int nTri, int type = 1) const = 0;
	//virtual double Volume(int nTri, double qrif) const = 0;
	virtual int GetAdiacent(unsigned int tri, int nod, std::set<unsigned int>& lst) = 0;
	//virtual void GetNormal(int n, VecOri* norm) const = 0;
	virtual size_t GetBorder(std::vector<DPOINT>& vec) = 0;
	virtual bool RayIntersect(const DPOINT& p1, const DPOINT& p2, DPOINT& pt);

	virtual std::string Error_string(void) const { return _err_mes; }
	virtual void SetEps(double eps) {}
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
protected:
	std::string	_err_mes;
	double	_xmin, _ymin, _zmin;
	double	_xmax, _ymax, _zmax;
	double _z_noVal;

};
class TOOLS_EXPORTS DSM_Grid: public DSM {
public:
	DSM_Grid(): _pos(0) {}
	double GetQuota(double x, double y, double z = Z_NOVAL, double zo = Z_OUT);
	DSM_Type GetType(void) const { return DSM_GRID; }

	unsigned int Npt(void) const;
	unsigned int Ntriangle(void) const;
	unsigned int Nseg(void) const;
	const NODE& Node(unsigned int i) const;
	const TRIANGLE& Triangle(unsigned int i) const;
	SEGMENT& Segment(unsigned int i);
	int FindTriangle(double X, double Y, int trIdx = -1);
	bool Open(const std::string& nome, bool verbose, Progress* prb = NULL);
	bool Save(const std::string& nome, Progress* prb = NULL);
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
	size_t GetBorder(std::vector<DPOINT>& vec);
	bool GetProperties(const std::string& nome);
	void AddQuote(float z);
	bool Merge(DSM_Grid& dsm1, DSM& dsm2, const std::string& nome, Progress* prb = NULL);
	bool Diff(DSM_Grid& dsm1, DSM& dsm2, const std::string& nome, Progress* prb = NULL);
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
	__int64				_pos;
	std::vector<float> quote;
};

class TOOLS_EXPORTS DSM_Factory {
public:
	DSM_Factory(void): _dsm(NULL), _eps(1.e-3) {}
	~DSM_Factory(void) {
		Close();
	}
	bool Open(const std::string nome, bool verbose, Progress* prb = NULL);
	void SetEps(double eps) {
		_eps = eps;
		if ( _dsm != NULL )
			_dsm->SetEps(_eps);
	}
	void Close(void);
	static DSM::DSM_Type GetType(const std::string nome);
	DSM* GetDsm(void) { return _dsm; }
	std::string err_mes(void) const {
		if ( _dsm != NULL )
			return _dsm->Error_string();
		return "";
	}
private:
	double _eps;
	DSM* _dsm;
};
#endif
