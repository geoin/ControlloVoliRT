#ifndef LIDAR_RAW_EXEC_H
#define LIDAR_RAW_EXEC_H

#include <iostream>
#include <string>

class lidar_raw_exec {

public:
    bool run();

    void set_proj_dir(const std::string&);
private:
    std::string _proj_dir;
};

#endif
