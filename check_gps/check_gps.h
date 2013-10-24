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
	void set_db_name(const std::string& nome){ _db_name.assign(nome); }
	void set_proj_dir(const std::string& nome){ _proj_dir.assign(nome); }

private:
	std::string _getnome(const std::string& nome, gps_type type);
	std::string _hathanaka(const std::string& nome);
	std::vector<std::string> _rawConv(const std::string& nome);
	bool RecordData(const std::string& nome, const std::string& code, vGPS& vg, MBR* mbr);
	bool SingleTrack(const std::string& nome, const std::string& code, std::vector<vGPS*>& vvg, MBR* mbr);
	void _record_base_file(const std::vector<DPOINT>& basi, const std::vector<std::string>& vs_base);
	bool _mission_process(const std::string& folder);

	std::string _rover_name;
	std::string _sigla_base;
	std::vector<std::string> _vs_base;
	GPS_OPT _gps_opt;

	std::string _out_folder;
	std::string _db_name;
	std::string _proj_dir;

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
private:
	void _handlePrj(const std::string & name, const std::string & value);
    void _handleConfigFile(const std::string & name, const std::string & value);
    void _handleConfigType(const std::string & name, const std::string & value);
	void _handleHelp(const std::string& name, const std::string& value);
	bool _helpRequested;
};
#endif
