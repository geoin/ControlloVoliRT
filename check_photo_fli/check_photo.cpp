/*
    File: check_photo.cpp
    Author:  F.Flamigni
    Date: 2013 October 29
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

#include "check_photo.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
//using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;

check_photo::check_photo():_helpRequested(false) 
{
}
void check_photo::initialize(Application& self) 
{
	//loadConfiguration(); // load default configuration files, if present
	Application::initialize(self);
	// add your own initialization code here
}

void check_photo::uninitialize() 
{
	// add your own uninitialization code here
	Application::uninitialize();
}
void check_photo::reinitialize(Application& self)
{
	Application::reinitialize(self);
	// add your own reinitialization code here
}
void check_photo::defineOptions(OptionSet& options)
{
	Application::defineOptions(options);

	options.addOption(
		Option("help", "h", "mostra le informazioni sui parametri da specificare")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<check_photo>(this, &check_photo::handleHelp)));

	options.addOption(
		Option("dir", "d", "Specifica la cartella del progetto")
			.required(false)
			.repeatable(false)
			.argument("value")
			.callback(OptionCallback<check_photo>(this, &check_photo::handlePrjDir)));

	options.addOption(
		Option("flight", "f", "Specifica operazione di verifica del volo")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<check_photo>(this, &check_photo::handleFlight)));
			
	options.addOption(
		Option("proj", "p", "Specifica operazione di verifica del progetto di volo")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<check_photo>(this, &check_photo::handleProject)));

	options.addOption(
		Option("scale", "s", "Specifica la scala di lavoro")
			.required(false)
			.repeatable(false)
			.argument("value")
			.callback(OptionCallback<check_photo>(this, &check_photo::handleScale)));
}
void check_photo::handleHelp(const std::string& name, const std::string& value)
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}
void check_photo::handlePrjDir(const std::string& name, const std::string& value) 
{
	_phe.set_proj_dir(value);
}
void check_photo::handleFlight(const std::string& name, const std::string& value)
{
	_phe.set_checkType(photo_exec::fli_type);
}
void check_photo::handleProject(const std::string& name, const std::string& value)
{
	_phe.set_checkType(photo_exec::Prj_type);
}
void check_photo::handleScale(const std::string& name, const std::string& value)
{
	_phe.set_ref_scale(value);
}
void check_photo::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPZIONI");
	helpFormatter.setHeader("Applicazione per la verifica del volo effettuato o del progetto di volo");
	helpFormatter.format(std::cout);
}
	
int check_photo::main(const std::vector<std::string>& args) 
{
	if ( !_helpRequested ) {
		//_phe.set_proj_dir("C:/Google_drive/Regione Toscana Tools/Dati_test/scarlino");
		//_phe.set_checkType(photo_exec::Prj_type);

		_phe.run();

		//logger().information("Arguments to main():");
		//for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
		//	logger().information(*it);
		//}
		//logger().information("Application properties:");
		////printProperties("");
	}
	return Application::EXIT_OK;
}

POCO_APP_MAIN(check_photo)
