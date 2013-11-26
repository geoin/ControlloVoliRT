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
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;


check_photo::check_photo(): _helpRequested(false) 
{
}
void check_photo::initialize(Application& self) 
{
	loadConfiguration(); // load default configuration files, if present
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
		Option("flight", "f", "Specifica operazione di verifica del volo")
			.required(false)
			.repeatable(false)
			//.argument("shape-file")
			.callback(OptionCallback<check_photo>(this, &check_photo::handleFlight)));
			
	options.addOption(
		Option("prj", "j", "Specifica il file di progetto")
			.required(false)
			.repeatable(false)
			.argument("shape-file")
			.callback(OptionCallback<check_photo>(this, &check_photo::handlePrj)));
	
	options.addOption(
		Option("pline", "p", "Specifica il file con le linee di volo proposte")
			.required(false)
			.repeatable(false)
			.argument("shape-file")
			.callback(OptionCallback<check_photo>(this, &check_photo::handlePline)));

	options.addOption(
		Option("fline", "i", "Specifica il file con le linee di volo effettuate")
			.required(false)
			.repeatable(false)
			.argument("shape-file")
			.callback(OptionCallback<check_photo>(this, &check_photo::handleFline)));

	options.addOption(
		Option("pcent", "e", "Specifica il file con i centri di presa")
			.required(false)
			.repeatable(false)
			.argument("file di testo")
			.callback(OptionCallback<check_photo>(this, &check_photo::handlePcent)));
	
	options.addOption(
		Option("carto", "c", "Specifica il file con le aree da cartografare")
			.required(false)
			.repeatable(false)
			.argument("shape-file")
			.callback(OptionCallback<check_photo>(this, &check_photo::handleCarto)));
	
	options.addOption(
		Option("cam", "m", "Specifica il file ascii con focale la focale della fotocamera")
			.required(false)
			.repeatable(false)
			.argument("file di testo")
			.callback(OptionCallback<check_photo>(this, &check_photo::handleCam)));

	options.addOption(
		Option("dtm", "g", "Specifica il file con il modello numerico del terreno")
			.required(false)
			.repeatable(false)
			.argument("file di testo")
			.callback(OptionCallback<check_photo>(this, &check_photo::handleDtm)));
}
void check_photo::handleCam(const std::string & name, const std::string & value)
{
	int a = 1;
}
void check_photo::handlePcent(const std::string & name, const std::string & value) {
	int a = 1;
}
void check_photo::handleDtm(const std::string & name, const std::string & value) {
	int a = 1;
}
void check_photo::handleCarto(const std::string & name, const std::string & value) {
	int a = 1;
}
void check_photo::handleFlight(const std::string & name, const std::string & value) {
	int a = 1;
}
void check_photo::handlePrj(const std::string & name, const std::string & value) {
	int a = 1;
}
void check_photo::handlePline(const std::string & name, const std::string & value) {
	int a = 1;
}
void check_photo::handleFline(const std::string & name, const std::string & value) {
	int a = 1;
}
void check_photo::handleHelp(const std::string& name, const std::string& value) {
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}

void check_photo::handleDefine(const std::string& name, const std::string& value)
{
	defineProperty(value);
}

void check_photo::handleConfig(const std::string& name, const std::string& value)
{
	loadConfiguration(value);
}
	
void check_photo::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPZIONI");
	helpFormatter.setHeader("Applicazione per la verifica del volo effettuato o del progetto di volo");
	helpFormatter.format(std::cout);
}
	
void check_photo::defineProperty(const std::string& def)
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

int check_photo::main(const std::vector<std::string>& args) 
{
	if ( !_helpRequested ) {
		_phe.set_cam_name("C:/Google_drive/Regione Toscana Tools/Dati_test/Vexcel_ucxp_263.xml");
		_phe.set_vdp_name("C:/Google_drive/Regione Toscana Tools/Dati_test/180710_CAST_PESC_CP.txt");
		_phe.set_dem_name("C:/Google_drive/Regione Toscana Tools/Dati_test/scarlino_50.asc");
		_phe.set_proj_dir("C:/Google_drive/Regione Toscana Tools/Dati_test/cast_pescaia");
		_phe.set_checkType(photo_exec::fli_type);

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
	
void check_photo::printProperties(const std::string& base)
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

POCO_APP_MAIN(check_photo)
