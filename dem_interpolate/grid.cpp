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
#include <fstream>
#include <iostream>
#include "Poco/StringTokenizer.h"
#include "Poco/String.h"
#include "Poco/Buffer.h"
#include <algorithm>

#include "laslib/lasreader.hpp"

#ifdef WIN32 //TODO (TEMP): needs to update win version
#define LAS_NUMBER_OF_RETURNS number_of_returns_of_given_pulse
#else
#define LAS_NUMBER_OF_RETURNS number_of_returns
#endif

MyLas::~MyLas() 
{
	if ( _lasreader != NULL )
		_lasreader->close();
	delete _lasreader;

}
unsigned int MyLas::open(const std::string& nome)
{
	LASreadOpener lasreadopener;
	lasreadopener.set_file_name(nome.c_str());
	if ( !lasreadopener.active() )
		return 0;
	_lasreader = lasreadopener.open();
	if ( _lasreader == NULL )
		return 0;
	return (unsigned int) _lasreader->npoints;
}
bool MyLas::get_next_point(DPOINT& p)
{
    _isvalid = true;
	if ( _lasreader->read_point() ) {
        if( _lasreader->point.LAS_NUMBER_OF_RETURNS <= 0 ) {
            _isvalid = false;
            return true;
        }
		p.x = _lasreader->get_x();
		p.y = _lasreader->get_y();
		p.z = _lasreader->get_z();
		_echo = 0;
		if ( _lasreader->point.return_number == 1 )
            _echo |= first_pulse; // first echo
        if ( _lasreader->point.return_number == _lasreader->point.LAS_NUMBER_OF_RETURNS )
            _echo |= last_pulse; // last echo
        if ( _lasreader->point.return_number > 1 && _lasreader->point.return_number < _lasreader->point.LAS_NUMBER_OF_RETURNS )
            _echo = intermediate_pulse;

		_angle = int( _lasreader->point.scan_angle_rank );

		return true;
	}
	return false;
}
void MyLas::get_min(double& _xmin, double& _ymin, double& _zmin) const
{
	_xmin = _lasreader->get_min_x();
	_ymin = _lasreader->get_min_y();
	_zmin = _lasreader->get_min_z();

}
void MyLas::get_max(double& _xmax, double& _ymax, double& _zmax) const
{
	_xmax = _lasreader->get_max_x();
	_ymax = _lasreader->get_max_y();
	_zmax = _lasreader->get_max_z();
}

/**************************************************************/
double DSM_Grid::GetQuota(double X, double Y, double Z, double zo) const
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
	//long count = _nx * _ny;

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
double DSM_Grid::GetQm(double x, double y, double dx, double dy, double z, double zo) const
{
	return GetQuota(x, y, zo);
}

const NODE DSM_Grid::Node(unsigned int i) const
{
	_nd.x = _getX(i);
	_nd.y = _getY(i);
	_nd.z = quote[i];
	return _nd;
}
void DSM_Grid::Node(const DPOINT& p, unsigned int i)
{
	 quote[i] = (float) p.z;
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
const SEGMENT& DSM_Grid::Segment(unsigned int i) const
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

int DSM_Grid::FindTriangle(double X, double Y, int trIdx) const
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
	//int val = -1;
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

//size_t DSM_Grid::GetBorder(std::vector<DPOINT>& vec)
//{
//	return 0;
//}

bool DSM_Grid::Save(const std::string& nome)
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
	txf.open(nome.c_str(), std::fstream::in | std::fstream::binary);
    if ( !txf.is_open() )
		return false;
    char mes[256];
	int count = 0;
	bool center = false;
    while ( txf.getline(mes, 255) ) {
		Poco::StringTokenizer tok(mes, " \t", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY );
		if ( tok.count() != 2 )
			break;
		std::string keyword = Poco::toLower(tok[0]);
		if ( keyword == "ncols" ) {
			_nx = atoi(tok[1].c_str());
			count++;
		} else if ( keyword == "nrows" ) {
			_ny = atoi(tok[1].c_str());
			count++;
		} else if ( keyword == "xllcorner" ) {
			_x0 = atof(tok[1].c_str());
			count++;
		} else if ( keyword == "xllcenter" ) {
			center = true;
			_x0 = atof(tok[1].c_str());
			count++;
		} else if ( keyword == "yllcorner" ) {
			_y0 = atof(tok[1].c_str());
			count++;
		} else if ( keyword == "yllcenter" ) {
			center = true;
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
	if ( count != 6)
		return false; // invalid format
	if ( !center ) {
		_x0 += _stx / 2;
		_y0 += _sty / 2;
	}
    _pos = txf.tellp();
    txf.close();
	_xmin = _x0;
	_ymin = _y0;
	return true;
}

bool DSM_Grid::Open(const std::string& nome, bool verbose, bool)
{
	// acquisisce i parametri del grid
	if ( !GetProperties(nome) )
		return false;
    FILE* fp = fopen(nome.c_str(), "rb");
	if ( fp == NULL )
		return false;
    fseek(fp, (long) _pos, SEEK_SET);

	long count = 0/*, mcount = _nx * _ny*/;
	double zmin = INF, zmax = -INF;

	const int bfsz = 65536;
	Poco::Buffer<char> bf1(bfsz + 1);
	long nr;
	int of = 0;
	while ( (nr = fread(bf1.begin() + of, 1, bfsz - of, fp)) ) {
		char* c1 = bf1.begin();
		char* cl = bf1.begin() + nr + of;
		char* cp = c1;
		of = 0;
        while ( c1 < cl ) {
            if ( *c1 == ' ' || *c1 == '\r' || *c1 == '\n') {
				*c1 = '\0';
                if ( c1 != cp && *cp != '\0' && *c1 != '\r' && *c1 != '\n') {
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

    int actual = quote.size();
    int wanted = _nx * _ny;
    if ( wanted != actual ) {
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
void DSM_Grid::Close()
{
	quote.clear();
	_nx = _ny = 0;
}
bool DSM_Grid::Merge(DSM_Grid& dsm1, DSM& dsm2, const std::string& nome)
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
bool DSM_Grid::Diff(DSM_Grid& dsm1, DSM& dsm2, const std::string& nome)
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
		Save(nome);
	return !error;
}
/********************************************************/
bool DSM_Factory::Open(const std::string nome, bool verbose, bool tria)
{
	DSM::DSM_Type ty = GetType(nome);

	if ( ty == DSM::DSM_GRID ) {
		_dsm = new DSM_Grid;
		return _dsm->Open(nome, verbose, tria);
	}
	if ( ty == DSM::DSM_TIN ) {
		// file di tipo tin;
		_dsm = new PSLG;
		_dsm->SetEcho(_lidar_echo);
		_dsm->SetMask(fm);
        _dsm->SetAngle( _lidar_angle / 2 );
        std::cout << "!!!!! Reading with angle of: " << _lidar_angle / 2 << " Echo " << _lidar_echo << std::endl;

		return _dsm->Open(nome, verbose, tria);
	}
	return false; // formato non riconosciuto
}
DSM::DSM_Type DSM_Factory::GetType(const std::string& nome)
{
    std::fstream tf;
    tf.open(nome.c_str(), std::fstream::in);
    if ( tf.is_open() ) {
        char mes[256];
        mes[0] = '\0';
        tf.getline(mes, 255);
		tf.close();
        if ( !strlen(mes) )
			return DSM::DSM_UNKN;
		Poco::StringTokenizer tok(Poco::trim(std::string(mes)), ", \t", Poco::StringTokenizer::TOK_TRIM);
		if ( tok.count() > 1 &&
			( !Poco::icompare(tok[0], "NCOLS") || !Poco::icompare(tok[0], "NROWS") || !Poco::icompare(tok[0], "CELLSIZE") || 
			!Poco::icompare(tok[0], "XLLCENTER") || !Poco::icompare(tok[0], "YLLCENTER") || !Poco::icompare(tok[0], "NODATA_VALUE") ) ) {
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
    if ( _dsm != NULL ) {
		_dsm->Close();
        delete _dsm;
    }
	_dsm = NULL;
}
