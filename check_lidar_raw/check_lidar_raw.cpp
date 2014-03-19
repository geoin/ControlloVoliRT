#include "check_lidar_raw.h"

#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::OptionCallback;

check_lidar_raw::check_lidar_raw() : _helpRequested(false) {

}

void check_lidar_raw::initialize(Application& self) {
    Application::initialize(self);
}

void check_lidar_raw::uninitialize() {
    Application::uninitialize();
}

void check_lidar_raw::reinitialize(Application& self) {
    Application::reinitialize(self);
}

void check_lidar_raw::defineOptions(OptionSet& options) {
    Application::defineOptions(options);

    options.addOption(
        Option("help", "h", "Mostra le informazioni sui parametri da specificare")
            .required(false)
            .repeatable(false)
            .callback(OptionCallback<check_lidar_raw>(this, &check_lidar_raw::handleHelp)));

    options.addOption(
        Option("dir", "d", "Specifica la cartella del progetto")
            .required(true)
            .repeatable(false)
            .argument("value")
            .callback(OptionCallback<check_lidar_raw>(this, &check_lidar_raw::handlePrjDir)));
}

void check_lidar_raw::handleHelp(const std::string& name, const std::string& value) {
    _helpRequested = true;

    displayHelp();
    stopOptionsProcessing();
}

void check_lidar_raw::handlePrjDir(const std::string& name, const std::string& value) {
    _check.set_proj_dir(value);
}

void check_lidar_raw::displayHelp() {
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPZIONI");
    helpFormatter.setHeader("Applicazione per la verifica dei dati raw");
    helpFormatter.format(std::cout);
}

int check_lidar_raw::main(const std::vector<std::string>& args) {
	if (_helpRequested) {
		return Application::EXIT_OK;
	}

    try {

		bool ret = true;
		ret = _check.openDBConnection();
        if (ret) {
			if (!_check.init()) {
				throw std::runtime_error("Error while initializing check");
			}
            ret = _check.run();
        }
		
		return ret ? Application::EXIT_OK : Application::EXIT_SOFTWARE;

    } catch (const std::exception& ex) {
		lidar_raw_exec::Error("Main", ex);
        return Application::EXIT_SOFTWARE;
    }
}

POCO_APP_MAIN(check_lidar_raw)
