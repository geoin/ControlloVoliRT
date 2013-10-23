#ifndef CHECK_GPS_H
#define CHECK_GPS_H

#include "gps.h"
#include "Poco/Util/Application.h"
#include <spatialite/sqlite3.h>

class gps_exec {
public:
	enum gps_type {
		rover_type = 0,
		base_type = 1
	};
	gps_exec() {}
	bool run(void);
	void set_out_folder(const std::string& nome);
	void set_rover_folder(const std::string& nome);
	void set_base_folder(const std::string& nome);
	void set_db_name(const std::string& nome){ _db_name.assign(nome); }

private:
	std::string _getnome(const std::string& nome, gps_type type);
	std::string _hathanaka(const std::string& nome);
	std::vector<std::string> _rawConv(const std::string& nome);
	bool RecordData(const std::string& nome, const std::string& code, vGPS& vg, MBR* mbr);
	bool SingleTrack(const std::string& nome, const std::string& code, std::vector<vGPS*>& vvg, MBR* mbr);
	void _record_base_file(const std::vector<DPOINT>& basi, const std::vector<std::string>& vs_base);

	std::string _rover_name;
	std::string _sigla_base;
	std::vector<std::string> _vs_base;
	GPS_OPT _gps_opt;

	std::string _rover_folder;
	std::string _base_folder;
	std::string _out_folder;
	std::string _db_name;

	sqlite3 *db_handle;
	sqlite3_stmt *stmt;
};

class check_gps: public Poco::Util::Application {
public:
	check_gps();
	gps_exec _gps;
protected:	
	void initialize(Poco::Util::Application& self);
	void uninitialize(void);
	void reinitialize(Poco::Util::Application& self);
	void defineOptions(Poco::Util::OptionSet& options);
	void displayHelp(void);
	void defineProperty(const std::string& def);
	int main(const std::vector<std::string>& args);
	void printProperties(const std::string& base);
	//int run(void);
	//gps_exec _gps;
private:
	void _handlePrj(const std::string & name, const std::string & value);
	void _handlerfolder(const std::string & name, const std::string & value);
	void _handlebfolder(const std::string & name, const std::string & value);
	void _handleStrip(const std::string & name, const std::string & value);
	void _handleRif(const std::string & name, const std::string & value);
	void _handleHelp(const std::string& name, const std::string& value);
	void _handleDefine(const std::string& name, const std::string& value);
	void _handleConfig(const std::string& name, const std::string& value);
	bool _helpRequested;
};
#endif
