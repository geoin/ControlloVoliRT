/*
    File: check_lidar.cpp
    Author:  F.Flamigni
    Date: 2013 November 22
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

#include "check_lidar.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;


check_lidar::check_lidar(): _helpRequested(false) 
{
}
void check_lidar::initialize(Application& self) 
{
	loadConfiguration(); // load default configuration files, if present
	Application::initialize(self);
	// add your own initialization code here
}

void check_lidar::uninitialize() 
{
	// add your own uninitialization code here
	Application::uninitialize();
}
void check_lidar::reinitialize(Application& self)
{
	Application::reinitialize(self);
	// add your own reinitialization code here
}
void check_lidar::defineOptions(OptionSet& options)
{
	Application::defineOptions(options);

	options.addOption(
		Option("help", "h", "mostra le informazioni sui parametri da specificare")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<check_lidar>(this, &check_lidar::handleHelp)));

	options.addOption(
		Option("dir", "d", "Specifica il file di progetto")
			.required(false)
			.repeatable(false)
			.argument("shape-file")
			.callback(OptionCallback<check_lidar>(this, &check_lidar::handlePrjDir)));
	
	options.addOption(
		Option("flight", "f", "Specifica operazione di verifica del volo")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<check_lidar>(this, &check_lidar::handleFlight)));
			
	options.addOption(
		Option("proj", "p", "Specifica operazione di verifica del progetto di volo")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<check_lidar>(this, &check_lidar::handleProject)));
	options.addOption(
		Option("scale", "s", "Specifica la scala di lavoro")
			.required(false)
			.repeatable(false)
			.argument("value")
			.callback(OptionCallback<check_lidar>(this, &check_lidar::handleScale)));
}
void check_lidar::handleFlight(const std::string& name, const std::string& value)
{
	_lix.set_checkType(lidar_exec::FLY_TYPE);
}
void check_lidar::handleProject(const std::string& name, const std::string& value)
{
	_lix.set_checkType(lidar_exec::PRJ_TYPE);
}
void check_lidar::handlePrjDir(const std::string & name, const std::string & value)
{
	_lix.set_proj_dir(value);
}
void check_lidar::handleScale(const std::string & name, const std::string & value)
{
}
void check_lidar::handleHelp(const std::string& name, const std::string& value)
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}

void check_lidar::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPZIONI");
	helpFormatter.setHeader("Applicazione per la verifica del volo / progetto di ripresa lidar");
	helpFormatter.format(std::cout);
}
	
int check_lidar::main(const std::vector<std::string>& args) 
{
	if ( !_helpRequested ) {
        _lix.run();
	}
	return Application::EXIT_OK;
}
	
POCO_APP_MAIN(check_lidar)
