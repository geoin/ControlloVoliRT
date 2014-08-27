#include <gtest/gtest.h>

#include <cstdio>

#include <CVUtil\cvspatialite.h>
#include <CVUtil\ogrgeomptr.h>

const std::string path = "../data/";

TEST (UtilTest, Database) {
	std::string file(path + "db.sqlite");
	{

	CV::Util::Spatialite::Connection cnn;
	cnn.create(file.c_str());
	cnn.initialize_metdata();

	ASSERT_TRUE(cnn.is_valid());
	
	cnn.begin_transaction();
	cnn.execute_immediate("CREATE TABLE IF NOT EXISTS TEST ( ID TEXT, VAL INTEGER )");
	cnn.execute_immediate("INSERT INTO TEST VALUES ('test_id', 2)");
    cnn.execute_immediate("select AddGeometryColumn('TEST', 'geom', 32632, 'POINT' )");
    cnn.commit_transaction();

	
	cnn.execute_immediate("DROP TABLE IF EXISTS TEST");

	}

	remove(file.c_str());
}

TEST (UtilTest, OGR) {
    OGRGeometry *gg;
	std::string wkt("POINT (10.0 20.0)");
	const char * geom = wkt.c_str();
	char * geomNC = const_cast<char*>(geom);
	OGRGeometryFactory::createFromWkt(&geomNC, NULL, &gg);

	
	CV::Util::Geometry::OGRGeomPtr ptr(gg);
	ASSERT_TRUE(ptr->IsValid());

	CV::Util::Geometry::OGRGeomPtr buf(ptr->Buffer(0.1));
	ASSERT_TRUE(buf->IsValid());

	CV::Util::Geometry::OGRGeomPtr buf2(ptr->Buffer(0.3));
	ASSERT_TRUE(buf2->IsValid());
	
	CV::Util::Geometry::OGRGeomPtr un(buf->Union(buf2));

	ASSERT_TRUE(un->IsValid());
}