SET OSGEO4W_ROOT=C:\OSGeo4W64
SET QGISNAME=qgis
SET QGIS=%OSGEO4W_ROOT%\apps\%QGISNAME%
SET QGIS_PREFIX_PATH=%QGIS%

CALL %OSGEO4W_ROOT%\bin\o4w_env.bat

SET PATH=%PATH%;%QGIS%\bin
SET PYTHONPATH=%QGIS%\python;%PYTHONPATH%

start python ../src/__main__.py ../test/5.ini
start python ../src/__main__.py ../test/50.ini
start python ../src/__main__.py ../test/80.ini
start python ../src/__main__.py ../test/100.ini