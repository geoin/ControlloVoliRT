import os, sys, ConfigParser

from spatial import shp

os.chdir(os.path.dirname(os.path.realpath(sys.argv[0])))
root = os.getcwd()

print "running in ", root

config = ConfigParser.ConfigParser()
config.read('../test/config.ini')
print config.get("DATA", "IN")
print config.get("DATA", "OUT")
