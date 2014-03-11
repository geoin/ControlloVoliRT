#include "lidar_final_exec.h"

void lidar_final_exec::set_proj_dir(const std::string& proj) {
    _proj_dir = proj;
}

bool lidar_final_exec::run() {
    std::cout << "Running" << std::endl;
    return true;
}
