from osgeo import gdal, ogr
import sys

# Define pixel_size and NoData value of new raster
#pixel_size = 10
NoData_value = 255
source_layer = 0

vector_fn = sys.argv[1]
raster_fn = sys.argv[2]

print vector_fn, raster_fn

source_ds = ogr.Open(vector_fn)

if len(sys.argv) <= 3:
	source_layer = source_ds.GetLayer()
else:
	source_layer = source_ds.GetLayer(sys.argv[3])
	
sourceSR = source_layer.GetSpatialRef()

x_min, x_max, y_min, y_max = source_layer.GetExtent()

pixel_size = (x_max - x_min) / 1024

x_res = int((x_max - x_min) / pixel_size)
y_res = int((y_max - y_min) / pixel_size)

target_ds = gdal.GetDriverByName('BMP').Create(raster_fn, x_res, y_res, 3, gdal.GDT_Byte)
target_ds.SetGeoTransform((x_min, pixel_size, 0, y_max, 0, -pixel_size))
band = target_ds.GetRasterBand(1)
band.SetNoDataValue(NoData_value)

# Rasterize
gdal.RasterizeLayer(target_ds, [3, 2, 1], source_layer, burn_values=[255, 255, 255])