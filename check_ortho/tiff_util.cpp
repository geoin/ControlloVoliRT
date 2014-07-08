/*
    File: tiff_util.cpp
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

#include "tiff_util.h"
#include <fstream>
#include "Poco/Path.h"
#include "dem_interpolate/geom.h"

using Poco::Path;
#define	EPS 1.e-4

TFW::TFW(const std::string& nm, const std::string& ext): _dimx(0), _dimy(0), _ext(ext) {
	read(nm);
	Path fn(nm);
	fn.setExtension("");
	_nome = fn.getBaseName();
}

DPOINT TFW::ter_img(const DPOINT& ter) const
{
	DPOINT t(ter);
	t.z = 1;
	DPOINT pix = _ter_pix * t;
	return pix;
}
DPOINT TFW::img_ter(const DPOINT& pix) const
{
	DPOINT p(pix);
	p.z = 1;
	DPOINT ter = _pix_ter * p;
	return ter;
}
bool TFW::read(const std::string& nome)
{
	std::ifstream ftfw(nome.c_str(), std::ifstream::in);
	if ( !ftfw.is_open() )
		return false;
	double v;
	ftfw >> v; _pix_ter[0][0] = v; // a
	ftfw >> v; _pix_ter[0][1] = v; // b
	ftfw >> v; _pix_ter[1][0] = v; // c
	ftfw >> v; _pix_ter[1][1] = v; // d
	ftfw >> v; _pix_ter[0][2] = v; // e
	ftfw >> v; _pix_ter[1][2] = v; // f
	_pix_ter[2][2]  = 1;
	_ter_pix = _pix_ter.Invert();
	
	Path fn(nome);
	fn.setExtension(_ext);
	CV_image ci;
	if ( ci.open(fn.toString()) ) {
		_dimx = ci.dimx();
		_dimy = ci.dimy();
	}

	return true;
}

double TFW::pix_size(void) const
{
	double dx = (_pix_ter[0][0] * _pix_ter[1][1] - _pix_ter[0][1] * _pix_ter[1][0]) / (_pix_ter[1][1] - _pix_ter[0][1]);
	double dy = (_pix_ter[0][1] * _pix_ter[1][0] - _pix_ter[0][0] * _pix_ter[1][1]) / (_pix_ter[1][0] - _pix_ter[0][0]);
	return sqrt(dx * dx + dy * dy);
}
long TFW::dimx(void) const
{
	return _dimx;
}
long TFW::dimy(void) const
{
	return _dimy;
}
const std::string& TFW::nome() const
{
	return _nome;
}
CV_image::~CV_image() 
{
	close();
}
bool CV_image::open(const std::string& nome)
{
	//img->_tfWrite = false;
	_tif = TIFFOpen(nome.c_str(), "rm");
	if ( _tif == NULL ) {
		return false;
	}
	_actStrip = -1;

	TIFFGetField(_tif, TIFFTAG_IMAGEWIDTH, &_dimx);
	TIFFGetField(_tif, TIFFTAG_IMAGELENGTH, &_dimy);

	short v;
	TIFFGetField(_tif, TIFFTAG_BITSPERSAMPLE, &v); _bit_sample = v;
	TIFFGetField(_tif, TIFFTAG_SAMPLESPERPIXEL, &v); _nbands = v;
	TIFFGetField(_tif, TIFFTAG_ROWSPERSTRIP, &RowStrip);

	TIFFGetField(_tif, TIFFTAG_PHOTOMETRIC, &PhotoInt);
	if ( _nbands == 3 && PhotoInt == 1 )
		PhotoInt = PHOTOMETRIC_RGB;

	if ( PhotoInt == PHOTOMETRIC_MINISWHITE || PhotoInt == PHOTOMETRIC_MINISBLACK ) {
		_type = _bit_sample == 1 ? ty_bilevel : ty_gray;
		bf = new unsigned char[_dimx];
	} else if ( PhotoInt == PHOTOMETRIC_RGB ) {
		_type = ty_rgb;
		col = new rgb[_dimx];
		bf = new unsigned char[_dimx];
	} else
		_type = ty_notsupported;

	int sz = TIFFStripSize(_tif);
	_buffs = (unsigned char*) _TIFFmalloc(sz);
	return true;
}
void CV_image::close()
{
	if ( _tif != NULL )
		TIFFClose(_tif);
	_tif = NULL;
	if ( col != NULL )
		delete [] col;
	if ( bf != NULL )
		delete [] bf;
	if ( _buffs != NULL )
		_TIFFfree(_buffs);
	col = NULL;
	bf = NULL;
	_buffs = NULL;
}
//bool CV_image::read_line(int line)
//{
//	if ( line < 0 || line >= _dimy )
//		return false;
//
//	uint32 strip = TIFFComputeStrip(_tif, line, 0);
//	if ( _type == ty_rgb ) {
//		TIFFReadEncodedStrip(_tif, strip, col, (tsize_t) -1);
//	} else  if ( _type == ty_gray )
//		TIFFReadEncodedStrip(_tif, strip, bf, (tsize_t) -1);
//	else
//		return false;
//
//	return true;
//}
bool CV_image::read_line(int line)
{
	long rws = 0;
	// data reading
	tstrip_t	strip;
	//if ( tilex > 0 ) {
	//	// modalità tile
	//	strip = line / img->tiley;
	//	if ( strip != _actStrip ) {
	//		if ( img->Planar == 2 ) {
	//			_readPlanarTile(img, line);
	//		} else {
	//			if ( img->PhotoInt == 6 ) {
	//				_readJpgTile(img, line);
	//			} else {
	//				_readTile(img, line);
	//			}
	//		}
	//		img->ActStrip = strip;
	//	}
	//	rws = img->tiley;
	//} else {
		// modalità strip
		strip = TIFFComputeStrip(_tif, line, 0);
		if ( strip != _actStrip ) {
			//if ( img->Planar == 2 ) {
			//	_readPlanarStrip(img, line);
			//} else {
			//	if ( img->PhotoInt == 6 ) {
			//		_readJpgStrip(img, line, strip);
			//	} else {
					TIFFReadEncodedStrip((TIFF*) _tif, strip, _buffs, (tsize_t) -1);
				//}
			//}
			_actStrip = strip;
		}
		rws = RowStrip;
	//}
	
	// DATA FORMATTING

	// monochrome images
	if ( PhotoInt <= 1 ) {
		if ( _bit_sample == 8 ) { // 8 bit
			uint32	off = (line - strip * rws) * _dimx;
			memcpy(bf, &_buffs[off], _dimx);
			return true;
		}
		//if ( _bit_sample == 16 ) { // 16 bit
		//	uint32	off = 2 * ((line - strip * rws) * _dimx);
		//	unsigned short* ubf = (unsigned short*) &_buffs[off];
		//	img->bbufs = ubf;
		//	for (int i = 0; i < n; i++) {
		//		short v = (short) (ubf[i] >> img->bitScale);
		//		if ( v < 0 ) v = 0; if ( v > 255 ) v = 255;
		//		img->bf[i] = (BYTE) v;
		//		//img->bf[i] = (BYTE) ((ubf[i] >> img->bitScale)  & 0xFF);
		//	}
		//	return 0;
		//}
		//if ( _bit_sample == 1 ) { // bilevel
		//	PIXEL wh = ( img->PhotoInt == 0 ) ? 0 : 255;
		//	uint32	off = (line - strip * img->RowStrip) * ((img->dimx + 7) / 8);
		//	byteExpand(&img->buffs[off], img->bf, img->dimx, wh);
		//	if ( x > 0 ) {
		//		for (int i = 0, j = x; i < n; i++, j++)
		//			img->bf[i] = img->bf[j];
		//	}
		//	return true;
		//}
	}

	// palette images
	//if ( img->PhotoInt == 3 ) {
	//	uint32	off = (line - strip * rws) * img->dimx + x;
	//	memcpy(img->bf, &img->buffs[off], n);
	//	for (int i = 0; i < n; i ++) {
	//		PIXEL lev = img->bf[i];
	//		img->rgb[i].r = (img->lut->r[lev] >> 8) & 0XFF;
	//		img->rgb[i].g = (img->lut->g[lev] >> 8) & 0XFF;
	//		img->rgb[i].b = (img->lut->b[lev] >> 8) & 0XFF;
	//	}
	//	return true;
	//}

	// color images
	if ( PhotoInt == 2 || PhotoInt == 6 ) {
		if ( _bit_sample == 16 ) {
			uint32	off = 6 * ((line - strip * rws) * _dimx);
			unsigned short* ubf = (unsigned short*) &_buffs[off];
			for (int i = 0; i < _dimx; i ++) {
				int k = 3 * i;

				short v = (short) (ubf[k + 0] >> bitScale);
				if ( v < 0 ) v = 0; if ( v > 255 ) v = 255;
				col[i].r = (unsigned char) v;

				v = (short) (ubf[k + 1] >> bitScale);
				if ( v < 0 ) v = 0; if ( v > 255 ) v = 255;
				col[i].g = (unsigned char) v;
				
				v = (short) (ubf[k + 2] >> bitScale);
				if ( v < 0 ) v = 0; if ( v > 255 ) v = 255;
				col[i].b = (unsigned char) v;
				col[i] = (unsigned char) (col[i].r * 0.30 + col[i].g * 0.59 + col[i].b * 0.11);
			}
			return true;
		}
		if (  _bit_sample == 8 ) {
			uint32	off = _nbands * ((line - strip * rws) * _dimx);
			unsigned char* b2 = &_buffs[off];
			if ( _nbands == 3 ) {
				memcpy(col, b2, 3 * _dimx);
			} else {
				for (int i = 0; i < _dimx; i ++) {
					memcpy((unsigned char*) &col[i], b2, 3);

					//memcpy((unsigned char*) &img->rgba[i], b2, 4);
					b2 += _nbands;
				}
			}
			for (int i = 0; i < _dimx; i ++)
				bf[i] = (unsigned char) (col[i].r * 0.30 + col[i].g * 0.59 + col[i].b * 0.11);

			return true;
		}
	}
	return true;
}
/***********************************************************************/
int BorderLine::_getBackGround()
{
	long nblack = 0;
	long nwhite = 0;
	for (int j = 0; j < _img.dimy(); j++){
		if ( !_img.read_line(j) )
			return -1;
		int inc = ( j == 0 || j == _img.dimy() - 1 ) ? 1 : _img.dimx() - 1;
		for (int i = 0; i < _img.dimx(); i += inc) {
			if ( _img.is_black(i) )
				nblack++;
			else if ( _img.is_white(i) )
				nwhite++;
		}
	}
	// incomplete table if at least 1/10 of the pixels are background
	//unsigned long thr = (unsigned long) (_img.dimx() * _img.dimy() * 0.1);
	//if ( nblack < thr || nwhite < thr )
	//	return -1;
	if ( nblack > nwhite )
		return 0;
	else if ( nblack < nwhite )
		return 255;
	return -1; // no background
}

/*----------------------------------------------------------------------------
!@  NextPtInChain
!@	return the next pixel in the chain
!
!----------------------------------------------------------------------------*/
bool BorderLine::_nextPtInChain(std::vector<Border>& chain, Border* prev, unsigned int *index)
{
	double dmin = INF;
	int idx = -1;
	int dx0 = 1000, dy0 = 1000;
	for (unsigned int i = 1; i < chain.size(); i++) {
		if ( chain[i].used )
			continue;
		int dx = abs(chain[i].x - prev->x);
		int dy = abs(chain[i].y - prev->y);
		if ( dx <= 1 && dy <= 1 ) {
			*index = i;
			return true;
		}
		double d = sqrt((double) (dx * dx) + (double) (dy * dy));
		if ( d < dmin ) {
			dmin = d;
			dx0 = dx;
			dy0 = dy;
			idx = i;
		}
	}
	if ( dmin < 15 ) {
		*index = idx;
		return true;
	}
	return false;
}

/*----------------------------------------------------------------------------
!@  distLine
!@	special two points line
!
!----------------------------------------------------------------------------*/
void BorderLine::_distLine(const Border& p1, const Border& p2, RETTA *retta)
{
	retta->X2Points(DPOINT(p1.x, p1.y), DPOINT(p2.x, p2.y));

	double den = sqrt(retta->a * retta->a + retta->b * retta->b);
	if ( den > EPS ) {
		retta->a /= den;
		retta->b /= den;
		retta->c /= den;
	}
}

/*----------------------------------------------------------------------------
!@  IsNewSide
!@	true if the new point is on the line
!
!----------------------------------------------------------------------------*/
bool BorderLine::_isInSide(const std::vector<Border>& bd, std::vector<unsigned int>& rt, unsigned int lt)
{
	RETTA	retta;
	_distLine(bd[rt[0]], bd[lt], &retta);

	for (unsigned int l = 1; l < rt.size(); l++) {
		double dist = fabs(retta.a * bd[rt[l]].x + retta.b * bd[rt[l]].y + retta.c);
		if ( dist >= 2. )
			return false;
	}
	return true;
}

/*----------------------------------------------------------------------------
!@  SetPixInfo
!@	Mark the pixels on the border
!
!----------------------------------------------------------------------------*/
void BorderLine::_setPixInfo(int line)
{
	_img.read_line(line);

	enum P_STATO {
		unkn = 0,
		back = 1,
		fore = 2
	};
	int i0, i9;
	P_STATO ps = unkn;
	for (int i = 0; i < _img.dimx(); i++) {
		bool is_back;
		if ( _img.is_type(CV_image::ty_rgb) )
			is_back = _img.col[i] == rgb(_background);
		else
			is_back = _img.bf[i] == _background;

		if ( ps == unkn ) {
			if ( is_back ) {
				ps = back;
				_img.bf[i] = 0;
			} else {
				ps = fore;
				_img.bf[i] = 1;
			}
			i0 = i9 = i;
		} else if ( ps == back ) {
			i9++;
			if ( !is_back ) {
				ps = fore;
				for ( int j = i0; j < i9; j++ )
					_img.bf[j] = 0;
				i0 = i9 = i;
			}
		} else {
			i9++;
			if ( is_back ) {
				ps = back;
				for ( int j = i0; j < i9; j++ )
					_img.bf[j] = 1;
				i0 = i9 = i;
			}
		}
	}
	unsigned char val = ps == back ? 0 : 1;
	for ( int j = i0; j < _img.dimx(); j++)
		_img.bf[j] = val;
}

bool BorderLine::_buildChain(std::vector<Border>& bd)
{
	unsigned char* _savebf = _img.bf;

	unsigned char* bfp = new unsigned char[_img.dimx()];
	unsigned char* bf = new unsigned char[_img.dimx()];
	
	_img.bf = bf;

	for (int i = 0; i < _img.dimy(); i++) {
		_setPixInfo(i);
		for (int j = 0; j < _img.dimx(); j++) {
			int i0 = i;
			bool _isborder = false;
			if ( (i == 0 || i == _img.dimy() - 1) && bf[j] == 1 ) {
				_isborder = true;
			} 
			if ( !_isborder ) {
				if ( j == 0 || j == _img.dimx() - 1 ) {
					if ( bf[j] == 1 )
						_isborder = true;
				} else  {
					if ( (bf[j - 1] == 0 || bf[j + 1] == 0 ) && bf[j] == 1 )
						_isborder = true;
				}
			}

			if ( _isborder == false && i > 0 ) {
				if ( bf[j] == 1 && bfp[j] == 0 )
					_isborder = true;
				else {
					if ( bf[j] == 0 && bfp[j] == 1 ) {
						_isborder = true;
						for (unsigned int k = 0; k < bd.size() && _isborder; k++) {
							if ( bd[k].x == j && bd[k].y == i - 1 )
								_isborder = false;
						}
						if ( _isborder )
							i0 = i - 1;
					}
				}
			}

			if ( _isborder ) {
				bd.push_back(Border(j, i0));
			}
		}
		_img.bf = bfp;
		bfp = bf;
		bf = _img.bf;
	}
	_img.bf = _savebf;
	delete [] bfp;
	delete [] bf;
	return !bd.empty();
}

/*----------------------------------------------------------------------------
!@  GetBorderLine
!@	build the chain with the border pixels
!
!----------------------------------------------------------------------------*/
bool BorderLine::_getBorderLine(std::vector<DPOINT>& pt)
{
	std::vector<Border> bord;
	if ( !_buildChain(bord) )
		return false; // border chain empty
	
	std::vector<unsigned int> side;
	side.push_back(0);

	pt.push_back(DPOINT(bord[0].x, bord[0].y));

	Border* _start = &bord[0];
	_start->used = true;
	Border* _new;

	unsigned int index;
	while ( true ) {
		bool found = false;
		if ( _nextPtInChain(bord, _start, &index) ) {
			_new = &bord[index];
			_new->used = true;
			found = true;
		} else
			break;

		if ( _isInSide(bord, side, index) ) {
			_start = _new;
			side.push_back(index);
		} else {
			// Add a new line
			pt.push_back(DPOINT(_start->x, _start->y));
			_start = _new;
			int idx = side[side.size() - 1];
			side.clear();
			side.push_back(idx);
		}
	}
	return !pt.empty();
}
bool BorderLine::Evaluate(const std::string& nome, std::vector<DPOINT>& pt)
{
	if ( !_img.open(nome) )
		return false;

	bool ret = false;
	if ( (_background = _getBackGround()) >= 0 ) {
		ret = _getBorderLine(pt);
	}
	if ( !ret ) {
		for (int i = 0; i < 4; i++) {
			int x = ( i == 0 || i == 3 ) ? 0 : _img.dimx() - 1;
			int y = ( i == 0 || i == 1 ) ? 0 : _img.dimy() - 1;
			pt.push_back(DPOINT(x, y));
		}
	}
	return true;
}
