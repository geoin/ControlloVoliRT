#!/usr/bin/python2
# -*- coding: UTF-8 -*-

import sys, os, subprocess, shutil

os.chdir(os.path.dirname(sys.path[0]))

def copyAllFiles(src, dest):
        "Copy all files from src to dest"
        files = os.listdir(src)
        count = 0;
        for name in files:
                fullName = os.path.join(src, name)
                if (os.path.isfile(fullName)):
                        shutil.copy(fullName, dest)
                        count += 1
        return count

ret = raw_input("\nAggiungere il repository ubuntu-gis e installare qgis?\n[sì (y/Y)] [no (n/N)]\n")
if (ret == 'y') or (ret == 'Y'):
	subprocess.call(["apt-add-repository", "-y", "ppa:ubuntugis/ubuntugis-unstable"])
	subprocess.call(["apt-get", "update"])
	subprocess.call(["apt-get", "-y", "upgrade"])
	subprocess.call(["apt-get", "-y", "install", "qgis"])

ret = subprocess.call(["apt-get", "-y", "install", "fop", "docbook-xsl-ns", "docbook5-xml", "docbook-xsl", "xsltproc", "libgeotiff2"])
if ret != 0:
        print "\nNon è stato possibile installare uno o più pacchetti necessari."
        print "Verificare ed eventualmente installare manualmente."
        print "[lista pacchetti]"
        ret = raw_input("Continuare?\n[sì (y/Y)] [no (n/N)]\n")
        if (ret != 'y') and (ret != 'Y'):
                sys.exit()

installDir = os.getcwd()
print "\nCartella di lavoro: " + installDir

libs = installDir + "/lib"
dest = "/usr/local/lib"
copyAllFiles(libs, dest)

env = installDir + "/env"
subprocess.call(["chmod", "+x", env + "/rt_env.sh"])
dest = "/etc/profile.d"
copyAllFiles(env, dest)

plugins = installDir + "/plugins"
dest = "/usr/lib/qgis/plugins"
copyAllFiles(plugins, dest)
reportDir = "/opt/docbookrt"
try:
        os.mkdir(reportDir)
except:
        print reportDir + " già presente"
        pass

docb = installDir + "/docbookrt"
copyAllFiles(docb, reportDir)

iconsDir = "/usr/lib/qgis/icons"
try:
        os.mkdir(iconsDir)
except:
        print iconsDir + " già presente"
        pass

ico = installDir + "/icons"
copyAllFiles(ico, iconsDir)





