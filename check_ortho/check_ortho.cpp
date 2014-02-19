/*
    File: check_ortho.cpp
    Author:  F.Flamigni
    Date: 2013 November 12
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

#include "check_ortho.h"
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


check_ortho::check_ortho(): _helpRequested(false) 
{
}
void check_ortho::initialize(Application& self) 
{
	//loadConfiguration();
	Application::initialize(self);
}

void check_ortho::uninitialize() 
{
	// add your own uninitialization code here
	Application::uninitialize();
}
void check_ortho::reinitialize(Application& self)
{
	Application::reinitialize(self);
}
void check_ortho::defineOptions(OptionSet& options)
{
	Application::defineOptions(options);

	options.addOption(
		Option("help", "h", "mostra le informazioni sui parametri da specificare")
			.required(false)
			.repeatable(false)
			.callback(OptionCallback<check_ortho>(this, &check_ortho::handleHelp)));

		
	options.addOption(
		Option("dir", "d", "Specifica la cartella del progetto")
			.required(false)
			.repeatable(false)
			.argument("value")
			.callback(OptionCallback<check_ortho>(this, &check_ortho::handlePrjDir)));
	
	options.addOption(
		Option("img", "i", "Specifica la cartella delle ortho immagini")
			.required(false)
			.repeatable(false)
			.argument("value")
			.callback(OptionCallback<check_ortho>(this, &check_ortho::handleImgDir)));
	
	//options.addOption(
	//	Option("scale", "s", "Specifica la scala di lavoro")
	//		.required(false)
	//		.repeatable(false)
	//		.argument("value")
	//		.callback(OptionCallback<check_ortho>(this, &check_ortho::handleScale)));
}
void check_ortho::handleImgDir(const std::string& name, const std::string & value)
{
	_otx.set_img_dir(value);
}
void check_ortho::handlePrjDir(const std::string& name, const std::string& value) 
{
	_otx.set_proj_dir(value);
}
//void check_ortho::handleScale(const std::string& name, const std::string& value)
//{
//	_otx.set_ref_scale(value);
//}

void check_ortho::handleHelp(const std::string& name, const std::string& value)
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}

void check_ortho::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPZIONI");
	helpFormatter.setHeader("Applicazione per la verifica dei orto immagini");
	helpFormatter.format(std::cout);
}
	
int check_ortho::main(const std::vector<std::string>& args) 
{
	if ( !_helpRequested ) {
		_otx.run();
	}
	return Application::EXIT_OK;
}
	
POCO_APP_MAIN(check_ortho)
