#!/usr/bin/python2
# -*- coding: UTF-8 -*-

import os, sys, subprocess

args = sys.argv
if len(args) < 2:
    print "Error, missing parameters."
    sys.exit()

print args

outFile = args[1]
path = os.path.dirname(os.path.realpath(outFile))
#outFile = args[2]
xslPath = "/usr/share/xml/docbook/stylesheet/docbook-xsl-ns/fo/docbook.xsl"

tmp = path + "/tmp.fo"
f = open(tmp, "w")
subprocess.call(["xsltproc", xslPath, outFile + ".xml"], stdout=f)
f.close()
subprocess.call(["fop", tmp, outFile + ".pdf"])
os.remove(tmp)
