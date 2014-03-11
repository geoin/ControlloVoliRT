#ifndef CHECK_LIDAR_FINAL_H
#define CHECK_LIDAR_FINAL_H

#include "lidar_final_exec.h"

#include "Poco/Util/Application.h"

class check_lidar_final: public Poco::Util::Application {
public:
    check_lidar_final();

protected:
    void initialize(Poco::Util::Application& self);
    void uninitialize();
    void reinitialize(Poco::Util::Application& self);
    void defineOptions(Poco::Util::OptionSet& options);
    void displayHelp();
    int main(const std::vector<std::string>& args);
private:
    void handleHelp(const std::string& name, const std::string& value);
    void handlePrjDir(const std::string& name, const std::string & value);

    bool _helpRequested;

    lidar_final_exec _check;
};

#endif
