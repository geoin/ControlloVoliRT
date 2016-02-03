from osgeo import ogr, osr

import os

driver = ogr.GetDriverByName('ESRI Shapefile')


def NewWithEPSG(path, name, type, epsg):
    try:
        srs = osr.SpatialReference()
        srs.ImportFromEPSG(epsg)

        shp = Shape(path, name, type, srs)
        if shp != None:
            shp.Create()
        return shp
    except Exception, err:
        print err
        return None


def New(path, name, type=None, srs=None):
    try:
        shp = Shape(path, name, type, srs)
        if shp != None:
            shp.Create()
        return shp
    except Exception, err:
        print  err
        return None


def Open(path, name):
    try:
        shp = Shape(path, name, None, None)
        if shp != None:
            shp.Open()
        return shp
    except Exception, err:
        print err
        return None


def Delete(path, name):
    try:
        shp = Shape(path, name, None, None)
        shp.Delete()
    except Exception, err:
        print err
        return None


class Shape:
    def __init__(self, path, name, type, srs):
        self.path = path
        self.name = name
        self.type = type
        self.srs = srs

    def Create(self):
        """create a new shapefile"""
        if self.OpenIfExists():
            return

        self.shapefile = driver.CreateDataSource(self.path)
        self.layer = self.shapefile.CreateLayer(self.name, self.srs, self.type)

    def Open(self):
        """open a new shapefile"""
        complete = os.path.join(self.path, self.name + ".shp")
        self.shapefile = driver.Open(complete, 1)
        self.layer = self.shapefile.GetLayer(0)
        self.type = self.layer.GetLayerDefn().GetGeomType()
        self.srs = self.layer.GetSpatialRef()

    def Clone(self, path, name, epsg=None):
        """clone the input shapefile structure and return it"""
        Delete(path, name)

        srs = self.srs
        if srs == None and epsg != None:
            srs = osr.SpatialReference()
            srs.ImportFromEPSG(epsg)

        out = New(path, name, self.type, srs)
        feature = self.layer.GetFeature(0)
        [out.layer.CreateField(feature.GetFieldDefnRef(i)) for i in range(feature.GetFieldCount())]
        return out

    def Delete(self):
        """delete a shapefile if present"""
        complete = os.path.join(self.path, self.name + ".shp")
        if os.path.exists(complete):
            ret = driver.DeleteDataSource(complete)
            return ret

    def Release(self):
        """release data"""
        self.shapefile.Destroy()


    def OpenIfExists(self):
        """open a shapefile if present"""
        complete = os.path.join(self.path, self.name + ".shp")
        if os.path.exists(complete):
            self.Open()
            return True
        return False

    def NewFeatureFromGeom(self, geom):
        """add a geometry as feature"""
        feature = ogr.Feature(self.layer.GetLayerDefn())
        feature.SetGeometry(geom)
        self.layer.CreateFeature(feature)
        return feature

    def AddFeature(self, feature):
        """add a feature"""
        self.layer.CreateFeature(feature)
        return feature

    def Count(self):
        return self.layer.GetFeatureCount()

    def Feature(self, i):
        return self.layer.GetFeature(i)

    def Geometry(self, i):
        if i >= self.Count():
            return None

        feat = self.layer.GetFeature(i)
        geom = feat.GetGeometryRef()
        if geom == None:
            return geom
        else:
            return geom.Clone()

    def Area(self):
        area = ogr.Geometry(ogr.wkbMultiPolygon)
        for i in range(self.Count()):
            geom = self.Geometry(i)
            if geom != None:
                area = area.Union(geom)
        return area

    def Geometries(self):
        geometries = []
        for i in range(self.Count()):
            geom = self.Geometry(i)
            geometries.append(geom)
        return geometries

    def EachFeature(self, fun):
        for i in range(self.Count()):
            feature = self.Feature(i)
            fun(i, feature)
