#include <Poco/Logger.h>
#include "CVUtil/cvspatialite.h"
#include <iostream>

#define PLANNED_FLIGHT_LAYER_NAME "AVOLOP"
#define SHAPE_CHAR_SET "CP1252"
#define GAUSS_BOAGA_SRID  32632
#define GEOM_COL_NAME "geom"

int main(int argc, char *argv[]) {
    try {
        CV::Util::Spatialite::Connection cnn;
        cnn.create("/Users/andrea/ControlloVoliRT/CVUtil/test/geo.sqlite");
        std::cout << "DB open" << std::endl;


        CV::Util::Spatialite::Statement stmt( cnn );
        stmt.execute("select name from sqlite_master" );

        cnn.initialize_metdata();
        std::cout << "DB spatial init" << std::endl;

        int nrows = cnn.load_shapefile("/Users/andrea/ControlloVoliRT/CVUtil/test/Castiglione_Scarlino",
                           PLANNED_FLIGHT_LAYER_NAME,
                           SHAPE_CHAR_SET,
                           GAUSS_BOAGA_SRID,
                           GEOM_COL_NAME,
                           false,
                           false,
                           false);
        std::cout << "Loaded shape" << std::endl;

        bool ret = cnn.check_metadata();
        std::cout << "metatdata " << ret << std::endl;
    }
    catch(std::exception &e){
        std::cout << std::string(e.what()) << std::endl;
    }
}
