from osgeo import ogr

import os

driver = ogr.GetDriverByName('ESRI Shapefile')


class Shape:
    @staticmethod
    def New(path, name, type):
        try:
            shp = Shape(path, name, type)
            shp.create()
            return shp
        except:
            return None

    @staticmethod
    def Open(path, name):
        try:
            shp = Shape(path, name, None)
            shp.open()
            return shp
        except:
            return None

    def __init__(self, path, name, type):
        self.path = path
        self.name = name
        self.type = type

    def create(self):
        """create a new shapefile"""
        self.delete()

        self.shapefile = driver.CreateDataSource(self.path)
        self.layer = self.shapefile.CreateLayer(self.name, geom_type=self.type)

    def open(self):
        """open a new shapefile"""
        complete = os.path.join(self.path, self.name + ".shp")
        self.shapefile = driver.Open(complete, 1)
        self.layer = self.shapefile.GetLayer(0)
        self.type = self.layer.GetLayerDefn().GetGeomType()

    def delete(self):
        """delete a shapefile if present"""
        complete = os.path.join(self.path, self.name + ".shp")
        if os.path.exists(complete):
            driver.DeleteDataSource(complete)

    def addFeature(self, geom):
        """add a geometry as feature"""
        feature = ogr.Feature(self.layer.GetLayerDefn())
        feature.SetGeometry(geom)
        self.layer.CreateFeature(feature)
        return feature

    def Count(self):
        return self.layer.GetFeatureCount()

    def Feature(self, i):
        return self.layer.GetFeature(i)

    def Geometry(self, i):
        return self.layer.GetFeature(i).GetGeometryRef()

    def EachFeature(self, fun):
        for i in range(self.Count()):
            feature = self.Feature(i)
            fun(i, feature)
