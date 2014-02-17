/*
    File: check_ta.cpp
    Author:  F.Flamigni
    Date: 2013 November 06
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

#include "check_ta.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::OptionCallback;

check_ta::check_ta(): _helpRequested(false) 
{
}
void check_ta::initialize(Application& self) 
{
	Application::initialize(self);
}

void check_ta::uninitialize() 
{
	Application::uninitialize();
}
void check_ta::reinitialize(Application& self)
{
	Application::reinitialize(self);
}
void check_ta::defineOptions(OptionSet& options)
{
	Application::defineOptions(options);

	options.addOption(
		Option("help", "h", "mostra le informazioni sui parametri da specificare")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<check_ta>(this, &check_ta::handleHelp)));

	options.addOption(
		Option("dir", "d", "Specifica la cartella del progetto")
			.required(false)
			.repeatable(false)
			.argument("value")
			.callback(OptionCallback<check_ta>(this, &check_ta::handlePrjDir)));
	
	options.addOption(
		Option("ref", "r", "Specifica il file con gli assetti di riferimento")
			.required(false)
			.repeatable(false)
			.argument("value")
			.callback(OptionCallback<check_ta>(this, &check_ta::handleRef)));

	options.addOption(
		Option("comp", "c", "Specifica il file con gli assetti da confrontare")
			.required(false)
			.repeatable(false)
			.argument("value")
			.callback(OptionCallback<check_ta>(this, &check_ta::handleComp)));
	
	options.addOption(
		Option("obs", "o", "Specifica il file con le osservazioni")
			.required(false)
			.repeatable(false)
			.argument("value")
			.callback(OptionCallback<check_ta>(this, &check_ta::handleObs)));

	//options.addOption(
	//	Option("scale", "s", "Specifica la scala di lavoro")
	//		.required(false)
	//		.repeatable(false)
	//		.argument("value")
	//		.callback(OptionCallback<check_ta>(this, &check_ta::handleScale)));
}
void check_ta::handleHelp(const std::string& name, const std::string& value) 
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}
void check_ta::handlePrjDir(const std::string & name, const std::string & value)
{
	_tae.set_proj_dir(value);
}

void check_ta::handleRef(const std::string& name, const std::string& value)
{
	_tae.set_vdp_name(value);
}
void check_ta::handleComp(const std::string& name, const std::string& value)
{
	_tae.set_vdp_name2(value);
}
void check_ta::handleObs(const std::string& name, const std::string& value)
{
	_tae.set_obs_name(value);
}
//void check_ta::handleScale(const std::string& name, const std::string& value)
//{
//	_tae.set_ref_scale(value);
//}
	
void check_ta::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPZIONI");
	helpFormatter.setHeader("Applicazione per la verifica della triangolazione aerea");
	helpFormatter.format(std::cout);
}
	
int check_ta::main(const std::vector<std::string>& args) 
{
	if ( !_helpRequested ) {

		_tae.run();
	}
	return Application::EXIT_OK;
}
	
POCO_APP_MAIN(check_ta)
