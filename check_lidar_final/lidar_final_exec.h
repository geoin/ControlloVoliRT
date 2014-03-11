#ifndef LIDAR_FINAL_EXEC_H
#define LIDAR_FINAL_EXEC_H

#include <iostream>
#include <string>

class lidar_final_exec {

public:
    bool run();

    void set_proj_dir(const std::string&);
private:
    std::string _proj_dir;
};

#endif
