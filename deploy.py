#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import sys, os, subprocess, zipfile

stat = dict()

make = ""
platform = sys.platform
if platform == "linux2":
    make = "make"
else:
    make = "nmake"

def buildList(l):
    for mod in l:
        os.chdir(mod)
        subprocess.call(["qmake", "-config", "release"])
        #subprocess.call(["make", "clean"])
        ret = subprocess.call([make])
        os.chdir("..")

        stat[mod] = "OK" if ret == 0 else "KO"
    return

def addToZip(path, zip):
    for root, dirs, files in os.walk(path):
        for file in files:
	    absFile = os.path.join(root, file)
            if os.path.islink(absFile): #symlinks in zip
            	linkto = os.readlink(absFile)
                a = zipfile.ZipInfo(absFile)
                a.filename = absFile
                a.create_system = 3
                a.external_attr = 2716663808L
                zip.writestr(a, linkto)
            else:
                zip.write(absFile)
        for dir in dirs:
            addToZip(dir, zip)
            
os.chdir(os.path.dirname(os.path.realpath(sys.argv[0])))
root = os.getcwd()
print "Git root: " + root

modules = ["CVUtil", "photo_util", "rtklib", "ziplib", "dem_interpolate"]
buildList(modules)

binaries = ["CVLoaderUI", "check_gps", "check_photo_fli", "check_ta", "check_ortho", "RT-qgis_plugin", "check_lidar_fli"]
buildList(binaries)

print stat

with zipfile.ZipFile('deploy.zip', 'w') as testzip:
    addToZip("lib", testzip)
    addToZip("bin", testzip)
    addToZip("icons", testzip)
    addToZip("script", testzip)
    addToZip("docbookrt", testzip)

#with zipfile.ZipFile('deploy.zip', 'r') as testzip:
#    testzip.extractall("deploy")
