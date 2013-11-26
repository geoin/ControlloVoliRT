/* 
	File: check_gps.cpp
	Author:  F.Flamigni
	Date: 2013 October 22
	Comment:

	Disclaimer:
		This file is part of RT_Controllo_Voli.

		Tabula is free software: you can redistribute it and/or modify
		it under the terms of the GNU Lesser General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version.

		Tabula is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU Lesser General Public License for more details.

		You should have received a copy of the GNU Lesser General Public License
		along with Tabula.  If not, see <http://www.gnu.org/licenses/>.


		Copyright (C) 2013 Geoin s.r.l.

*/
#include "check_gps.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
//#include "Poco/Util/AbstractConfiguration.h"
//#include "Poco/AutoPtr.h"
//#include "Poco/StringTokenizer.h"
#include "Poco/String.h"
#include <iostream>
#include <set>

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
//using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
//using Poco::AutoPtr;

check_gps::check_gps(): _helpRequested(false) 
{
}
void check_gps::initialize(Application& self)
{
    Application::initialize(self);
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
        Option("help", "h", "mostra le opzioni di linea di comando")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<check_gps>(this, &check_gps::_handleHelp)));

	options.addOption(
        Option("prj", "p", "Specifica il file di progetto")
			.required(false)
			.repeatable(false)
			.argument("file di testo")
			.callback(OptionCallback<check_gps>(this, &check_gps::_handlePrj)));
}

void check_gps::_handlePrj(const std::string & name, const std::string & value)
{
    // controllo esistenza directory

    // esistenza db con lo stesso nome della directory

    // impostare il valore del path al progetto
    //_proj_dir = ....value
}
void check_gps::_handleHelp(const std::string& name, const std::string& value)
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
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
		_gps.set_proj_dir("C:/Google_drive/Regione Toscana Tools/Dati_test/cast_pescaia");

		_gps.run();
	}
	return Application::EXIT_OK;
}

POCO_APP_MAIN(check_gps)
