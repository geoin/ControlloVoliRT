#!/usr/bin/python2
# -*- coding: UTF-8 -*-

import sys, os, subprocess, shutil

scriptPath = os.path.dirname(os.path.realpath(sys.argv[0]));
print "\n## Regione Toscana Tools - Plugin QGis ##"
print "#########################################"
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

def setExecPrivilege(target):
    "Set exec privilege by calling chmod +x on target"
    subprocess.call(["chmod", "+x", target])

def setExecPrivilegesToAll(src):
    "Set exec to all files in folder"
    files = os.listdir(src)
    for name in files:
        fullName = os.path.join(src, name)
        fileName, fileExtension = os.path.splitext(fullName)
        if len(fileExtension) == 0 or fileExtension == ".so":
	    setExecPrivilege(fullName)
    
ret = raw_input("\nAggiungere il repository ubuntu-gis e installare qgis?\n[sì (s/S)] [no (n/N)]\n")
if (ret == 's') or (ret == 'S'):
    subprocess.call(["apt-add-repository", "-y", "ppa:ubuntugis/ubuntugis-unstable"])
    subprocess.call(["apt-get", "update"])
    subprocess.call(["apt-get", "-y", "upgrade"])
    subprocess.call(["apt-get", "-y", "install", "qgis"])

ret = raw_input("\nInstallare le dipendenze?\n[sì (s/S)] [no (n/N)]\n")
if (ret == 's') or (ret == 'S'):
    ret = subprocess.call(["apt-get", "-y", "install", "fop", "docbook-xsl-ns", "docbook5-xml", "docbook-xsl", "xsltproc", "libgeotiff2"])
    if ret != 0:
        print "\nNon è stato possibile installare uno o più pacchetti necessari."
	print "Verificare ed eventualmente installare manualmente."
	print "[lista pacchetti]"
	ret = raw_input("Continuare?\n[sì (s/S)] [no (n/N)]\n")
	if (ret != 's') and (ret != 'S'):
	    sys.exit()

processname = 'qgis.bin'

for line in os.popen("ps xa"):
    fields  = line.split()
    pid     = fields[0]
    process = fields[4]

    if process.find(processname) > 0:
	ret = raw_input("\nChiudere QGis prima di effettuare l'installazione\n(Premere un tasto per continuare)\n")
	break

installDir = os.getcwd()
print "\nCartella di lavoro: " + installDir

#Put library in local/lib
libs = installDir + "/lib"
setExecPrivilegesToAll(libs)

dest = "/usr/local/lib"
copyAllFiles(libs, dest)

#Setting env
env = installDir + "/script/rt_env.sh"
#subprocess.call(["chmod", "+x", env])
setExecPrivilege(env)
dest = "/etc/profile.d"
shutil.copy(env, dest)

#Copy plugin binaries
plugins = installDir + "/bin"
setExecPrivilegesToAll(plugins)

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

