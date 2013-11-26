/*
    File: cvloader.cpp
    Author:  A.Comparini
    Date: 2013 November 03
    Comment:
        Load files and data for cartography aerial missions to be processed for suitable precision
        and reports creation

    Disclaimer:
        This file is part of CV a framework for verifying aerial missions for cartography.

        CV is free software: you can redistribute it and/or modify
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

#include "Poco/Util/Application.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/AutoPtr.h"
#include "Poco/File.h"
#include <iostream>
#include <sqlite3.h>
#include "spatialite.h"

#include "CVUtil/cvspatialite.h"

using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::Util::AbstractConfiguration;
using Poco::Util::OptionCallback;
using Poco::AutoPtr;

#define PLANNED_FLIGHT_LAYER_NAME "AVOLOP"
#define FLIGHT_LAYER_NAME "AVOLOV"
#define SHAPE_CHAR_SET "CP1252"
#define GAUSS_BOAGA_SRID  32632
#define GEOM_COL_NAME "geom"
#define MAX_MSG_LEN 250
#define GEO_DB_NAME "geo.sqlite"

class CVLoader: public Application
    /// This sample demonstrates some of the features of the Util::Application class,
    /// such as configuration file handling and command line arguments processing.
    ///
    /// Try SampleApp --help (on Unix platforms) or SampleApp /help (elsewhere) for
    /// more information.
{
public:
    CVLoader(): _helpRequested(false), _exit_err(Application::EXIT_OK) {
//        printf("Premere return ....");
//        getchar();
    }

protected:
    void initialize(Application& self) {
        loadConfiguration(); // load default configuration files, if present
        Application::initialize(self);
        // add your own initialization code here
    }

    void uninitialize() {
        // add your own uninitialization code here
        Application::uninitialize();
    }

    void reinitialize(Application& self) {
        Application::reinitialize(self);
        // add your own reinitialization code here
    }

    void defineOptions(OptionSet& options) {
        Application::defineOptions(options);

        options.addOption(
            Option("help", "h", "shows command line argument")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<CVLoader>(this, &CVLoader::handleHelp)));

        options.addOption(
            Option("prj", "j", "Project directory")
                .required(true)
                .repeatable(false)
                .argument("Project directory")
                .callback(OptionCallback<CVLoader>(this, &CVLoader::handlePrj)));

        options.addOption(
            Option("planned-flight-lines", "p", "Planned flight line shape file")
                .required(false)
                .repeatable(false)
                .argument("Planned flight line shape file")
                .callback(OptionCallback<CVLoader>(this, &CVLoader::handlePlannedFlightLines)));

        options.addOption(
            Option("flight-lines", "f", "Flight lines shape file")
                .required(false)
                .repeatable(false)
                .argument("Flight lines shape files")
                .callback(OptionCallback<CVLoader>(this, &CVLoader::handleFlightLines)));
    }

    void handleHelp(const std::string& name, const std::string& value) {
        _helpRequested = true;
        displayHelp();
        stopOptionsProcessing();
    }

    void handlePrj(const std::string& name, const std::string& value) {
        // Project dir must exist
        _prjdir.clear();
        Poco::File prjfile(value);
        if ( prjfile.exists() && prjfile.isDirectory() )
            _prjdir = value;
        else {
            logger().error(Poco::format("error: \"%s\" is not a directory", value ));
            _exit_err = Application::EXIT_USAGE;
            stopOptionsProcessing();
        }
    }

    void handlePlannedFlightLines(const std::string& name, const std::string& value) {
        // if have not extension set "shp" extension to verify existence
        Poco::Path pfl_path(value);
        if ( pfl_path.getExtension().empty())
            pfl_path.setExtension(".shp");

        Poco::File pfl;
        pfl = pfl_path;

        if ( pfl.exists() && ( pfl.isFile() || pfl.isLink()) ) {
            // remove extension
            pfl_path.setExtension("");
            _planned_flight_line = pfl_path.toString();
        }
        else {
            logger().error(Poco::format("error: \"%s\" is not a file", value ));
            stopOptionsProcessing();
        }
    }

    void handleFlightLines(const std::string& name, const std::string& value) {
        // if have not extension set "shp" extension to verify existence
        Poco::Path fl_path(value);
        if ( fl_path.getExtension().empty())
            fl_path.setExtension(".shp");

        Poco::File fl;
        fl = fl_path;

        if ( fl.exists() && ( fl.isFile() || fl.isLink()) ) {
            // remove extension
            fl_path.setExtension("");
            _flight_line = pfl_path.toString();
        }
        else {
            logger().error(Poco::format("error: \"%s\" is not a file", value ));
            stopOptionsProcessing();
        }

    }

    void displayHelp() {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("Project loader module");
        helpFormatter.format(std::cout);
    }

    int load_planned_flight_lines() {
        int nrows;
        try {
            Poco::Path dbpath( _prjdir );
            dbpath.makeDirectory( ); // we are sure this is a directory
            dbpath.setFileName( GEO_DB_NAME);
            CV::Util::Spatialite::Connection cnn;
            cnn.create( dbpath.toString() ); // Create or open spatialite db
            if ( cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA )
                cnn.initialize_metdata(); // Initialize metadata (if already initialized noop)

            // load shapefile
            nrows = cnn.load_shapefile(_planned_flight_line,
                               PLANNED_FLIGHT_LAYER_NAME,
                               SHAPE_CHAR_SET,
                               GAUSS_BOAGA_SRID,
                               GEOM_COL_NAME,
                               false,
                               false,
                               false);
        }
        catch(std::exception const& e){
            logger().error(std::string( e.what()) );
            return Application::EXIT_DATAERR;
        }
        logger().error(Poco::format("Loaded %d rows from %s", nrows, _planned_flight_line ) );
        return Application::EXIT_OK;
    }

    int load_flight_lines() {
        int nrows;
        try {
            Poco::Path dbpath( _prjdir );
            dbpath.makeDirectory( ); // we are sure this is a directory
            dbpath.setFileName( GEO_DB_NAME );
            CV::Util::Spatialite::Connection cnn;
            cnn.create( dbpath.toString() ); // Create or open spatialite db
            if ( cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA )
                cnn.initialize_metdata(); // Initialize metadata (if already initialized noop)

            // load shapefile
            nrows = cnn.load_shapefile(_flight_line,
                               PLANNED_FLIGHT_LAYER_NAME,
                               SHAPE_CHAR_SET,
                               GAUSS_BOAGA_SRID,
                               GEOM_COL_NAME,
                               false,
                               false,
                               false);
        }
        catch(std::exception const& e){
            logger().error(std::string( e.what()) );
            return Application::EXIT_DATAERR;
        }
        logger().error(Poco::format("Loaded %d rows from %s", nrows, _flight_line ) );
        return Application::EXIT_OK;
    }

    int main(const std::vector<std::string>& args) {
        if (_exit_err != Application::EXIT_OK )
            return _exit_err;

        if (!_planned_flight_line.empty() ){
            return load_planned_flight_lines();
        }

        if (!_flight_line.empty() ){
            return load_flight_lines();
        }


        return Application::EXIT_OK;
    }

private:
    bool _helpRequested;
    std::string _prjdir; // Project directory
    std::string _planned_flight_line; // planned flight line shape file
    std::string _flight_line; // flight line shape
    int _exit_err;
};


POCO_APP_MAIN(CVLoader)
