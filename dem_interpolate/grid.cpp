/*=============================================================================
!   Filename:  DSM.H
!
!   Contents:	DSM class interface
!
!   History:
!			26/11/2011	created
!			20/06/2013	updated
!=================================================================================*/
#include "dsm.h"
#include "pslg.h"
//#include "txtfile.h"
#include <fstream>
#include "Poco/StringTokenizer.h"
#include "Poco/String.h"
#include "Poco/Buffer.h"
#include <algorithm>


//double TRIANGLE::Surface(const DSM* dsm, int type) const 
//{
//	double	len[3];
//
//	double sperim = 0.;	// perimeter
//	for (int i = 0; i < 3; i++) {
//		int i1 = (i + 1) % 3;
//		NODE nd1 =dsm->Node(p[i]);
//		NODE nd2 =dsm->Node(p[i1]);
//		if ( nd1.z == dsm->z_noVal() || nd2.z == dsm->z_noVal() )
//			return 0.;
//		double dx = nd1.x - nd2.x;
//		double dy = nd1.y - nd2.y;
//		double dz = nd1.z - nd2.z;
//		if ( type == DSM::Surface_3D )
//			len[i] = sqrt(dx * dx + dy * dy + dz * dz);	// 3d length of side i
//		else
//			len[i] = sqrt(dx * dx + dy * dy);	// 2D length of side i
//		sperim += len[i];
//	}
//	sperim /= 2.;
//
//	double surface = 1.;
//	for (int i = 0; i < 3; i++)
//		surface *= (sperim - len[i]);
//	surface = sqrt(sperim * surface);
//	return surface;
//}
//double TRIANGLE::Volume(const DSM* dsm, double qrif) const 
//{
//	double Ab = Surface(dsm, DSM::Surface_2D);
//	if ( Ab == 0. )
//		return 0.;
//	double z[3];
//	for (int i = 0; i < 3; i++) {
//		double zt = dsm->Node(p[i]).z;
//		if ( zt == dsm->z_noVal() )
//			return 0.;
//		z[i] = zt - qrif;
//	}
//
//	return Ab * (z[0] + z[1] + z[2]) / 3.;
//}
void TRIANGLE::GetNormal(const DSM* dsm, VecOri* norm) const 
{
	NODE n0 = dsm->Node(p[0]);
	NODE n1 = dsm->Node(p[1]);
	NODE n2 = dsm->Node(p[2]);
	VecOri v1(VecOri(n0) - VecOri(n2));
	VecOri v2(VecOri(n1) - VecOri(n2));
	*norm = v1 * v2;
	norm->Normalize();
}	
//double TRIANGLE::GetSlope(const DSM* dsm) const {
//	VecOri norm;
//	GetNormal(dsm, &norm);
//	if ( norm.GetZ() == 0. ) return 3.14 / 2;
//	double ta = sqrt(1 - norm.GetZ() * norm.GetZ()) / norm.GetZ();
//	return 100. * ta;
//}

/**************************************************************/
double DSM_Grid::GetQuota(double X, double Y, double Z, double zo)
{
	double znv = Z == Z_NOVAL ? _z_noVal : Z;
	double zout = zo == Z_OUT ? Z_OUT : zo;

	bool xok = true;
	if ( X < _xmin ) {
		if ( fabs(X - _xmin) < _stx / 10 ) {
			X = _xmin;
			//xok = true;
		} else
			xok = false;
	}
	if ( X > _xmax ) {
		if ( fabs(X - _xmax) < _stx / 10 ) {
			X = _xmax;
			//xok = true;
		} else
			xok = false;
	}
	bool yok = true;
	if ( Y < _ymin ) {
		if ( fabs(Y - _ymin) < _sty / 10 ) {
			Y = _ymin;
			//yok = true;
		} else
			yok = false;
	}
	if ( Y > _ymax ) {
		if ( fabs(Y - _ymax) < _sty / 10 ) {
			Y = _ymax;
			//yok = true;
		} else
			yok = false;
	}
	if ( !xok || !yok ) {
		return zout;
	}

	long ind = _index(X, Y);
	long count = _nx * _ny;

	double x = X - _getX(ind);
	double y = Y - _getY(ind);

	double q01 = quote[ind];
	if ( q01 == _z0 )
		return znv;
	if ( X != _xmax ) {
		double q2 = quote[ind + 1];
		if ( q2 == _z0 )
			return znv;
		q01 = q01 + (q2 - q01) * (x / _stx);
	}
	double q02 = q01;
	if ( Y != _ymax ) {
		q02 = quote[ind - _nx];
		if ( q02 == _z0 )
			return znv;
		if ( X != _xmax ) {
			double q2 = quote[ind - _nx + 1];
			if ( q2 == _z0 )
				return znv;
			q02 = q02 + (q2 - q02) * (x / _stx);
		}
	}

	Z = q01 + (q02 - q01) * (y / _sty);
	return Z;
}
const NODE& DSM_Grid::Node(unsigned int i) const
{
	_nd.x = _getX(i);
	_nd.y = _getY(i);
	_nd.z = quote[i];
	return _nd;
}
unsigned int DSM_Grid::Ntriangle(void) const
{
	return 2 * (_nx - 1) * (_ny - 1);
}
unsigned int DSM_Grid::Npt(void) const
{
	return _nx * _ny;
}
unsigned int DSM_Grid::Nseg(void) const
{
	return 0;
}
const TRIANGLE& DSM_Grid::Triangle(unsigned int i) const
{
	int b = 2 * (_nx - 1);
	long ix = (long) i % b;
	long iy = (long) i / b;
	_tr.p[0] = ix / 2 + _nx * (iy  + 1); // basso sinistra
	_tr.p[1] = _tr.p[0] + (i % 2 ? 1 : - _nx + 1); // alto destra  o basso destra
	_tr.p[2] = _tr.p[0] - _nx + (i % 2 ? 1 : 0); // alto sinistra
/*
    2   1    2
	----    /|  
	| /    / |
	|/    ----
	0     0  1
	     
*/
	if ( i % 2 ) {
		_tr.n[0] = i + 1;
		_tr.n[2] = i + b - 1;
		_tr.n[1] = i - 1;
	} else  {
		_tr.n[0] = i - b + 1;
		_tr.n[2] = i + 1;
		_tr.n[1] = i - 1;
	}
	for ( int k = 0; k < 3; k++) {
		if ( _tr.n[k] < 0 || _tr.n[k] >= (int) Ntriangle() )
			_tr.n[k] = -1;
	}
	return _tr;
}
SEGMENT& DSM_Grid::Segment(unsigned int i)
{
	return _sg;
}
long DSM_Grid::_indexX(double X) const
{
	return (long) ((X - _x0) / _stx);
}
long DSM_Grid::_indexY(double Y) const
{
	return (_ny - 1) - (long) ((Y - _y0) / _sty);
}
long DSM_Grid::_indexX(long i) const
{
	return i % _nx;
}
long DSM_Grid::_indexY(long i) const
{
	return i / _nx;
}
long  DSM_Grid::_index(double X, double Y) const
{
	return _nx * _indexY(Y) + _indexX(X);
}
double DSM_Grid::_getX(long i) const
{
	long  ix = _indexX(i);
	return _x0 + ix * _stx;
}
double DSM_Grid::_getY(long i) const
{
	long iy = _indexY(i);
	return _y0 + _sty * (_ny - 1 - iy);
}

int DSM_Grid::FindTriangle(double X, double Y, int trIdx)
{
	if ( X < _xmin || X > _xmax || Y < _ymin || Y > _ymax )
		return -1;
	long ix = (long) ((X - _x0) / _stx);
	long iy = (_ny - 2) - (long) ((Y - _y0) / _sty);
	if ( iy < 0 ) iy = 0;
	long tri = 2 * (iy * (_nx - 1) + ix) ;
	double x = X - ix * _stx - _x0;
	double y = Y - (_ny - 2 - iy) * _sty - _y0;
	if ( x != 0. ) {
		if ( y / x < _sty / _stx ) 
			tri += 1;
	}
	return tri;
}
void DSM_Grid::AddQuote(float z) 
{
	quote.push_back(z);
}

//double DSM_Grid::Surface(int nTri, int type) const
//{
//	return Triangle(nTri).Surface(this, type);
//}
//double DSM_Grid::Volume(int nTri, double qrif) const
//{
//	return Triangle(nTri).Volume(this, qrif);
//}
bool DSM_Grid::_check_tri(int tri) 
{
	return ( tri >= 0 && tri < (int) Ntriangle() );

}
void DSM_Grid::_inc_tri(int ix, int iy, int icx, int icy, std::set<unsigned int>& lst)
{
	ix += icx;
	if ( ix < 0 || ix >= 2 * (_nx - 1) )
		return;
	iy += icy;
	if ( iy < 0 || iy >= _ny - 1 )
		return;
	int tri = iy * 2 * (_nx - 1) + ix;
	if ( tri < 0 && tri >= (int) Ntriangle() )
		return;
	lst.insert(tri);
}
int DSM_Grid::GetAdiacent(unsigned int tri, int nod, std::set<unsigned int>& lst)
{
	lst.clear();
	int val = -1;
	int b = 2 * (_nx - 1);
	int ix = tri % b;
	int iy = tri / b;

	lst.insert(tri);
	if ( tri % 2 == 0 ) {
		if ( nod == 0 ) {
			_inc_tri(ix, iy, -1, 0, lst);
			_inc_tri(ix, iy,  1, 0, lst);
			_inc_tri(ix, iy,  0, 1, lst);
			_inc_tri(ix, iy, -1, 1, lst);
			_inc_tri(ix, iy, -2, 1, lst);
		} else if ( nod == 2 ) {
			_inc_tri(ix, iy, -1,  0, lst);
			_inc_tri(ix, iy, -2,  0, lst);
			_inc_tri(ix, iy, -1, -1, lst);
			_inc_tri(ix, iy,  0, -1, lst);
			_inc_tri(ix, iy,  1, -1, lst);
		} else if ( nod == 1 ) {
			_inc_tri(ix, iy,  1,  0, lst);
			_inc_tri(ix, iy,  2,  0, lst);
			_inc_tri(ix, iy,  1, -1, lst);
			_inc_tri(ix, iy,  2, -1, lst);
			_inc_tri(ix, iy,  3, -1, lst);
		}
	} else {
		if ( nod == 0 ) {
			_inc_tri(ix, iy, -1, 0, lst);
			_inc_tri(ix, iy, -2, 0, lst);
			_inc_tri(ix, iy, -1, 1, lst);
			_inc_tri(ix, iy, -2, 1, lst);
			_inc_tri(ix, iy, -3, 1, lst);
		} else if ( nod == 2 ) {
			_inc_tri(ix, iy, -1,  0, lst);
			_inc_tri(ix, iy,  1,  0, lst);
			_inc_tri(ix, iy,  0, -1, lst);
			_inc_tri(ix, iy,  1, -1, lst);
			_inc_tri(ix, iy,  2, -1, lst);
		} else if ( nod == 1 ) {
			_inc_tri(ix, iy,  1,  0, lst);
			_inc_tri(ix, iy,  2,  0, lst);
			_inc_tri(ix, iy, -1,  1, lst);
			_inc_tri(ix, iy,  0,  1, lst);
			_inc_tri(ix, iy,  1,  1, lst);
		}
	}
	return (int) lst.size();
}
//void DSM_Grid::GetNormal(int nTri, VecOri* norm) const
//{
//	Triangle(nTri).GetNormal(this, norm);
//}
size_t DSM_Grid::GetBorder(std::vector<DPOINT>& vec)
{
	return 0;
}
bool DSM::RayIntersect(const DPOINT& p0, const DPOINT& p1, DPOINT& pt)
{
	// p1 centro proiezione, p2 vettore;

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
		double zt = GetQuota(v.GetX(), v.GetY(), z);
		if ( zt == Z_OUT ) {
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
		//if ( dx < _stpx / 5 && dy < _stpy / 5 ) {
			pt.x = x;
			pt.y = y;
			pt.z = z;

		//} else
			return false;
	}
	return true;
}
bool DSM_Grid::Save(const std::string& nome, Progress* prb)
{
    std::fstream txf;
    txf.open(nome.c_str(), std::fstream::out | std::fstream::trunc);
    if ( !txf.is_open() )
		return false;
    txf.precision(18);
    txf << "ncols\t" << _nx << std::endl;
    txf << "nrows\t" << _ny << std::endl;
    txf << "xllcorner\t" << _x0 << std::endl;
    txf << "yllcorner\t" << _y0 << std::endl;
    txf << "cellsize\t" << _stx << std::endl;
    txf << "NODATA_value\t" << _z0 << std::endl;

    //txf.PrintNextRecord("ncols\t%d", _nx);
    //txf.PrintNextRecord("nrows\t%d", _ny);
    //txf.PrintNextRecord("xllcorner\t%.18lf", _x0);
    //txf.PrintNextRecord("yllcorner\t%.18lf", _y0);
    //txf.PrintNextRecord("cellsize\t%.18lf", _stx);
    //txf.PrintNextRecord("NODATA_value\t%lf", _z0);

    txf << std::endl;
    txf.precision(3);
	for (unsigned int i = 0; i < quote.size(); i++) {
        txf << quote[i] << " ";
        //txf.PrintNextRecord("%.3lf ", quote[i]);
	}
    txf << std::endl;
	return true;
}
void DSM_Grid::Set(double xmin, double ymin, int nx, int ny, double stepx, double z0)
{
	_nx = nx;
	_ny = ny;
	_x0 = xmin;
	_y0 = ymin;
	_z0 = z0;
	_stx = _sty = stepx;
}
bool DSM_Grid::GetProperties(const std::string& nome) 
{
    std::fstream txf;
    txf.open(nome.c_str(), std::fstream::in);
    if ( txf.is_open() )
		return false;
    char mes[256];
	int count = 0;
    while ( txf.getline(mes, 255) ) {
		Poco::StringTokenizer tok(mes, " \t", Poco::StringTokenizer::TOK_TRIM );
		if ( tok.count() != 2 )
			break;
		std::string keyword = Poco::toLower(tok[0]);
		if ( keyword == "ncols" ) {
			_nx = atoi(tok[1].c_str());
			count++;
		} else if ( keyword == "nrows" ) {
			_ny = atoi(tok[1].c_str());
			count++;
		} else if ( keyword == "xllcorner" || keyword == "xllcenter" ) {
			_x0 = atof(tok[1].c_str());
			count++;
		} else if ( keyword == "yllcorner" || keyword == "yllcenter" ) {
			_y0 = atof(tok[1].c_str());
			count++;
		} else if ( keyword == "nodata_value" ) {
			_z0 = atof(tok[1].c_str());
			count++;
		} else if ( keyword == "cellsize" ) {
			_stx = _sty =atof(tok[1].c_str());
			count++;
		}
		if ( count == 6 )
			break;
	}
    _pos = txf.tellp();
    txf.close();
	_xmin = _x0;
	_ymin = _y0;
	return true;
}
#ifdef STREAM
bool DSM_Grid::Open(const std::string& nome, bool verbose, Progress* prb)
{
	//Logger lg("c:\\temp\\grid.log");
	// acquisisce i parametri del grid
	if ( !GetProperties(nome) )
		return false;
	std::fstream fs;
	fs.open(nome);
	if ( !fs.is_open() )
		return false;
	fs.seekg(_pos, std::ios_base::beg);

	HourCur hS;
	Progress* prg = ( prb == NULL ) ? new Progress : prb;

	long count = 0, mcount = _nx * _ny;
	prg->Start(_nx * _ny);
	double zmin = INF, zmax = -INF;
	while ( !fs.eof() && ++count <= mcount ) {
		//count++;
		if ( !prg->Set((double) quote.size()) )
			break; // abortito dall'operatore
		double z;
		fs >> z;
		if ( !fs.eof() ) {
			quote.push_back( (float) z);
			if ( z != _z0 ) {
				zmin = min(zmin, z);
				zmax = max(zmax, z);
			}
		}
	}

	prg->Quit();
	//lg << "size" << _nx * _ny << " " << (long) quote.size() << "\n";
	if ( _nx * _ny != quote.size() ) {
		Close();
		return false;
	}
	_xmin = _x0;
	_ymin = _y0;
	_z_noVal = _z0;
	_zmin = zmin;
	_xmax = _x0 + _stx * (_nx - 1);
	_ymax = _y0 + _sty * (_ny - 1);
	_zmax = zmax;

	return true;
}
#else
bool DSM_Grid::Open(const std::string& nome, bool verbose, Progress* prb)
{
	//Logger lg("c:\\temp\\grid.log");
	// acquisisce i parametri del grid
	if ( !GetProperties(nome) )
		return false;
	FILE* fp = fopen(nome.c_str(), "r");
	if ( fp == NULL )
		return false;
    fseek(fp, _pos, SEEK_SET);

	//Progress* prg = ( prb == NULL ) ? new Progress : prb;

	long count = 0, mcount = _nx * _ny;
	//prg->Start(_nx * _ny);
	double zmin = INF, zmax = -INF;

	const int bfsz = 65536;
	Poco::Buffer<char> bf1(bfsz + 1);
	long nr;
	int of = 0;
	while ( (nr = fread(bf1.begin() + of, 1, bfsz - of, fp)) ) {
		//if ( !prg->Set((double) quote.size()) )
		//	break; // abortito dall'operatore
		char* c1 = bf1.begin();
		char* cl = bf1.begin() + nr + of;
		char* cp = c1;
		of = 0;
		while ( c1 < cl ) {
			if ( *c1 == ' ' || *c1 == '\n' ) {
				*c1 = '\0';
				if ( c1 != cp && *cp != '\0' ) {
					double z = atof(cp);
					quote.push_back( (float) z);
					if ( z != _z0 ) {
						zmin = std::min(zmin, z);
						zmax = std::max(zmax, z);
					}
				}
				cp = c1;
			} else {
				if ( *cp == '\0' )
					cp = c1;
			}
			c1++;
		}
		if ( cp != c1 ) {
			of = c1 - cp;
			memcpy(bf1.begin(), cp, of);
		}
	}

	fclose(fp);

	//prg->Quit();
	//if ( prb == NULL )
	//	delete prg;

	//lg << "size" << _nx * _ny << " " << (long) quote.size() << "\n";
	if ( _nx * _ny != quote.size() ) {
		Close();
		return false;
	}
	_xmin = _x0;
	_ymin = _y0;
	_z_noVal = _z0;
	_zmin = zmin;
	_xmax = _x0 + _stx * (_nx - 1);
	_ymax = _y0 + _sty * (_ny - 1);
	_zmax = zmax;

	return true;
}
#endif
void DSM_Grid::Close()
{
	quote.clear();
	_nx = _ny = 0;
}
bool DSM_Grid::Merge(DSM_Grid& dsm1, DSM& dsm2, const std::string& nome, Progress* prb)
{
	_x0 = std::min(dsm1.Xmin(), dsm2.Xmin());
	_y0 = std::min(dsm1.Ymin(), dsm2.Ymin());

	double xm = std::max(dsm1.Xmax(), dsm2.Xmax());
	double ym = std::max(dsm1.Ymax(), dsm2.Ymax());
	_nx = (int) ((xm - _x0) / dsm1.stx());
	_ny = (int) ((ym - _y0) / dsm1.sty());
	_stx = dsm1.stx();
	_sty = dsm1.sty();
	_z0 = dsm1.z_noVal();
	//Progress* prg = ( prb == NULL ) ? new Progress : prb;
	//prg->Start(_nx * _ny);

	bool error = false;

	try {
		for ( int i = 0; i < _ny; i++) {
			for ( int j = 0; j < _nx; j++) {
				long index = i * _nx + j;
				//prg->Set(index);
				double x = _getX(index);
				double y = _getY(index);
				double z1 = dsm1.GetQuota(x, y);
				double z2 = dsm2.GetQuota(x, y);
				if ( !dsm1.IsInside(z1) || z1 == dsm1.z_noVal() ) { // z1 non valido
					if ( !dsm2.IsInside(z2) || z2 == dsm2.z_noVal() ) // z2 non valido
						quote.push_back((float) _z0);
					else
						quote.push_back((float) z2);
				} else { // z1 valido
					if ( !dsm2.IsInside(z2) || z2 == dsm2.z_noVal() )
						quote.push_back((float) z1);
					else {
						double zt = (z1 + z2) / 2;
						quote.push_back((float) zt);				}
				}
			}
		}
	} catch (std::runtime_error e) {
		_err_mes = e.what();
		error = true;
	}
	catch( ... ) {
		_err_mes = "memory allocation error";
		error = true;
	}
	//prg->Quit();
	if ( !error )
		Save(nome);
	//if ( prb == NULL )
	//	delete prg;
	return !error;
}
bool DSM_Grid::Diff(DSM_Grid& dsm1, DSM& dsm2, const std::string& nome, Progress* prb)
{
	_x0 = std::max(dsm1.Xmin(), dsm2.Xmin());
	_y0 = std::max(dsm1.Ymin(), dsm2.Ymin());

	double xm = std::min(dsm1.Xmax(), dsm2.Xmax());
	double ym = std::min(dsm1.Ymax(), dsm2.Ymax());
	if ( xm <= _x0 || ym <= _y0 )
		return false; // DEM disgiunti
	_nx = (int) ((xm - _x0) / dsm1.stx());
	_ny = (int) ((ym - _y0) / dsm1.sty());
	_stx = dsm1.stx();
	_sty = dsm1.sty();
	_z0 = dsm1.z_noVal();

	bool error = false;
	try {
		for ( int i = 0; i < _ny; i++) {
			for ( int j = 0; j < _nx; j++) {
				long index = i * _nx + j;
				double x = _getX(index);
				double y = _getY(index);
				double z1 = dsm1.GetQuota(x, y);
				double z2 = dsm2.GetQuota(x, y);
				if ( z1 == dsm1.z_noVal() &&  z2 == dsm2.z_noVal() )
					quote.push_back((float) _z0);
				else if ( z1 != dsm1.z_noVal() &&  z2 == dsm2.z_noVal() )
					quote.push_back((float) _z0);
				else if ( z1 == dsm1.z_noVal() &&  z2 != dsm2.z_noVal() )
					quote.push_back((float) _z0);
				else
					quote.push_back((float) (z1 - z2));
			}
		}
	} catch ( ... ) {
		error = true;
	}
	if ( !error )
		Save(nome, prb);
	return !error;
}
/********************************************************/
bool DSM_Factory::Open(const std::string nome, bool verbose, Progress* prb)
{
	DSM::DSM_Type ty = GetType(nome);

	if ( ty == DSM::DSM_GRID ) {
		_dsm = new DSM_Grid;
		return _dsm->Open(nome, verbose, prb);
	}
	if ( ty == DSM::DSM_TIN ) {
		// file di tipo tin;
		_dsm = new PSLG;
		return _dsm->Open(nome, verbose, prb);
	}
	return false; // formato non riconosciuto
}
DSM::DSM_Type DSM_Factory::GetType(const std::string nome)
{
    std::fstream tf;
    tf.open(nome.c_str());
    if ( tf.is_open() ) {
        char mes[256];
        mes[0] = '\0';
        tf.getline(mes, 255);
        if ( !strlen(mes) )
			return DSM::DSM_UNKN;
		Poco::StringTokenizer tok(Poco::trim(std::string(mes)), ", \t", Poco::StringTokenizer::TOK_TRIM);
		if ( tok.count() > 1 ) {
			// file di tipo grid;
			return DSM::DSM_GRID;
		} else
			// file di tipo tin;
			return DSM::DSM_TIN;
	}
	return DSM::DSM_UNKN;
}
void DSM_Factory::Close()
{
	if ( _dsm != NULL )
		_dsm->Close();
	delete _dsm;
	_dsm = NULL;
}
