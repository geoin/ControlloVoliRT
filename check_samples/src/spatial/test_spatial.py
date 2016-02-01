from unittest import TestCase
from osgeo import gdal, ogr

import unittest
import shp
import random

testPath = "../../test/data"


def handleFeature(i, feat):
    print i, feat.GetGeometryRef()
    return i


class TestShape(TestCase):
    def test_New(self):
        shp.Delete("../../test/data", "sampleLayer.shp")
        layer = shp.NewWithEPSG("../../test/data", "sampleLayer", ogr.wkbPolygon, 4326)
        self.assertIsNotNone(layer, "")
        self.assertEqual(layer.name, "sampleLayer")

        layer.NewFeatureFromGeom(ogr.Geometry(type=ogr.wkbPolygon))
        layer.NewFeatureFromGeom(ogr.Geometry(type=ogr.wkbPolygon))
        layer.NewFeatureFromGeom(ogr.Geometry(type=ogr.wkbPolygon))

        indexes = []
        layer.EachFeature(lambda i, feat: indexes.append(handleFeature(i, feat)))
        print indexes

    def test_Open(self):
        layer = shp.New("../../test/data", "sampleLayer", ogr.wkbPolygon, 4326)
        self.assertIsNotNone(layer, "")
        self.assertEqual(layer.name, "sampleLayer")

        layer.NewFeatureFromGeom(ogr.Geometry(type=ogr.wkbPolygon))
        layer.NewFeatureFromGeom(ogr.Geometry(type=ogr.wkbPolygon))
        layer.NewFeatureFromGeom(ogr.Geometry(type=ogr.wkbPolygon))

        indexes = []
        layer.EachFeature(lambda i, feat: indexes.append(handleFeature(i, feat)))
        print indexes

    def test_Clone(self):
        layer = shp.Open("../../test/data", "zona2castpescaia")
        other = layer.Clone("../../test/data", "clonedLayer")
        layer.EachFeature(lambda i, feat: other.AddFeature(feat))


    def test_Union(self):
        layer = shp.Open("../../test/data", "zona2castpescaia")
        print layer.Area()


if __name__ == '__main__':
    unittest.main()
