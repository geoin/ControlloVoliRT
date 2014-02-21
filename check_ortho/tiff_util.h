/*
    File: tiff_util.h
    Author:  F.Flamigni
    Date: 2013 November 25
    Comment:

    Disclaimer:
        This file is part of RT_Controllo_Voli.

        Tabula is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        Tabula is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with Tabula.  If not, see <http://www.gnu.org/licenses/>.


        Copyright (C) 2013 Geoin s.r.l.

*/
#ifndef TIFF_UTIL_H
#define TIFF_UTIL_H

#include "dem_interpolate/geom.h"
#include "geotiff/xtiffio.h"
#include "geotiff/geotiffio.h"
#include <string>
#include <vector>

class TFW {
public:
	TFW(): _dimx(0), _dimy(0) {}
	TFW(const std::string& nm, const std::string& ext = "tif");
	DPOINT ter_img(const DPOINT& ter) const;
	DPOINT img_ter(const DPOINT& pix) const;
	bool read(const std::string& nome);
	double pix_size(void) const;
	long dimx(void) const;
	long dimy(void) const;
	const std::string& nome(void) const;
private:
	MatOri _pix_ter;
	MatOri _ter_pix;
	std::string _nome, _ext;
	long _dimx, _dimy;
};
class rgb {
public:
	rgb(): r(0), g(0), b(0) {}
	rgb(const rgb& col): r(col.r), g(col.g), b(col.b) {}
	rgb(unsigned char re, unsigned char gr, unsigned char bl): r(re), g(gr), b(bl) {}
	rgb(unsigned char v): r(v), g(v), b(v) {}
	unsigned char r, g, b;
	bool operator==(const rgb& col) const {
		return r == col.r && g == col.g && b == col.b;
	}
	void operator=(const rgb& col) {
		r = col.r; g = col.g; b = col.b;
	}
	bool is_black(void) const {
		return r == 0 && g == 0 && b == 0;
	}
	bool is_white(void) const {
		return r == 255 && g == 255 && b == 255;
	}
};
class CV_image {
public:
	enum IMG_TYPE {
		ty_bilevel = 0,
		ty_gray = 1,
		ty_rgb = 2,
		ty_palette = 3,
		ty_notsupported = -1
};
    CV_image(): _tif(NULL), col(NULL), bf(NULL), _dimx(0), _dimy(0) {
		_prevW = TIFFSetWarningHandler(NULL);
		_prevE = TIFFSetErrorHandler(NULL);
	}
	~CV_image();
	bool open(const std::string& nome);
	void close(void);
	long dimx(void) const { return _dimx; }
	long dimy(void) const { return _dimy; }
	bool is_type(IMG_TYPE ty) const { return _type == ty; }
	bool is_black(int i) const {
		if ( _type == ty_rgb )
			return col[i].is_black();
		else if ( _type == ty_gray )
			return bf[i] == 0;
		return false;
	}
	bool is_white(int i) const {
		if ( _type == ty_rgb )
			return col[i].is_white();
		else if ( _type == ty_gray )
			return bf[i] == 255;
		return false;
	}
	bool read_line(int line);

	unsigned char* bf;
	rgb* col;
private:
	TIFF* _tif ;
	long _dimx, _dimy;
	long _nbands;
	long _bit_sample;
	IMG_TYPE _type;
	TIFFErrorHandler _prevW;
	TIFFErrorHandler _prevE;
};

class Border {
public:
	Border(int X, int Y): x(X), y(Y), used(false) {}
	int		x, y;  // img coord of pixel
	bool	used;  // flag used
	void Set(int _x, int _y) {
		x = _x;
		y = _y;
		used = false;
	}
};

class BorderLine {
public:
	BorderLine(void): _background(0) {}
	bool Evaluate(const std::string& nome, std::vector<DPOINT>& pt);
private:
	int _getBackGround(void);
	bool _nextPtInChain(std::vector<Border>& chain, Border* prev, unsigned int *index);
	void _distLine(const Border& p1, const Border& p2, RETTA *retta);
	bool _isInSide(const std::vector<Border>& bd, std::vector<unsigned int>& rt, unsigned int lt);
	void _setPixInfo(int line);
	bool _buildChain(std::vector<Border>& bd);
	bool _getBorderLine(std::vector<DPOINT>& pt);
	void _showborder(const std::vector<Border>& bord);

	CV_image _img;
	int	_background;
};

#endif
