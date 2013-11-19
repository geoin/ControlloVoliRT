#include <Poco/Logger.h>
#include "CVUtil/cvspatialite.h"
#include <iostream>
#include "ogr_geometry.h"

#define PLANNED_FLIGHT_LAYER_NAME "AVOLOP"
#define SHAPE_CHAR_SET "CP1252"
#define GAUSS_BOAGA_SRID  32632
#define GEOM_COL_NAME "geom"

#if defined(__APPLE__)
#define TEST_GEO_DB "/Users/andrea/ControlloVoliRT/CVUtil/test/geo.sqlite"
#define  TEST_SHAPE_FILE "/Users/andrea/ControlloVoliRT/CVUtil/test/avolov"
#else
#define TEST_GEO_DB "C:/ControlloVoliRT/CVUtil/test/geo.sqlite"
#define  TEST_SHAPE_FILE "C:/Google_drive/Regione Toscana Tools/Dati_test/assi volo/avolov"
#endif

// Load a shape file
void load_shape() {
    try {
        CV::Util::Spatialite::Connection cnn;
        cnn.create(TEST_GEO_DB);
        std::cout << "DB open" << std::endl;

        if ( cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA )
            cnn.initialize_metdata();
        std::cout << "DB spatial initialized " << std::endl;

        int nrows = cnn.load_shapefile(TEST_SHAPE_FILE,
                           PLANNED_FLIGHT_LAYER_NAME,
                           SHAPE_CHAR_SET,
                           GAUSS_BOAGA_SRID,
                           GEOM_COL_NAME,
                           false,
                           false,
                           false);
        std::cout << "Shape file loaded" << std::endl;
    }
    catch( std::exception &e){
        std::cout << std::string(e.what()) << std::endl;
    }
}

void dump_geometry(std::vector<unsigned char> &wkb){
    OGRGeometryFactory gf;
    OGRGeometry *gg;
    int ret = gf.createFromWkb( (unsigned char *)&wkb[0], NULL, &gg);
    if (ret != OGRERR_NONE ){
        std::cout << "Invalid geometry";
        return;
    }

    char *txt=NULL;
    gg->exportToWkt(&txt);
    std::cout << std::string(txt) ;

    // free allocated memory
    OGRFree(txt);
    OGRGeometryFactory::destroyGeometry(gg);
}

//dump all linear geometry of a shape file
void dump_shape(){
    CV::Util::Spatialite::Connection cnn;
    cnn.open(TEST_GEO_DB);
    std::cout << "DB open" << std::endl;

    if ( cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA ){
        std::cout << "DB with no spatial metadata" << std::endl;
        return;
    }
    std::cout << "DB is spatial initialized " << std::endl;

    CV::Util::Spatialite::Statement stmt(cnn, "select id, AsBinary(geom) from AVOLOP");
    CV::Util::Spatialite::Recordset rs = stmt.recordset();
    while( !rs.eof() ){

        int id = rs[0].toInt();
        std::cout << id <<  ": --> " ;

        std::vector<unsigned char> gg;
        rs[1].toBlob(gg);
        dump_geometry(gg);
        std::cout << std::endl;

        rs.next();
    }

}


void line2end_points(std::vector<unsigned char> &wkb, OGRPoint *firstlast){
    OGRGeometryFactory gf;
    OGRGeometry *gg;
    int ret = gf.createFromWkb( (unsigned char *)&wkb[0], NULL, &gg);
    if (ret != OGRERR_NONE ){
        std::cout << "Invalid geometry";
        return;
    }

    if (gg->getGeometryType() != wkbLineString ){
        OGRGeometryFactory::destroyGeometry(gg);
        std::cout << "Geometry must be a LINESTRING" << std::endl;
        return;
    }

    OGRLineString *ls = (OGRLineString *)gg;
    ls->StartPoint(firstlast);
    ls->EndPoint(firstlast+1);


    //free allocated memory
    OGRGeometryFactory::destroyGeometry(gg);
}

// read a linear shapefile and create a vector point layer with the end points of line features
void layer_line2point(){
    CV::Util::Spatialite::Connection cnn;
    cnn.open(TEST_GEO_DB);
    std::cout << "DB open" << std::endl;

    if ( cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA ){
        std::cout << "DB with no spatial metadata" << std::endl;
        return;
    }
    std::cout << "DB is spatial initialized " << std::endl;

    try {
        cnn.remove_layer("AVOLOP_PT");
    }
    catch(...){}

    //create output layer
    cnn.begin_transaction();
    cnn.execute_immediate("Create table AVOLOP_PT ( PK_UID INTEGER PRIMARY KEY AUTOINCREMENT, id double )");
    cnn.execute_immediate("select AddGeometryColumn('AVOLOP_PT', 'geom', 32632, 'POINT' )");
    cnn.commit_transaction();

    CV::Util::Spatialite::Statement stmt(cnn, "select ID, HEIGHT, AsBinary(geom) from AVOLOP");
    CV::Util::Spatialite::Recordset rs = stmt.recordset();

    CV::Util::Spatialite::Statement stmt_out( cnn, "insert into AVOLOP_PT (id, geom) values (:id, ST_GeomFromWKB(:geom, 32632) )");
    //CV::Util::Spatialite::Statement stmt_out( cnn, "insert into AVOLOP_PT (id, geom) values (:id, ST_GeomFromTEXT('POINTZ(648322.57782173 4736657.061840324 2500.2)', 32632) )");
    while( !rs.eof() ){

        int id = rs[0].toInt();
        double height = rs[1].toDouble();

        std::vector<unsigned char> gg;
        rs[2].toBlob(gg);

        OGRPoint firstlast[2];
        //return first and last point of the line string
        line2end_points(gg, firstlast);
//        firstlast[0].setZ(height);
//        firstlast[1].setZ(height);

        for( int i=0; i<2; i++){
            std::vector<unsigned char> buffin;
            char *buffout=NULL;
            int size_out = 0, size_in = firstlast[i].WkbSize();
            buffin.resize(size_in);
            firstlast[i].exportToWkb(wkbNDR, &buffin[0]);
            firstlast[i].exportToWkt(&buffout);
            std::cout << std::string(buffout) << std::endl;
            OGRFree(buffout);

            /*
            gaiaGeomCollPtr geo = gaiaFromWkb(&buffin[0], size_in);
            geo->Srid = 32632;
            geo->DimensionModel  = GAIA_XY_Z;
            geo->DeclaredType = GAIA_POINTZ;

            // debug in wkt
            gaiaOutBuffer wkt;
            gaiaOutBufferInitialize(&wkt);
            gaiaToEWKT(&wkt, geo);
            std::cout << std::string(wkt.Buffer, wkt.BufferSize) << std::endl;
            gaiaOutBufferReset(&wkt);

            gaiaToSpatiaLiteBlobWkb(geo, &buffout, &size_out);
            std::vector<unsigned char> vtmp;
            vtmp.resize(size_out);
            memcpy(&vtmp[0], buffout, size_out);
            gaiaFree(buffout);
            gaiaFreeGeomColl(geo);
            */

            stmt_out[1] = id;
            stmt_out[2].fromBlob(buffin);
            stmt_out.execute();
            stmt_out.reset();
        }

        rs.next();
    }


}

int main( int argc, char *argv[]) {
try {
    //load_shape();

    // get geometry form db
    //dump_shape();

    // read linear layer, split line in points e copy points in another layer
    layer_line2point();
}
catch(std::runtime_error const &e){
    std::cout << std::string(e.what()) <<  std::endl;
    std::cout.flush();
}

}
