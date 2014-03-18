#ifndef LIDAR_RAW_EXEC_H
#define LIDAR_RAW_EXEC_H

#include <iostream>
#include <string>

#include "cv/lidar.h"

class lidar_raw_exec {

public:
    void set_proj_dir(const std::string&);

    bool run();
	bool openDBConnection();

private:
    std::string _proj_dir;
	
	CV::Util::Spatialite::Connection cnn;
};

#endif
