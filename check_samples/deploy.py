"""deploy script"""

import os
import sys
import zipfile
import shutil

os.chdir(os.path.dirname(os.path.realpath(sys.argv[0])))
os.chdir("src")

with zipfile.ZipFile('sampler.pyz', 'w') as zip_:
    zip_.write("__main__.py")
    zip_.write("config.py")
    zip_.write("spatial/shp.py")
    zip_.write("spatial/__init__.py")

shutil.copyfile("sampler.pyz", "../../bin64/sampler.pyz")
