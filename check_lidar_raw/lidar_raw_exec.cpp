#include "lidar_raw_exec.h"

#include "Poco/Path.h"
#include "Poco/SharedPtr.h"

#define GEO_DB_NAME "geo.sqlite"

void lidar_raw_exec::set_proj_dir(const std::string& proj) {
    _proj_dir = proj;
}

bool lidar_raw_exec::openDBConnection() {
	try {
		Poco::Path geodb(_proj_dir, GEO_DB_NAME);
		cnn.open(geodb.toString());
		return true;
	} catch (const std::exception&) {
		return false;
	}
}

bool lidar_raw_exec::run() {
    std::cout << "Running" << std::endl;
    return true;
}
