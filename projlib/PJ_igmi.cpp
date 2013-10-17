/*----------------------------------------------------------------------------
!   Filename:  DIRLIST.CPP
!
!   Contents:	fill a list a directory file list
!
!   History:
!			09/09/2008	created
!			17/06/2009	updated
!----------------------------------------------------------------------------*/
#include <Poco/Path.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "common_defs.h"

#define _USE_MATH_DEFINES
#include <math.h>
#define DEG_RAD(val) (M_PI / 180.) * (val)
#define RAD_DEG(val) (180. / M_PI) * (val)


static int nearest(double val)
{
	return ( val > 0.) ? (int) (0.5 + val) : (int) (val - 0.5);
}
class GridItem {
public:
	double	val;
	bool	used;
};
class iGrid {
public:
	enum HGRID_TYPE {
		ty_UK = -1,
		ty_GR = 0,
		ty_GK = 1
	};
	enum DATUM_TYPE {
		ty_ROMA40 = 0,
		ty_ED50 = 1
	};
	int Nlat(void) const { return nlat; }
	int Nlon(void) const { return nlon; }
	double Lat0(void) const { 
		return lat0; 
	}
	double Lon0(void) const { 
		return lon0; 
	}
	double Lat1(void) const {
		return lat0 + plat * (nlat - 1);
	}
	double Lon1(void) const {
		return lon0 + plon * (nlon - 1);
	}
	double Plat(void) const { 
		return plat; 
	}
	double Plon(void) const { 
		return plon; 
	}

protected:
	double lat0;	// latitudine 1^ elemento matrice
	double lon0;		// longitudine 1^ elemento matrice
	double plat;	// passo matrice in latitudine
	double plon;	// passo matrice in longitudine
	int nlat;	// numero di righe della matrice (latitudine)
	int nlon;	// numero di colonne della matrice (longitudine)

	//virtual bool Init(const char* nome, int type = 0) = 0; // inizializza
	virtual void Release(void) = 0; // rilascia la memoria utilizzata
	HGRID_TYPE _type;	// tipo di grigliato grx o gkx
	int _idxLat0, _idxLon0;
};
class hGrid:public iGrid {	// griglia per correzioni planimetriche
public:
	hGrid(void): dlat(NULL), dlon(NULL) {
		plon = DEG_RAD(7.5 / 60.);	// da primi a radianti
		plat = DEG_RAD(5. / 60.);	// da primi a radianti
	}
	~hGrid(void) {
		Release();
	}
	bool Init(const char* nome, HGRID_TYPE type = ty_GR, DATUM_TYPE datumtype = ty_ROMA40); // inizializza
	bool MergeGrid(const char* nome, DATUM_TYPE dt = ty_ROMA40);
	void Release(void) {
		if ( dlat != NULL )
			delete [] dlat;
		if ( dlon != NULL )
			delete [] dlon;
		dlat = dlon = NULL;
	}
	bool IsUsed(int index) { return dlat[index].used; }
	bool GetCorrections(const double lat, const double lon, double* dlat, double* dlon); // calcola le correzioni
	bool GetCoord(const double latIn, const double lonIn, double* latOut, double* lonOut); // calcola le coordinate corrette
	bool GetCoordI(const double latIn, const double lonIn, double* latOut, double* lonOut); // calcola le coordinate corrette usando la trasformazione inversa
private:
	GridItem* dlat;	// matrice di correzione latitudine ordinata per righe
	GridItem* dlon;	// matrice di correzione longitudine ordinata per righe
	int _idxLatS, _idxLatE;
	int _idxLonS, _idxLonE;
};
class vGrid:public iGrid {	// griglia per correzioni altimetriche
public:
	vGrid(void): dq(NULL) {
		plon = DEG_RAD(2. / 60.);
		plat = DEG_RAD(2. / 60.);
	}
	~vGrid(void) {
		Release();
	}
	bool Init(const char* nome, int type = 0); // inizializza
	void Release(void) {
		if ( dq != NULL )
			delete [] dq;
		dq = NULL;
	}
	bool GetCorrections(const double lat, const double lon, double* cn);
	bool GetQuota(const double latIn, const double lonIn, double* quota);
	bool MergeGrid(const char* nome);
private:
	GridItem* dq;	// matrice di correzione ondulazione ordinata per righe
	int _idxQS, _idxQE;
};
// apre il file ed acquisisce i parametri di correzione tra roma40 e wgs84
bool hGrid::Init(const char* nome, HGRID_TYPE type, DATUM_TYPE datumtype)
{
	if ( type == ty_GR ) {
		_type = ty_GR;
		_idxLat0 = 148; // posizione dove c'è la lat del grid 0, 0
		_idxLon0 = 149; // posizione dove c'è la lon del grid 0, 0
		if ( datumtype == ty_ROMA40 ) { // caso roma40
			_idxLatS = 76;  // posizione di partenza delle corr. di latitudine
			_idxLatE = 111; // posizione di fine delle corr. di latitudine
			_idxLonS = 112; // posizione di partenza delle corr. di longitudine
			_idxLonE = 147; // posizione di fine delle corr. di latitudine
		} else { // caso ed50
			_idxLatS = 3;  // posizione di partenza delle corr. di latitudine
			_idxLatE = 38; // posizione di fine delle corr. di latitudine
			_idxLonS = 39; // posizione di partenza delle corr. di longitudine
			_idxLonE = 74; // posizione di fine delle corr. di latitudine
		}
	} else {
		_type = ty_GK;
		_idxLat0 = 151;
		_idxLon0 = 152;
		if ( datumtype == ty_ROMA40 ) { // caso roma40
			_idxLatS = 79;
			_idxLatE = 114;
			_idxLonS = 115;
			_idxLonE = 150;
		} else {
			_idxLatS = 5;
			_idxLatE = 40;
			_idxLonS = 41;
			_idxLonE = 76;
		}
	}

    std::fstream tf;
    tf.open(nome, std::fstream::in);
    if ( !tf.is_open() )
		return false;
    char mes[256];
	Release(); 

	nlat = 6;
	nlon = 6;
	dlat = new GridItem[nlat * nlon];
	dlon = new GridItem[nlat * nlon];
	for (int i = 0; i < nlat * nlon; i++) {
		dlat[i].used = false;
		dlon[i].used = false;
	}

	int k = 1;
	int i = 0, j = 0;
    while ( tf.getline(mes, 255) != NULL ) {
		if ( k == _idxLat0 ) lat0 = DEG_RAD(atof(mes)); //riga 148 contiene la latitudine dell'origine roma40 wgs84
		if ( k == _idxLon0 ) lon0 = DEG_RAD(atof(mes)); //riga 149 contiene la longitudine dell'origine
		if ( k == _idxLatS || k == _idxLonS ) { // 76 inizio grigliato latitudine, 112 inizio grigliato longitudine
			i = j = 0;
		}
		// correzioni della latitudine
		if ( k >= _idxLatS && k <= _idxLatE ) { // da 76 a 111 ci sono i dati della latitudine
			int index = i * nlon + j++;
			dlat[index].val = atof(mes);
			dlat[index].used = true;
			if ( j >= nlon ) {
				j = 0;
				i++;
			}
		}
		// correzioni della longitudine
		if ( k >= _idxLonS && k <= _idxLonE ) { // da 112 a 147 ci sono i dati della longitudine
			int index = i * nlon + j++;
			dlon[index].val = atof(mes);
			dlon[index].used = true;
			if ( j >= nlon/*6*/ ) {
				j = 0;
				i++;
			}
		}
		k++;
	}
	return true;
}
bool hGrid::MergeGrid(const char* nome, DATUM_TYPE dt)
{
	hGrid hg;
	if ( !hg.Init(nome, _type, dt) )
		return false;
	if ( plat != hg.plat || plon != hg.plon )
		return false; // non si possono fondere grigliati con passi differenti

    double minlat = std::min(lat0, hg.lat0);
    double minlon = std::min(lon0, hg.lon0);
	
	double lat1 = lat0 + plat * (nlat - 1);
	double lon1 = lon0 + plon * (nlon - 1);
    double maxlat = std::max(lat1, hg.lat0 + hg.plat * (hg.nlat - 1));
    double maxlon = std::max(lon1, hg.lon0 + hg.plon * (hg.nlon - 1));

	int newlat = nearest((maxlat - minlat) / plat) + 1;
	int newlon = nearest((maxlon - minlon) / plon) + 1;
	GridItem* llat = new GridItem[newlat * newlon];
	GridItem* llon = new GridItem[newlat * newlon];
	for (int i = 0; i < newlat * newlon; i++) {
		llat[i].used = false;
		llon[i].used = false;
	}

	try {
		for (int i = 0; i < newlat; i++) {
			double xlat = minlat + i * plat;
			int i1 = nearest((xlat - lat0) / plat); // indice lat nel grigliato di partenza
			int i2 = nearest((xlat - hg.lat0) / hg.plat); // indice lat nel grigliato da unire
			for (int j = 0; j < newlon; j++) {
				int index = i * newlon + j;
				double xlon = minlon + j * plon; 
				int j1 = nearest((xlon - lon0) / plon); // indice lon nel grigliato di partenza
				int j2 = nearest((xlon - hg.lon0) / hg.plon); // indice lon nel grigliato da unire
				if ( i1 >= 0 && i1 < nlat && j1 >= 0 && j1 < nlon ) {
					// il pt attuale coinvolge il grigliato di partenza
					int idx = i1 * nlon + j1;
					llat[index].val = dlat[idx].val;
					llon[index].val = dlon[idx].val;
					llat[index].used = dlat[idx].used;
					llon[index].used = dlon[idx].used;
				}
				if ( i2 >= 0 && i2 < hg.nlat && j2 >= 0 && j2 < hg.nlon ) {
					// il punto attuale coinvolge il nuovo grigliato
					int idx = i2 * hg.nlon + j2;
					if ( llat[index].used && hg.dlat[idx].used ) {
						if ( llat[index].val != hg.dlat[idx].val )
                            throw(std::runtime_error("Anomalia"));
					} else {
						llat[index].val = hg.dlat[idx].val;
						llat[index].used = hg.dlat[idx].used;
					}
					if ( llon[index].used && hg.dlon[idx].used ) {
						if ( llon[index].val != hg.dlon[idx].val )
                            throw(std::runtime_error("Anomalia"));
					} else {
						llon[index].val = hg.dlon[idx].val;
						llon[index].used = hg.dlon[idx].used;
					}
				}
			}
		}
	} catch(std::exception e) {
		delete [] llat;
		delete [] llon;
		return false;
	}
	delete [] dlat;
	dlat = llat;
	nlat = newlat;
	lat0 = minlat;

	delete [] dlon;
	dlon = llon;
	nlon = newlon;
	lon0 = minlon;
	return true;
}

bool hGrid::GetCorrections(const double lat, const double lon, double* clat, double* clon)
{
	double dt = lat - lat0; // offset della latitudine dall'inizio della griglia
	double dn = lon - lon0; // offset della longitudine dall'inizio della griglia
	if ( dt < 0. || dn < 0 )
		return false; // fuori griglia
	int nt = (int) (dt / plat); // indice riga latitudine
	int nl = (int) (dn / plon); // indice colonna longitudine
	if ( nt >= nlat - 1 || nl >= nlon - 1 )
		return false; // fuori griglia
	
	double ddt = lat - (lat0 + nt * plat); // distanza in latitudine rispetto al crocicchio precedente
	double ddn = lon - (lon0 + nl * plon); // distanza in longitudine rispetto al crocicchio precedente

	int index1 = nt * nlon + nl;		// calcolo indice nella matrice
	int index2 = (nt + 1) * nlon + nl;	// calcolo indice della riga successiva nella matrice
	if ( !dlat[index1].used || !dlat[index1 + 1].used || !dlat[index2].used || !dlat[index2 + 1].used )
		return false;

	double dlat1 = dlat[index1].val + ddn * (dlat[index1 + 1].val - dlat[index1].val) / plon;
	double dlat2 = dlat[index2].val + ddn * (dlat[index2 + 1].val - dlat[index2].val) / plon;
	*clat = dlat1 + ddt * (dlat2 - dlat1) / plat;

	double dlon1 = dlon[index1].val + ddn * (dlon[index1 + 1].val - dlon[index1].val) / plon;
	double dlon2 = dlon[index2].val + ddn * (dlon[index2 + 1].val - dlon[index2].val) / plon;
	*clon = dlon1 + ddt * (dlon2 - dlon1) / plat;
	return true;
}
bool hGrid::GetCoord(const double latIn, const double lonIn, double* latOut, double* lonOut)
{
	double clat, clon;
	*latOut = latIn;
	*lonOut = lonIn;
	if ( !GetCorrections(latIn, lonIn, &clat, &clon) )
		return false;

	*latOut = latIn + DEG_RAD(clat / 3600.);
	*lonOut = lonIn + DEG_RAD(clon / 3600.);
	return true;
}
bool hGrid::GetCoordI(const double latIn, const double lonIn, double* latOut, double* lonOut)
{
	double clat, clon;
	*latOut = latIn;
	*lonOut = lonIn;
	if ( !GetCorrections(latIn, lonIn, &clat, &clon) )
		return false;

	double lattmp = latIn - DEG_RAD(clat / 3600.);
	double lontmp = lonIn - DEG_RAD(clon / 3600.);
	if ( !GetCorrections(lattmp, lontmp, &clat, &clon) )
		return false;

	*latOut = latIn - DEG_RAD(clat / 3600.);
	*lonOut = lonIn - DEG_RAD(clon / 3600.);
	return true;
}

bool vGrid::Init(const char* nome, int type)
{
	if ( type == ty_GR ) {
		_type = ty_GR;
		_idxLat0 = 291;
		_idxLon0 = 292;
		_idxQS = 151;
		_idxQE = 290;
	} else {
		_type = ty_GK;
		_idxLat0 = 407;
		_idxLon0 = 408;
		_idxQS = 267;
		_idxQE = 406;
	}

    std::fstream tf;
    tf.open(nome, std::fstream::in);
    if ( !tf.is_open() )
		return false;
    char mes[256];
	Release(); 

	nlat = 10;
	nlon = 14;
	dq = new GridItem[nlat * nlon];
	for (int i = 0; i < nlat * nlon; i++) {
		dq[i].used = false;
	}

	int k = 1;
	int i = 0, j = 0;
    while ( tf.getline(mes, 255) != NULL ) {
		if ( k == _idxLat0 ) lat0 = DEG_RAD(atof(mes));
		if ( k == _idxLon0 ) lon0 = DEG_RAD(atof(mes));
		if ( k == _idxQS ) {
			i = nlat - 1;
			j = 0;
		}
		// correzioni delle ondulazioni
		if ( k >= _idxQS && k <= _idxQE ) {
			int index = i * nlon + j++;
			dq[index].val = atof(mes);
			dq[index].used = true;
			if ( j >= nlon ) {
				j = 0;
				i--;
			}
		}
		k++;
	}
	return true;
}
bool vGrid::MergeGrid(const char* nome)
{
	vGrid vg;
	if ( !vg.Init(nome, _type) )
		return false;
	if ( plat != vg.plat || plon != vg.plon )
		return false; // non si possono fondere grigliati con passi differenti

    double minlat = std::min(lat0, vg.lat0);
    double minlon = std::min(lon0, vg.lon0);
	
	double lat1 = lat0 + plat * (nlat - 1);
	double lon1 = lon0 + plon * (nlon - 1);
    double maxlat = std::max(lat1, vg.lat0 + vg.plat * (vg.nlat - 1));
    double maxlon = std::max(lon1, vg.lon0 + vg.plon * (vg.nlon - 1));

	int newlat = nearest((maxlat - minlat ) / plat) + 1;
	int newlon = nearest((maxlon - minlon ) / plon) + 1;
	GridItem* lq = new GridItem[newlat * newlon];
	for (int i = 0; i < newlat * newlon; i++) {
		lq[i].used = false;
	}

	try {
		for (int i = 0; i < newlat; i++) {
			double xlat = minlat + i * plat;
			int i1 = nearest((xlat - lat0) / plat);
			int i2 = nearest((xlat - vg.lat0) / vg.plat);
			for (int j = 0; j < newlon; j++) {
				int index = i * newlon + j;
				double xlon = minlon + j * plon;
				int j1 = nearest((xlon - lon0) / plon);
				int j2 = nearest((xlon - vg.lon0) / vg.plon);
				if ( i1 >= 0 && i1 < nlat && j1 >= 0 && j1 < nlon ) {
					int idx = i1 * nlon + j1;
					lq[index].val = dq[idx].val;
					lq[index].used = dq[idx].used;
				}
				if ( i2 >= 0 && i2 < vg.nlat && j2 >= 0 && j2 < vg.nlon ) {
					int idx = i2 * vg.nlon + j2;
					if ( lq[index].used ) {
						if ( lq[index].val != vg.dq[idx].val )
                            throw(std::runtime_error("Anomalia"));

					} else {
						lq[index].val = vg.dq[idx].val;
						lq[index].used = vg.dq[idx].used;
					}
				}
			}
		}
	} catch(std::exception e) {
		delete [] lq;
		return false;
	}
	delete [] dq;
	dq = lq;
	nlat = newlat;
	nlon = newlon;
	lat0 = minlat;
	lon0 = minlon;
	return true;
}
bool vGrid::GetCorrections(const double lat, const double lon, double* cn)
{
	double dt = lat - lat0; // offset della latitudine dall'inizio della griglia
	double dn = lon - lon0; // offset della longitudine dall'inizio della griglia
	//if ( dt < 0. || dn < 0 )
	//	return false; // fuori griglia
	int nt = (int) (dt / plat); // indice riga latitudine
	int nl = (int) (dn / plon); // indice colonna longitudine
	//if ( nt >= nlat - 1 || nl >= nlon - 1 )
	//	return false; // fuori griglia

	double ddt = lat - (lat0 + nt * plat); // distanza in latitudine rispetto al crocicchio precedente
	double ddn = lon - (lon0 + nl * plon); // distanza in longitudine rispetto al crocicchio precedente

	int nt1 = nt + 1;
	if ( nt < 0 ) {
		nt = nt1 = 0;
		ddt = 0.;
	}
	if ( nt >= nlat - 1 ) {
		nt = nt1 = nlat - 1;
		ddt = 0.;
	}

	int nl1 = nl + 1;
	if ( nl < 0 ) {
		nl = nl1 = 0;
		ddn = 0.;
	}
	if ( nl >= nlon - 1 ) {
		nl = nl1 = nlon - 1;
		ddn = 0.;
	}

	int index1 = nt * nlon + nl;	// calcolo indice nella matrice
	int index2 = nt1 * nlon + nl;	// calcolo indice della riga successiva nella matrice
	int index3 = nt * nlon + nl1;	// calcolo indice nella matrice
	int index4 = nt1 * nlon + nl1;	// calcolo indice della riga successiva nella matrice

	if ( !dq[index1].used || !dq[index2].used || !dq[index3].used || !dq[index4].used )
		return false;

	double dn1 = dq[index1].val + ddn * (dq[index3].val - dq[index1].val) / plon;
	double dn2 = dq[index2].val + ddn * (dq[index4].val - dq[index2].val) / plon;
	*cn = dn1 + ddt * (dn2 - dn1) / plat;
	return true;
}
bool vGrid::GetQuota(const double latIn, const double lonIn, double* quota)
{
	double cn;
	if ( !GetCorrections(latIn, lonIn, &cn) )
		return false;
	*quota = *quota + cn;
	return true;
}

class IgmGrid {
public:

	IgmGrid(void): hg(NULL), vg(NULL), hge(NULL), Quote(true), Plan(true) {}
	~IgmGrid(void) {
		Release();
	}
	void Release(void) {
		if ( hg != NULL )
			delete hg;
		if ( hge != NULL )
			delete hge;
		if ( vg != NULL )
			delete vg;
		hg = NULL;
		hge = NULL;
		vg = NULL;
	}
	bool Init(const char* nome, iGrid::HGRID_TYPE type) {
		Release(); 
		hg = new hGrid;
		vg = new vGrid;
		bool ret1 = hg->Init(nome, type, iGrid::ty_ROMA40);
		bool ret2 = vg->Init(nome, type);
		bool ret3 = true;
		if ( _datumtype == iGrid::ty_ED50) {
			hge = new hGrid;
			ret3 = hge->Init(nome, type, iGrid::ty_ED50);
		}
		return ret1 && ret2 && ret3;
	}
	bool Merge(const char* nome) {
		bool ret1 = hg->MergeGrid(nome, iGrid::ty_ROMA40);
		bool ret3 = true;
		if ( _datumtype == iGrid::ty_ED50 )
			ret3 = hge->MergeGrid(nome, iGrid::ty_ED50);
		bool ret2 = vg->MergeGrid(nome);
		return ret1 && ret2 && ret3;
	}
	// da roma40 a wgs84
	bool GetCoord(const double latIn, const double lonIn, double* latOut, double* lonOut) {
		if ( _datumtype == iGrid::ty_ED50 && hge != NULL ) {
			bool ret2 = hge->GetCoordI(latIn, lonIn, latOut, lonOut);
			if ( ret2 ) return hg->GetCoord(*latOut, *lonOut, latOut, lonOut);
		} else
			return hg->GetCoord(latIn, lonIn, latOut, lonOut);
		return false;
	}
	// da wgs84 a roma40
	bool GetCoordI(const double latIn, const double lonIn, double* latOut, double* lonOut) { // calcola le coordinate corrette usando la trasformazione inversa
		bool ret1 = hg->GetCoordI(latIn, lonIn, latOut, lonOut);
		bool ret2 = true;
		if ( _datumtype == iGrid::ty_ED50 && hge != NULL )
			ret2 = hge->GetCoord(*latOut, *lonOut, latOut, lonOut);
		return ret1 && ret2;
	}
	// da ortometrica a ellissoidica
	bool GetQuota(const double latIn, const double lonIn, double* quota) {
		return vg->GetQuota(latIn, lonIn, quota);
	}
	// da ellissoidica aa ortometrica
	bool GetQuotaI(const double latIn, const double lonIn, double* quota) {
		double latOut, lonOut;
		if ( !hg->GetCoordI(latIn, lonIn, &latOut, &lonOut) )
			return false;
		double q;
		if ( !vg->GetCorrections(latIn, lonIn, &q) )
			return false;
		*quota  = *quota - q;
		return true;
	}
	void SetDatum(iGrid::DATUM_TYPE dt) {
		_datumtype = dt;
	}
	iGrid::DATUM_TYPE GetDatum(void) { return _datumtype; }
	bool Quote;
	bool Plan;
    std::fstream lg;
	hGrid* hg;
	hGrid* hge;
	vGrid* vg;
	iGrid::DATUM_TYPE _datumtype;
};

extern "C" {
void* grInit(const char* igmgrids)
{
	IgmGrid* ig = new IgmGrid;
	ig->Plan = true; // modalità che trasforma solo il datum planimetrico
	ig->Quote = true; // modalità che trasforma solo il datum altimetrico
	char* igm = new char[strlen(igmgrids) + 1];
	strcpy(igm, igmgrids);
	char* chv = strrchr(igm, ',');
	if ( chv != NULL ) {
		if ( !strcmp(chv + 1, "1" ) ) {
			ig->Plan = true;
			ig->Quote = false;
		} else if ( !strcmp(chv + 1, "2" ) ) {
			ig->Plan = false;
			ig->Quote = true;
		} else if ( !stricmp(chv + 1, "ed" ) ) {
			ig->SetDatum(iGrid::ty_ED50);
		}
		*chv = '\0';
		chv = strrchr(igm, ',');
		if ( chv != NULL ) {
			if ( !strcmp(chv + 1, "1" ) ) {
				ig->Plan = true;
				ig->Quote = false;
			} else if ( !strcmp(chv + 1, "2" ) ) {
				ig->Plan = false;
				ig->Quote = true;
			} else if ( !stricmp(chv + 1, "ed" ) ) {
				ig->SetDatum(iGrid::ty_ED50);
			}
			*chv = '\0';
		}
	}
    Poco::Path fn(igm);
    fn.setExtension("log");
    ig->lg.open(fn.toString().c_str(), std::fstream::out | std::fstream::trunc);
	if ( ig->GetDatum() == iGrid::ty_ED50 ) {
        ig->lg << "Datum ED50\n";
	} else {
        ig->lg << "Datum ROMA40\n";
	}

	const char* ch = strrchr(igm, '.');
	if ( ch == NULL ) return NULL;
	if ( !stricmp(ch + 1, "gr1") || !stricmp(ch + 1, "gr2")) {
		ig->lg << " grigliato in formato gr1 - gr2\n";
		// grigliato di tipo gr1 o gr2
		ig->Init(igm, iGrid::ty_GR);
	} else if ( !stricmp(ch + 1, "gk1") || !stricmp(ch + 1, "gk2")) {
		ig->lg << " grigliato in formato gk1 - gk2\n";
		// grigliato di tipo gk1 o gk2
		ig->Init(igm, iGrid::ty_GK);
	} else {
		// grigliato di tipo non riconosciuto
        std::fstream tf;
        tf.open(igm, std::fstream::in);
        if ( !tf.is_open() ) {
			ig->lg << " impossibile aprire " << igm << "\n";
			delete [] igm;
			return NULL;
		}
        char mes[256];
		int k = 0;
		ig->lg << " grigliato multiplo\n";
        while ( tf.getline(mes, 255) != NULL ) {
			if ( strlen(mes) == 0 ) 
				continue;
			if ( k == 0 ) {
				iGrid::HGRID_TYPE type = iGrid::ty_UK;
				const char* ch = strrchr(mes, '.');
				if ( ch == NULL ) continue;
				if ( !stricmp(ch + 1, "gr1") || !stricmp(ch + 1, "gr2")) type = iGrid::ty_GR;
				else if ( !stricmp(ch + 1, "gk1") || !stricmp(ch + 1, "gk2")) type = iGrid::ty_GK;
				if ( type == iGrid::ty_UK ) {
					ig->lg << "\tformato di: " << mes << " non riconosciuto\n";
					continue;
				}
				if ( ig->Init(mes, type) ) {
					ig->lg << "\taggiunto: " << mes << "\n";
				} else {
					ig->lg << "\timpossibile aggiungere: " << mes << "\n";
				}
			} else {
				if ( ig->Merge(mes) ) {
					ig->lg << "\taggiunto: " << mes << "\n";
				} else {
					ig->lg << "\timpossibile aggiungere: " << mes << "\n";
				}
			}
			k++;
		}
		if ( k == 0 ) {
			ig->lg << " nessun grigliato trovato " << igm << "\n";
			delete [] igm;
			return NULL;
		}
	}
	delete [] igm;
	ig->lg << "Grigliato complessivo: " << ig->hg->Nlat() << "x" << ig->hg->Nlon() << "\n";
    std::stringstream ms;
    ms.precision(5);
	
	ig->lg << "S-O: ";
    ms.seekg(0, std::ios_base::beg);
    ms << RAD_DEG(ig->hg->Lat0());
    ig->lg << ms.str() << ", ";
    ms.seekg(0, std::ios_base::beg);
    ms << RAD_DEG(ig->hg->Lon0());
    ig->lg << ms.str() << "\n";
	
	ig->lg << "N-E: ";
    ms.seekg(0, std::ios_base::beg);
    ms << RAD_DEG(ig->hg->Lat1());
    ig->lg << ms.str() << ", ";
    ms.seekg(0, std::ios_base::beg);
    ms << RAD_DEG(ig->hg->Lon1());
    ig->lg << ms.str() << "\n\n";

	for (int i = 0, k = 1; i < ig->hg->Nlat(); i++) {
		double lat = ig->hg->Lat0() + i * ig->hg->Plat();
		for (int j = 0; j < ig->hg->Nlon(); j++) {
			long index = i * ig->hg->Nlon() + j;
			double lon = ig->hg->Lon0() + j* ig->hg->Plon();
			if ( ig->hg->IsUsed(index) ) {
                std::stringstream ms;
                ms.precision(5);

				ms << k++ << " ";
				ms << RAD_DEG(lat) << " " << RAD_DEG(lon);
                ig->lg << ms.str() << "\n";
			}
		}
	}

	return ig;
}
void grEnd(void*_ig_)
{
	IgmGrid* ig = (IgmGrid*) _ig_;
	if ( ig != NULL )
		delete ig;
}
void grApply(void*_ig_, double* x, double* y, double* z)
{
	IgmGrid* ig = (IgmGrid*) _ig_;
	if ( ig == NULL ) return;
	double xc, yc, qc = *z;
	if ( ig->Quote ) {
		ig->GetQuota(*y, *x, &qc);
		*z = qc;
	}
	if ( ig->Plan ) {
		if ( !ig->GetCoord(*y, *x, &yc, &xc) ) {
			;
			//ig->lg << " punto fuori zona: " << *x << ", " << *y << "\n";
		}
		*x = xc;
		*y = yc;
	}
}
void grApplyI(void*_ig_, double* x, double* y, double* z)
{
	// da wgs84
	IgmGrid* ig = (IgmGrid*) _ig_;
	if ( ig == NULL ) return;
	double xc, yc, qc = *z;
	if ( ig->Quote ) {
		ig->GetQuotaI(*y, *x, &qc);
		*z = qc;
	}
	if ( ig->Plan ) {
		if ( !ig->GetCoordI(*y, *x, &yc, &xc) ) {
			;
			//ig->lg << " punto fuori zona: " << *x << ", " << *y << "\n";
		}
		*x = xc;
		*y = yc;
	}
}
}
