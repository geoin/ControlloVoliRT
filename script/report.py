#!/usr/bin/python2
# -*- coding: iso-8859-15 -*-

import os, sys, subprocess

args = sys.argv
if len(args) < 3:
	print "Error, missing parameters."
	sys.exit()

print args

projPath = args[1]
outFile = args[2]
xslPath = "/usr/share/xml/docbook/stylesheet/docbook-xsl-ns/fo/docbook.xsl"

os.chdir(projPath)

tmp = "tmp.fo"
f = open(tmp, "w")
subprocess.call(["xsltproc", xslPath, outFile + ".xml"], stdout=f)
f.close()
subprocess.call(["fop", tmp, outFile + ".pdf"])
os.remove(tmp)
