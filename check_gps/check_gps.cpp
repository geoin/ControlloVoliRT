
#include "check_gps.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/AutoPtr.h"
#include "Poco/StringTokenizer.h"
#include "Poco/String.h"

#include <iostream>

#include <set>

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::AutoPtr;

check_gps::check_gps(): _helpRequested(false) 
{
}
void check_gps::initialize(Application& self)
{
	loadConfiguration(); // load default configuration files, if present
	Application::initialize(self);
	// add your own initialization code here
}
void check_gps::uninitialize()
{
	// add your own uninitialization code here
	Application::uninitialize();
}

void check_gps::reinitialize(Application& self)
{
	Application::reinitialize(self);
	// add your own reinitialization code here
}
void check_gps::defineOptions(OptionSet& options)
{
	Application::defineOptions(options);

	options.addOption(
		Option("help", "h", "mostra le informazioni sui parametri da specificare")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<check_gps>(this, &check_gps::_handleHelp)));

	options.addOption(
		Option("prj", "j", "Specifica il file di progetto")
			.required(false)
			.repeatable(false)
			.argument("file di testo")
			.callback(OptionCallback<check_gps>(this, &check_gps::_handlePrj)));
	
	options.addOption(
		Option("rfolder", "r", "Specifica la crtella con i dati del rover")
			.required(false)
			.repeatable(false)
			.argument("shape-file")
			.callback(OptionCallback<check_gps>(this, &check_gps::_handlerfolder)));

	options.addOption(
		Option("bfolder", "b", "Specifica la crtella con i dati delle basi")
			.required(false)
			.repeatable(false)
			.argument("cartella")
			.callback(OptionCallback<check_gps>(this, &check_gps::_handlebfolder)));

	options.addOption(
		Option("strip", "s", "Specifica il file con gli assi di volo")
			.required(false)
			.repeatable(false)
			.argument("shape file")
			.callback(OptionCallback<check_gps>(this, &check_gps::_handleStrip)));
	
	options.addOption(
		Option("rif", "i", "Specifica il file con i valori di riferimento")
			.required(false)
			.repeatable(false)
			.argument("file di testo")
			.callback(OptionCallback<check_gps>(this, &check_gps::_handleRif)));
}
void check_gps::_handlePrj(const std::string & name, const std::string & value)
{
	int a = 1;
}
void check_gps::_handlerfolder(const std::string & name, const std::string & value)
{
	_gps.set_rover_folder(value);
}
void check_gps::_handlebfolder(const std::string & name, const std::string & value)
{
	_gps.set_base_folder(value);
}
void check_gps::_handleStrip(const std::string & name, const std::string & value)
{
	int a = 1;
}
void check_gps::_handleRif(const std::string & name, const std::string & value)
{
	int a = 1;
}
void check_gps::_handleHelp(const std::string& name, const std::string& value)
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}
void check_gps::_handleDefine(const std::string& name, const std::string& value)
{
	defineProperty(value);
}
void check_gps::_handleConfig(const std::string& name, const std::string& value) 
{
	loadConfiguration(value);
}
void check_gps::displayHelp() 
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPZIONI");
	helpFormatter.setHeader("Applicazione per la verifica dei dati GPS dell'aereo");
	helpFormatter.format(std::cout);
}
	
void check_gps::defineProperty(const std::string& def)
{
	std::string name;
	std::string value;
	std::string::size_type pos = def.find('=');
	if ( pos != std::string::npos ) {
		name.assign(def, 0, pos);
		value.assign(def, pos + 1, def.length() - pos);
	}
	else 
		name = def;
	config().setString(name, value);
}

int check_gps::main(const std::vector<std::string>& args) 
{
	if ( !_helpRequested ) {
		//logger().information("Arguments to main():");
		//for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
		//	logger().information(*it);
		//}
		//logger().information("Application properties:");
		//printProperties("");
		_gps.set_out_folder("C:/Google_drive/Regione Toscana Tools/Dati_test/Out");
		_gps.set_rover_folder("C:/Google_drive/Regione Toscana Tools/Dati_test/GPS-t/Rinex_aereo");
		_gps.set_base_folder("C:/Google_drive/Regione Toscana Tools/Dati_test/GPS-t/Rinex_stazioni_di_terra");
		_gps.set_db_name("C:/Google_drive/Regione Toscana Tools/Dati_test/Rilievo/cast_pescaia.sqlite");

		_gps.run();
	}
	return Application::EXIT_OK;
}

POCO_APP_MAIN(check_gps)
