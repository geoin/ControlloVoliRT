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
	loadConfiguration(); // load default configuration files, if present
	Application::initialize(self);
	// add your own initialization code here
}

void check_ortho::uninitialize() 
{
	// add your own uninitialization code here
	Application::uninitialize();
}
void check_ortho::reinitialize(Application& self)
{
	Application::reinitialize(self);
	// add your own reinitialization code here
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
		Option("prj", "j", "Specifica il file di progetto")
			.required(false)
			.repeatable(false)
			.argument("shape-file")
			.callback(OptionCallback<check_ortho>(this, &check_ortho::handlePrj)));
	
	//options.addOption(
	//	Option("dtm", "g", "Specifica il file con il modello numerico del terreno")
	//		.required(false)
	//		.repeatable(false)
	//		.argument("file di testo")
	//		.callback(OptionCallback<check_photo>(this, &check_ortho::handleDtm)));
}
void check_ortho::handleCam(const std::string & name, const std::string & value)
{
	int a = 1;
}
void check_ortho::handlePcent(const std::string & name, const std::string & value) {
	int a = 1;
}


void check_ortho::handlePrj(const std::string & name, const std::string & value) {
	int a = 1;
}


void check_ortho::handleHelp(const std::string& name, const std::string& value) {
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}

void check_ortho::handleDefine(const std::string& name, const std::string& value)
{
	defineProperty(value);
}

void check_ortho::handleConfig(const std::string& name, const std::string& value)
{
	loadConfiguration(value);
}
	
void check_ortho::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPZIONI");
	helpFormatter.setHeader("Applicazione per la verifica dei orto immagini");
	helpFormatter.format(std::cout);
}
	
void check_ortho::defineProperty(const std::string& def)
{
	std::string name;
	std::string value;
	std::string::size_type pos = def.find('=');
	if (pos != std::string::npos) {
		name.assign(def, 0, pos);
		value.assign(def, pos + 1, def.length() - pos);
	} else 
		name = def;
	config().setString(name, value);
}

int check_ortho::main(const std::vector<std::string>& args) 
{
	if ( !_helpRequested ) {
		_otx.set_proj_dir("C:/Google_drive/Regione Toscana Tools/Dati_test/cast_pescaia");
		_otx.set_img_dir("H:/Regione-Toscana/VOLO2010SCARLIO-CASTPESCAIA-ARGENTARIO/6-Ortofoto/Cast_Pescaia");
		_otx.run();

		//logger().information("Arguments to main():");
		//for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
		//	logger().information(*it);
		//}
		//logger().information("Application properties:");
		////printProperties("");
	}
	return Application::EXIT_OK;
}
	
void check_ortho::printProperties(const std::string& base)
{
	AbstractConfiguration::Keys keys;
	config().keys(base, keys);
	if ( keys.empty() ) {
		if ( config().hasProperty(base) ) {
			std::string msg;
			msg.append(base);
			msg.append(" = ");
			msg.append(config().getString(base));
			logger().information(msg);
		}
	} else {
		for ( AbstractConfiguration::Keys::const_iterator it = keys.begin(); it != keys.end(); ++it ) {
			std::string fullKey = base;
			if ( !fullKey.empty() )
				fullKey += '.';
			fullKey.append(*it);
			printProperties(fullKey);
		}
	}
}

POCO_APP_MAIN(check_ortho)
