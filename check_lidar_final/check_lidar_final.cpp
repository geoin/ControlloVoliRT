#include "check_lidar_final.h"

#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"

#include "cv_version.h"

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::OptionCallback;

check_lidar_final::check_lidar_final() : _helpRequested(false) {

}

void check_lidar_final::initialize(Application& self) {
    Application::initialize(self);
}

void check_lidar_final::uninitialize() {
    Application::uninitialize();
}

void check_lidar_final::reinitialize(Application& self) {
    Application::reinitialize(self);
}

void check_lidar_final::defineOptions(OptionSet& options) {
    Application::defineOptions(options);

    options.addOption(
        Option("help", "h", "Mostra le informazioni sui parametri da specificare")
            .required(false)
            .repeatable(false)
            .callback(OptionCallback<check_lidar_final>(this, &check_lidar_final::handleHelp)));

    options.addOption(
        Option("dir", "d", "Specifica la cartella del progetto")
            .required(true)
            .repeatable(false)
            .argument("value")
            .callback(OptionCallback<check_lidar_final>(this, &check_lidar_final::handlePrjDir)));

	
    options.addOption(
        Option("tile", "t", "Percentuale punti tile grezze")
            .required(true)
            .repeatable(false)
            .argument("value")
			.callback(OptionCallback<check_lidar_final>(this, &check_lidar_final::handleTilesPointsPerc)));

	
    options.addOption(
        Option("classF", "cf", "Percentuale file classificazione")
            .required(true)
            .repeatable(false)
            .argument("value")
			.callback(OptionCallback<check_lidar_final>(this, &check_lidar_final::handleClassFilesPerc)));

	
    options.addOption(
        Option("classP", "cp", "Percentuale punti classificazione")
            .required(true)
            .repeatable(false)
            .argument("value")
			.callback(OptionCallback<check_lidar_final>(this, &check_lidar_final::handleClassPointsPerc)));

	 options.addOption(
        Option("resampleF", "rf", "Percentuale file ricampionamento")
            .required(true)
            .repeatable(false)
            .argument("value")
			.callback(OptionCallback<check_lidar_final>(this, &check_lidar_final::handleResampleFilesPerc)));

	
    options.addOption(
        Option("resampleP", "rp", "Percentuale punti ricampionamento")
            .required(true)
            .repeatable(false)
            .argument("value")
			.callback(OptionCallback<check_lidar_final>(this, &check_lidar_final::handleResamplePointsPerc)));

	
    options.addOption(
        Option("quota", "q", "Percentuale punti quota")
            .required(true)
            .repeatable(false)
            .argument("value")
			.callback(OptionCallback<check_lidar_final>(this, &check_lidar_final::handleTilesPointsPerc)));
}

void check_lidar_final::handleHelp(const std::string& name, const std::string& value) {
    _helpRequested = true;

    displayHelp();
    stopOptionsProcessing();
}

void check_lidar_final::handlePrjDir(const std::string& name, const std::string& value) {
    _check.set_proj_dir(value);
}

void check_lidar_final::displayHelp() {
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPZIONI");
    helpFormatter.setHeader("Applicazione per la verifica dei dati lidar");
    helpFormatter.format(std::cout);
}

int check_lidar_final::main(const std::vector<std::string>& args) {
	if (_helpRequested) {
		return Application::EXIT_OK;
	}

	CV::Version::print();

	if (!_check.openDBConnection()) {
		return Application::EXIT_IOERR;
	}

    try {
		_check.readFolders();

		_check.run();
		_check.createReport();

    } catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return Application::EXIT_SOFTWARE;
    }
    return Application::EXIT_OK;
}

POCO_APP_MAIN(check_lidar_final)
