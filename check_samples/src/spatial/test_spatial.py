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
        layer = shp.Shape.New("../../test/data", "sampleLayer", ogr.wkbPolygon)
        self.assertIsNotNone(layer, "")
        self.assertEqual(layer.name, "sampleLayer")

        layer.addFeature(ogr.Geometry(type=ogr.wkbPolygon))
        layer.addFeature(ogr.Geometry(type=ogr.wkbPolygon))
        layer.addFeature(ogr.Geometry(type=ogr.wkbPolygon))

        indexes = []
        layer.EachFeature(lambda i, feat : indexes.append(handleFeature(i, feat)))
        print indexes

    def test_Open(self):
        layer = shp.Shape.Open("../../test/data", "sampleLayer")
        self.assertIsNotNone(layer, "")
        self.assertEqual(layer.name, "sampleLayer")

        layer.addFeature(ogr.Geometry(type=ogr.wkbPolygon))
        layer.addFeature(ogr.Geometry(type=ogr.wkbPolygon))
        layer.addFeature(ogr.Geometry(type=ogr.wkbPolygon))

        indexes = []
        layer.EachFeature(lambda i, feat : indexes.append(handleFeature(i, feat)))
        print indexes



if __name__ == '__main__':
    unittest.main()
