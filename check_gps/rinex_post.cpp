/*==============================================================================
!   Filename:  RINEX_POST.CPP
!
!   Contents:	rinex data manager
!
!   History:
!			28/12/2011	created
!			28/12/2011	updated
!================================================================================*/
#include <stdarg.h>
#include "gps.h"
#include "rtklib/rtklib.h"
#include <Poco/Path.h>

#include <vector>

#define PRGNAME     "RTKPOST"

static gtime_t tstart_={0};         // time start for progress-bar
static gtime_t tend_  ={0};         // time end for progress-bar
vGPS* pr;
//Abort* myabort;

extern "C" {

// show message in message area ---------------------------------------------
int fshowmsg(char *format, ...)
{
	return 0;
	va_list arg;
	char buff[1024];
	if ( *format ) {
		va_start(arg, format);
		vsprintf(buff, format, arg);
		va_end(arg);
		//printf("%s\n", buff);
	}
	return 0;
}
// set time span of progress bar --------------------------------------------
void fsettspan(gtime_t ts, gtime_t te)
{
    tstart_ = ts;
    tend_  = te;
}
// set current time to show progress ----------------------------------------
int fsettime(gtime_t time)
{
    double tt;
	int perc = 0;
    if ( tend_.time != 0 && tstart_.time != 0 && (tt = timediff(tend_, tstart_)) > 0.0 ) {
        perc = (int)(timediff(time, tstart_) / tt * 100.0 + 0.5);
		//if ( myabort != NULL ) {
		//	if ( myabort->Set(perc) )
		//		return 1;
		//}
    }
	return 0;
}
void fresettime(char* mes)
{
	//if ( myabort != NULL ) {
	//	myabort->Clear();
	//	myabort->Start(100);
	//	Fname fn(mes);
	//	myabort->SetTitle(fn.GetNome());
	//}
}
void fsetread(double ps)
{
	//if ( myabort != NULL )
	//	myabort->Set(ps);
}
void fmyCoordPlot(double* rec, const char* date)
{
	GPS_DATA gd;
	gd.pos.set(rec[1], rec[0], rec[2]);
	if ( date == NULL )
		gd.data.assign("base");
	else
		gd.data.assign(date);
	pr->push_back(gd);
}
}
/********************************************************/
class rinex_post {
public:
	rinex_post(int sol_mode): _prcopt(prcopt_default), _solopt(solopt_default),
 		_timeStart_Checked(false), _timeEnd_Checked(false)	{
    	_filopt.satantp[0] = '\0';
    	_filopt.rcvantp[0] = '\0';
    	_filopt.stapos[0] = '\0'; 
    	_filopt.geoid[0] = '\0';  
    	_filopt.iono[0] = '\0';   
    	_filopt.dcb[0] = '\0';    
    	_filopt.tempdir[0] = '\0';
    	_filopt.geexe[0] = '\0';  
    	_filopt.solstat[0] = '\0';
    	_filopt.trace[0] = '\0';
		_prcopt.mode = sol_mode;
		read_options();
	}

	int read_options(void);
	int write_options(void);
	int ExecProc(void);
	void set_rover(const std::string& rover) {
		Poco::Path fn(rover);
		std::string ext = fn.getExtension();

		int l = (int) ext.size() - 1;
		ext[l] = 'o';
		fn.setExtension(ext);
		_inputFile1 = fn.toString();
		ext[l] = 'n'; 
		fn.setExtension(ext);
		_inputFile3 = fn.toString();
		if ( _prcopt.navsys & SYS_GLO ) {
			ext[l] = 'g'; 
			fn.setExtension(ext);
			_inputFile31 = fn.toString();
		}
	}
	void set_base(const std::string& base) {
		Poco::Path fn(base);
		std::string ext = fn.getExtension();

		int l = (int) ext.size() - 1;
		ext[l] = 'o';
		fn.setExtension(ext);
		_inputFile2 = fn.toString();
		ext[l] = 'n'; // default navigazione da base
		fn.setExtension(ext);
		_inputFile3 = fn.toString();
		if ( _prcopt.navsys & SYS_GLO ) {
			ext[l] = 'g';
			fn.setExtension(ext);
			_inputFile31 = fn.toString();
		}
	}
	void set_out(const std::string& output) {
		_outputFile = output;
	}
	void set_mode(int mode) {
		_prcopt.mode = mode;
	}
	void set_type(int type) {
		_prcopt.soltype = type;
	}
	void set_nav(int nav) {
		_prcopt.navsys = nav;
	}
	void set_freq(int fre) {
		_prcopt.nf = fre;
	}
	void set_max_base(double max_base_dst) {
		if ( max_base_dst > 0 )
	        _prcopt.baseline[1] = max_base_dst;
	}
	void set_ref_pos(DPOINT& pt) {
		double pos[3];
		pt.assign(&pos[1], &pos[0], &pos[2]);
		pos2ecef(pos, &_prcopt.rb[0]);
		_prcopt.refpos = 0;
	}
	void set_lim(MBR* mbr) {
		_prcopt.xlim[0] = _prcopt.xlim[1] = 0.;
		_prcopt.ylim[0] = _prcopt.ylim[1] = 0.;
		if ( mbr == NULL ) 
			return;
		_prcopt.xlim[0] = mbr->GetMinX();
		_prcopt.xlim[1] = mbr->GetMaxX();
		_prcopt.ylim[0] = mbr->GetMinY();
		_prcopt.ylim[1] = mbr->GetMaxY();

	}
private:
	int _obsToNav(const char *obsfile, char *navfile);
	prcopt_t _prcopt;
	solopt_t _solopt;
	filopt_t _filopt;
	ssat_t	_ssopt;
	std::string _inputFile1;
	std::string _inputFile2;
	std::string _inputFile3;
	std::string _inputFile31;
	std::string _inputFile4;
	std::string _inputFile5;
	std::string _outputFile;
 	bool _timeStart_Checked;
	bool _timeEnd_Checked;
};
int rinex_post::read_options()
{
    char buff[1024], *p;
    int sat,ex;

	//IniMgr im("C:\\windows\\rtkpostaaa.ini");
    // processing options
	//im.SetSection("opt");
	int PosMode = _prcopt.mode; //im.GetIntKey("posmode", 4);
    //_prcopt.mode     = PosMode;
    _prcopt.soltype  = 2;// im.GetIntKey("solution", 2); // 0 forward 1 backward 2 combined
    _prcopt.nf       = 2; //im.GetIntKey("freq", 1) + 1; // L1 + L2
    _prcopt.navsys   = SYS_GPS; //im.GetIntKey("navsys", SYS_GPS);
	_prcopt.elmin    = 15.0 * D2R; //im.GetFloatKey("elmask", 15.0) * D2R;
    _prcopt.snrmin   = 0.; //im.GetFloatKey("snrmask", 0.0);
    _prcopt.sateph   = 0; //im.GetIntKey("satephem",  0);
    _prcopt.modear   = 1; //im.GetIntKey("ambres", 1);
    _prcopt.glomodear= 1; //im.GetIntKey("gloambres", 1);
    _prcopt.maxout   = 5; //im.GetIntKey("outcntresetbias", 5);
    _prcopt.minfix   = 10; //im.GetIntKey("fixcntholdamb", 10);
    _prcopt.minlock  = 0; //im.GetIntKey("lockcntfixamb",  0);
    _prcopt.ionoopt  = IONOOPT_BRDC; //im.GetIntKey( "ionoopt", IONOOPT_BRDC);
    _prcopt.tropopt  = TROPOPT_SAAS; //im.GetIntKey("tropopt", TROPOPT_SAAS);
    _prcopt.dynamics = 0; //im.GetIntKey("dynamicmodel", 0);
    _prcopt.tidecorr = 0; //im.GetIntKey("tidecorr", 0);
    _prcopt.niter    = 1; //im.GetIntKey("numiter", 1);
    _prcopt.intpref  = 0; //im.GetIntKey("intprefobs", 0);
    _prcopt.sbassatsel= 0; //im.GetIntKey("sbassat", 0);
	
	_prcopt.xlim[0] = _prcopt.xlim[1] = 0.;
	_prcopt.ylim[0] = _prcopt.ylim[1] = 0.;

    _prcopt.eratio[0]= 100.; //im.GetFloatKey("measeratio1", 100.0);
    _prcopt.eratio[1]= 100.; //im.GetFloatKey("measeratio2", 100.0);
    _prcopt.err[1]   = 0.003; //im.GetFloatKey("measerr2", 0.003);
    _prcopt.err[2]   = 0.003; //im.GetFloatKey("measerr3", 0.003);
    _prcopt.err[3]   = 0.; //im.GetFloatKey("measerr4", 0.000);
    _prcopt.err[4]   = 10.; //im.GetFloatKey("measerr5", 10.000);
    _prcopt.prn[0]   = 1E-4; //im.GetFloatKey("prnoise1", 1E-4);
    _prcopt.prn[1]   = 1E-3; //im.GetFloatKey("prnoise2", 1E-3);
    _prcopt.prn[2]   = 1E-4; //im.GetFloatKey("prnoise3", 1E-4);
    _prcopt.prn[3]   = 1E+1; //im.GetFloatKey("prnoise4", 1E+1);
    _prcopt.prn[4]   = 1E+1; //im.GetFloatKey("prnoise5", 1E+1);
    _prcopt.sclkstab = 5E-12; //im.GetFloatKey("satclkstab", 5E-12);
    _prcopt.thresar  = 3.; //im.GetFloatKey("validthresar", 3.0);
    _prcopt.elmaskar = 0. * D2R; //im.GetFloatKey("elmaskar", 0.0) * D2R;
    _prcopt.elmaskhold= 0. * D2R; //im.GetFloatKey("elmaskhold", 0.0) * D2R;
    _prcopt.thresslip= 0.05; //im.GetFloatKey("slipthres", 0.05);
    _prcopt.maxtdiff  = 30.; //im.GetFloatKey("maxagediff", 30.0);
    _prcopt.maxgdop  = 30.; //im.GetFloatKey("rejectgdop", 30.0);
    _prcopt.maxinno  = 30.; //im.GetFloatKey("rejectthres", 30.0);
	int BaseLineConst = 0; //im.GetIntKey("baselineconst", 0);
    if ( BaseLineConst ) {
        _prcopt.baseline[0] = 0.; //im.GetFloatKey("baselinelen", 0.0);
        _prcopt.baseline[1] = 0.; //im.GetFloatKey("baselinesig", 0.0);
    } else {
        _prcopt.baseline[0] = 0.0;
        _prcopt.baseline[1] = 0.0;
    }

	int RovPosType = 0; //im.GetIntKey("rovpostype", 0);
    if ( PosMode != PMODE_FIXED && PosMode != PMODE_PPP_FIXED ) {
        for (int i = 0; i < 3; i++) 
			_prcopt.ru[i] = 0.0;
	} else if ( RovPosType <= 2 ) {
		_prcopt.ru[0] = 0.; //im.GetFloatKey("rovpos1", 0.0);
		_prcopt.ru[1] = 0.; //im.GetFloatKey("rovpos2", 0.0);
		_prcopt.ru[2] = 0.; //im.GetFloatKey("rovpos3", 0.0);
    } else 
		_prcopt.rovpos = RovPosType - 2; // 1:single,2:posfile,3:rinex
    
    int RefPosType = 5; //im.GetIntKey("refpostype", 5);
	if ( PosMode == PMODE_SINGLE || PosMode == PMODE_MOVEB ) {
        for (int i = 0; i < 3; i++) 
			_prcopt.rb[i] = 0.0;
    } else if ( RefPosType <= 2 ) {
		_prcopt.rb[0] = 0.; //im.GetFloatKey("refpos1", 0.0);
		_prcopt.rb[1] = 0.; //im.GetFloatKey("refpos2", 0.0);
		_prcopt.rb[2] = 0.; //im.GetFloatKey("refpos3", 0.0);
    } else 
		_prcopt.refpos = RefPosType - 2;
    
	int RovAntPcv = 0; //im.GetIntKey("rovantpcv", 0);
    if ( RovAntPcv ) {
        strcpy(_prcopt.anttype[0], ""); //im.GetStrKey("rovant", "").c_str());
        _prcopt.antdel[0][0] = 0.; //im.GetFloatKey("rovante", 0.0);
        _prcopt.antdel[0][1] = 0.; //im.GetFloatKey("rovantn", 0.0);
        _prcopt.antdel[0][2] = 0.; //im.GetFloatKey("rovantu", 0.0);
    }
	int RefAntPcv = 0; //im.GetIntKey("refantpcv", 0);
    if ( RefAntPcv ) {
        strcpy(_prcopt.anttype[1], ""); //im.GetStrKey("refant", "").c_str());
        _prcopt.antdel[1][0] = 0.; //im.GetFloatKey("refante", 0.0);
        _prcopt.antdel[1][1] = 0.; //im.GetFloatKey("refantn", 0.0);
        _prcopt.antdel[1][2] = 0.; //im.GetFloatKey("refantu", 0.0);
    }
	std::string ExSats = ""; //im.GetStrKey("exsats", "");
	if ( !ExSats.empty() ) { // excluded satellites
        strcpy(buff, ExSats.c_str());
        for ( p = strtok(buff," "); p; p = strtok(NULL, " ")) {
            if ( *p == '+' ) {
				ex = 2; 
				p++;
			} else 
				ex = 1;
            if ( !(sat = satid2no(p)) ) 
				continue;
            _prcopt.exsats[sat-1] = ex;
        }
    }
    // solution options
    _solopt.posf     = 0; //im.GetIntKey("solformat", 0);
	int TimeFormat = 1; //im.GetIntKey("timeformat", 1);
    _solopt.times    = TimeFormat == 0 ? 0 : TimeFormat/* - 1*/;
    _solopt.timef    = TimeFormat == 0 ? 0 : 1;
	int TimeDecimal = 3; //im.GetIntKey("timedecimal", 3);
    _solopt.timeu    = TimeDecimal <= 0 ? 0 : TimeDecimal;
    _solopt.degf     = 0; //im.GetIntKey("latlonformat", 0);
    _solopt.outhead  = 1; //im.GetIntKey("outputhead", 1);
    _solopt.outopt   =1; //im.GetIntKey("outputopt", 1);
    _solopt.datum    =0; //im.GetIntKey("outputdatum", 0);
    _solopt.height   =0; //im.GetIntKey("outputheight", 0);
    _solopt.geoid    =0; //im.GetIntKey("outputgeoid", 0);
    _solopt.solstatic=0; //im.GetIntKey("solstatic", 0); // per soluzione statica stampa un solo valore
    _solopt.sstat    =0; //im.GetIntKey("debugstatus", 0);
    _solopt.trace    =0; //im.GetIntKey("debugtrace", 0);

	std::string FieldSep = ""; //im.GetStrKey("fieldsep", "");
    strcpy(_solopt.sep, FieldSep != "" ? FieldSep.c_str() : " ");
    sprintf(_solopt.prog, "%s ver.%s", PRGNAME, VER_RTKLIB);
    
    // file options
	strcpy(_filopt.satantp, ""); //im.GetStrKey("satpcvfile", "").c_str());
    strcpy(_filopt.rcvantp, ""); //im.GetStrKey("antpcvfile", "").c_str());
    strcpy(_filopt.stapos, ""); //im.GetStrKey("staposfile", "").c_str());
    strcpy(_filopt.geoid, ""); //im.GetStrKey("geoiddatafile", "").c_str());
    strcpy(_filopt.iono, ""); //im.GetStrKey("ionofile", "").c_str());
    strcpy(_filopt.dcb, ""); //im.GetStrKey("dcbfile", "").c_str());
    
    return 1;
}
int rinex_post::_obsToNav(const char *obsfile, char *navfile)
{
    char *p;
    strcpy(navfile, obsfile);
    if ( !(p = strrchr(navfile, '.')) ) 
		return 0;
    if ( strlen(p) == 4 && *(p + 3) == 'o')
		*(p + 3) = '*';
    else if ( strlen(p) == 4 && *(p + 3) == 'd')
		*(p + 3) = '*';
    else if ( strlen(p) == 4 && *(p + 3) == 'O')
		*(p + 3) = '*';
    else if ( !strcmp(p, ".obs") ) 
		strcpy(p, ".*nav");
    else if ( !strcmp(p,".OBS") ) 
		strcpy(p,".*NAV");
    else if ( !strcmp(p, ".gz") || !strcmp(p, ".Z") ) {
        if ( *(p - 1)=='o' ) 
			*(p - 1) = '*';
        else if ( *(p - 1) == 'd' )
			*(p - 1) = '*';
        else if ( *(p - 1) == 'O' )
			*(p - 1) = '*';
        else 
			return 0;
    }
    else 
		return 0;
    return 1;
}

int rinex_post::ExecProc()
{
    gtime_t ts = {0}, te = {0};
    double ti = 0.0, tu = 0.0;
    int n = 0, stat;
    char infile_[5][1024] = {""}, *infile[5], outfile[1024];
    char *p, *q, *r;
    
    // get processing options

    //if (TimeStart->Checked) ts=GetTime1();
    //if (TimeEnd  ->Checked) te=GetTime2();
    //if (TimeIntF ->Checked) ti=str2dbl(TimeInt ->Text);
    //if (TimeUnitF->Checked) tu=str2dbl(TimeUnit->Text)*3600.0;

	/*if ( !read_options() ) 
		return 0;*/
    
    // set input/output files
    for (int i = 0; i < 5; i++) 
		infile[i] = infile_[i];
    
    strcpy(infile[n++], _inputFile1.c_str());
    
    if ( PMODE_DGPS <= _prcopt.mode && _prcopt.mode <= PMODE_FIXED ) {
        strcpy(infile[n++], _inputFile2.c_str());
    }
	if ( !_inputFile3.empty() ) {
        strcpy(infile[n++], _inputFile3.c_str());
    } else if ( (_prcopt.navsys & SYS_GPS) && !_obsToNav(_inputFile1.c_str(), infile[n++]) ) {
        showmsg("error: no gps navigation data");
        return 0;
    }
	if ( !_inputFile31.empty() ) {
        strcpy(infile[n++], _inputFile31.c_str());
	}
	if ( !_inputFile4.empty() ) {
        strcpy(infile[n++], _inputFile4.c_str());
    }
	if ( !_inputFile5.empty() ) {
		strcpy(infile[n++], _inputFile5.c_str());
    }
	/*if ( _outputFile.empty() ) {
        showmsg("error: no output file");
        return 0;
	}*/
    strcpy(outfile, _outputFile.c_str());
    
	std::string RovList;
    /*for (int i = 0; i < 10; i++) {
        RovList += im.GetStrKey("opt", Sprintf("rovlist%d" , i + 1), "");
    }*/
	std::string BaseList;
    /*for (int i = 0; i < 10; i++) {
        BaseList += im.GetStrKey("opt", Sprintf("baselist%d", i + 1), "");
    }*/
    for (p = (char*) RovList.c_str() ; *p; p++) {
        if ( (p = strstr(p, "@@")) ) 
			strncpy(p, "\r\n", 2); 
		else
			break;
    }
    for (p = (char*) BaseList.c_str(); *p  ;p++) {
        if ( (p = strstr(p, "@@")) )
			strncpy(p, "\r\n", 2);
		else
			break;
    }

    // set rover and base station list
   char* rov = new char [strlen(RovList.c_str())];
   char*  base = new char [strlen(BaseList.c_str())];
    
    for (p = (char*) RovList.c_str(), r = rov; *p; p = q + 2) {
        if ( !(q = strstr(p, "\r\n")) ) {
            if ( *p != '#' ) 
				strcpy(r, p);
			break;
        } else if ( *p != '#' ) {
            strncpy(r, p, q - p);
			r += q - p;
            strcpy(r++, " ");
        }
    }
    for (p = (char*) BaseList.c_str(), r = base; *p; p = q + 2) {
        if ( !(q = strstr(p, "\r\n")) ) {
            if ( *p!='#' ) 
				strcpy(r, p);
			break;
        } else if ( *p!='#' ) {
            strncpy(r, p, q - p);
			r += q - p;
            strcpy(r++, " ");
        }
    }
	showmsg("reading...");
    
    // post processing positioning
	if ( _prcopt.mode == 3 )
		_prcopt.modear = 3; // per modalità statica
    if ( (stat = postpos(ts, te, ti, tu, &_prcopt, &_solopt, &_filopt, infile, n, outfile, rov, base)) == 1 ) {
        showmsg("aborted");
    }
    delete [] rov;
    delete [] base;
    
    return stat;
}
/*************************/
//bool RecordData(const std::string& nome, const std::string& code, std::vector<DPOINT>& dp)
//{
//	vConn fd;
//	fd._cad = new CAD;
//	fd._cad->InitmVar();
//	
//	fd._cad->Cset->SetCod("code");
//	fd._cad->Cset->SetProven(1);
//	//fd._cad->Cset->Penna = _fd._cad->Vmap.GetLayerColor("code");
//
//	Fname fn1(nome);
//	fn1.SetNome(code);
//	fn1.SetExt("gdf");
//	if ( !fd.CreateGDF(fn1) )
//		return false;
//	if ( !fd.OpenGDFw(fn1) )
//		return false;
//
//	/*DDATA dd;
//	dd.SetCod(code.c_str());
//	dd.ActCod = 1;
//	dd.proven = 1;*/
//	fd._cad->Uid.SetFast();
//	fd.Unmap();
//	for (UINT i = 0; i < dp.size(); i++) {
//		FVDATA fv1;
//		fv1.Punctual(-1, dp[i].x,  dp[i].y,  dp[i].z);
//		fd.FeatureCreate(fv1);
//	}
//
//	fd.CloseGDF();
//
//	fd._cad->ResetmVar();
//	delete fd._cad;
//	return true;
//}
// elabora posizione senza ref station
bool RinexPost(const std::string& rover, const std::string& out, vGPS* data) //, Abort* ab)
{
	rinex_post rp(PMODE_SINGLE);
	rp.set_rover(rover);
	rp.set_out(out);
	pr = data;
	//myabort = ab;

	return rp.ExecProc() == 0;
}

bool RinexPost(const std::string& rover, const std::string& base, const std::string& out, MBR* mbr, vGPS* data, GPS_OPT* gps) //, Abort* ab)
{
	rinex_post rp(PMODE_KINEMA); //(PMODE_SINGLE);
	rp.set_rover(rover);
	if ( !base.empty() )
		rp.set_base(base);
	rp.set_out(out);

	rp.set_mode(gps->Position_mode);
	rp.set_nav(gps->Nav_sys);
	rp.set_type(gps->Solution_type);
	rp.set_freq(gps->frequence);
	//rp.set_max_base(gps->max_base_dst);

	rp.set_lim(mbr);
	pr = data;
	//myabort = ab;

	return rp.ExecProc() == 0;
}
int Crx2Rnx(const char* crx)
{
	return crx2rnx(crx);
}
int Raw2Rnx(const char* crx, const char* ext, std::vector<std::string>& vs)
{
	char* ofile[7];
	char of[7][1024];
	for (int i = 0; i < 7; i++) {
		of[i][0] = '\0';
		ofile[i] = of[i];
	}

	int ret = raw2rnx(crx, ext, ofile);

	if ( ret == 1 ) {
		for (int i = 0; i < 7; i++) {
			if ( strlen(ofile[i]) )
				vs.push_back(ofile[i]);
		}
	}

	return ret;
}

