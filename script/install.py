#!/usr/bin/python2
# -*- coding: UTF-8 -*-

import sys, os, subprocess, shutil

scriptPath = os.path.dirname(os.path.realpath(sys.argv[0]));
print scriptPath
os.chdir(scriptPath)
os.chdir("..")

def copyAllFiles(src, dest):
    "Copy all files from src to dest"
    files = os.listdir(src)
    count = 0;
    for name in files:
        fullName = os.path.join(src, name)
	if os.path.islink(fullName):
            linkto = os.readlink(fullName)
            destName = os.path.join(dest, name)
	    if not os.path.lexists(destName):
                os.symlink(linkto, destName)
        else:
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

#Put library in local/lib
libs = installDir + "/lib"
dest = "/usr/local/lib"
copyAllFiles(libs, dest)

#Setting env
env = installDir + "/script/rt_env.sh"
subprocess.call(["chmod", "+x", env])
dest = "/etc/profile.d"
shutil.copy(env, dest)

#Copy plugin binaries
plugins = installDir + "/bin"
dest = "/usr/lib/qgis/plugins"
copyAllFiles(plugins, dest)

#Docbook stuff
reportDir = "/opt/docbookrt"
try:
    os.mkdir(reportDir)
except:
    print reportDir + " già presente"
    pass

docb = installDir + "/docbookrt"
copyAllFiles(docb, reportDir)
shutil.copy(installDir + "/script/report.py", reportDir)

#icons
iconsDir = "/usr/lib/qgis/icons"
try:
    os.mkdir(iconsDir)
except:
    print iconsDir + " già presente"
    pass

ico = installDir + "/icons"
copyAllFiles(ico, iconsDir)

