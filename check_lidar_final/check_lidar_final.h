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

	void handleTilesPointsPerc(const std::string& name, const std::string& p) { _check.setTilesPointsPerc(std::stoi(p)); }
	void handleClassFilesPerc(const std::string& name, const std::string& p) { _check.setClassFilesPerc(std::stoi(p)); }
	void handleClassPointsPerc(const std::string& name, const std::string& p) { _check.setClassPointsPerc(std::stoi(p)); }
	void handleResampleFilesPerc(const std::string& name, const std::string& p) { _check.setResampleFilesPerc(std::stoi(p)); }
	void handleResamplePointsPerc(const std::string& name, const std::string& p) { _check.setResamplePointsPerc(std::stoi(p)); }
	void handleQuotaPointsPerc(const std::string& name, const std::string& p) { _check.setQuotaPointsPerc(std::stoi(p)); }

    bool _helpRequested;

    lidar_final_exec _check;
};

#endif
