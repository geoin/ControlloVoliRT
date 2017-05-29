/*=============================================================================
!   Filename:  statistics.h
!
!   Contents:	statistic value estimator
!
!   History:
!			11/09/2015	created
!			11/09/2015	updated
!=============================================================================*/
#pragma once
#include <vector>

class statistics {
public:
    statistics(): _size(0), _count(0) {}
    statistics(const statistics& st) : _size(st._size), _count(st._count),
        _ev(st._ev), _ev2(st._ev2), _max(st._max), _min(st._min),
        _amax(st._amax), _amin(st._amin) {}
	statistics(int size) : _size(size), _count(0) {
		_ev.resize(size);
		_ev2.resize(size);
		_max.resize(size);
		_min.resize(size);
		_amax.resize(size);
		_amin.resize(size);
		for ( size_t i = 0; i < size; i++ ) {
			_max[i] = -10e30;
			_min[i] = 10e30;
			_amax[i] = -10e30;
			_amin[i] = 10e30;
		}
	}
    statistics operator=(const statistics& st) {
        _size = st._size;
        _count = st._count;
        _ev = st._ev;
        _ev2 = st._ev2;
        _max = st._max;
        _min = st._min;
        _amax = st._amax;
        _amin = st._amin;
        return *this;
    }

	int add_point(const std::vector<double>& v) {
		if ( v.size() != _size )
			return -1;
		double cr = (double) _count;
		double vc = (double) ++_count;
		//double cr = (vc - 1) / vc;
		for ( int i = 0; i < _size; i++ ) {
			_ev[i] = (cr * _ev[i] + v[i]) / vc;
			_ev2[i] = (cr * _ev2[i] + v[i] * v[i]) / vc;
			_max[i] = (std::max)(_max[i], v[i]);
			_min[i] = (std::min)(_min[i], v[i]);
			_amax[i] = (std::max)(_amax[i], fabs(v[i]));
			_amin[i] = (std::min)(_amin[i], fabs(v[i]));
		}
		return _count;
	}
	std::vector<double> mean(void) const {
		return _ev;
	}
	double meanS( void ) const
	{
		double vret = 0.;
		for (int i = 0; i < _size; i++)
			vret += (_ev[i] * _ev[i]);
		return sqrt( vret);
	}
	std::vector<double> Min(void) const {
		return _min;
	}
	std::vector<double> Max(void) const {
		return _max;
	}
	std::vector<double> aMin(void) const {
		return _amin;
	}
	std::vector<double> aMax(void) const {
		return _amax;
	}	
	double aMaxS( void ) const
	{
		double vret = _amax[0];
		for (int i = 0; i < _size; i++)
			vret = std::max( vret, _amax[i] );
		return vret;
	}
	std::vector<double> variance(void) const {
		std::vector<double> var(_size);
		for ( int i = 0; i < _size; i++ ) {
			var[i] = _ev2[i] - _ev[i] * _ev[i];
		}
		return var;
	}
	std::vector<double> varianceC(void) const {
		std::vector<double> var = variance();
		double cr = (double) _count / (_count - 1.);
		for ( int i = 0; i < _size; i++ ) {
			var[i] = cr * var[i];
		}
		return var;
	}
	std::vector<double> st_dev(void) const {
		std::vector<double> var = variance();
		for ( int i = 0; i < _size; i++ ) {
			var[i] = sqrt(var[i]);
		}
		return var;
	}
	double st_devS( void )
	{
		std::vector<double> ds = st_dev();
		double vret = 0.;
		for (int i = 0; i < _size; i++)
			vret += (ds[i] * ds[i]);
		return sqrt( vret );
	}

    double scale(void) const {
        std::vector<double> var = variance();
        double ret = 0;
        for (std::vector<double>::iterator it = var.begin(); it != var.end(); it++ ){
            double& v = *it;
            ret += v;
        }
        return sqrt(ret/_size);
    }

	std::vector<double> interval(double perc) const {
		std::vector<double> var = varianceC();
		double cr = 1. / sqrt((double) _count);
		for ( int i = 0; i < _size; i++ ) {
			var[i] = cr * perc * var[i];
		}
		return var;
	}
	long count(void) const {
		return _count;
	}
	void clear( int sz = 0)
	{
		if ( sz > 0 )
			_size = sz;
		_count = 0;
		_ev.clear(); _ev.resize( _size );
		_max.clear(); _max.resize( _size );
		_amax.clear(); _amax.resize( _size );
		_min.clear(); _min.resize( _size );
		_amin.clear(); _amin.resize( _size );
		_ev2.clear(); _ev2.resize(_size);
		for (size_t i = 0; i < _size; i++) {
			_max[i] = -10e30;
			_min[i] = 10e30;
			_amax[i] = -10e30;
			_amin[i] = 10e30;
		}
	}
	void FullReport( std::ostream& os )
	{
		for (size_t i = 0; i < _size; i++) {
			os << "DIMENSION : " << i << "------------------" << std::endl;
			os << "\tMean: " << _ev[i] << std::endl;
			os << "\tMax: " << _amax[i] << std::endl;
			os << "\tStdev: " << st_dev()[i] << std::endl;
		}
	}
    void ShortReport(std::ostream& os) {
        os << "\tMean: " << meanS() << std::endl;
        os << "\tMax: " << aMaxS() << std::endl;
		os << "\tStdev: " << st_devS() << std::endl;
	}

private:
	int _size;
	long _count;
	std::vector<double> _ev;
	std::vector<double> _max, _amax;
	std::vector<double> _min, _amin;
	std::vector<double> _ev2;
};
