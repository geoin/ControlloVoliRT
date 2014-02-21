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

f = open("tmp.fo", "w")
subprocess.call(["xsltproc", xslPath, outFile + ".xml"], stdout=f)
f.close()
subprocess.call(["fop", "tmp.fo", outFile + ".pdf"])
