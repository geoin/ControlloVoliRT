#include "lidar_raw_exec.h"

void lidar_raw_exec::set_proj_dir(const std::string& proj) {
    _proj_dir = proj;
}

bool lidar_raw_exec::run() {
    std::cout << "Running" << std::endl;
    return true;
}
