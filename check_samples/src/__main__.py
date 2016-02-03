import os, sys

import random

from spatial import shp
import config

os.chdir(os.path.dirname(os.path.realpath(sys.argv[0])))
root = os.getcwd()


def main(args):
    if len(args) < 2:
        print "Nessuna configurazione trovata. [python", args[0], "path/conf.ini]"
        return -1

    conf = config.Data(args[1])

    epsg = None
    if len(args) >= 3:
        epsg = int(args[2])

    zone = shp.Open(*conf.Zone())
    if zone == None:
        print "Empty zone"
        exit()

    areas = zone.Geometries()

    out = {}
    for dir, filename in conf.Shapes():
        cloned = None
        layer = None
        try:
            name = filename.lower()
            out[name] = {}

            layer = shp.Open(dir, filename)
            if layer == None: continue

            for i in range(layer.Count()):
                geom = layer.Geometry(i)
                if geom == None: continue

                for j in range(len(areas)):
                    g = areas[j]
                    if g.Intersect(geom):
                        if j not in out[name]: out[name][j] = []
                        out[name][j].append(i)
                        break

            data = None
            for key, outInfo in conf.Samples():
                if key == name:
                    data = outInfo
                    break

            if not data: continue

            cloned = layer.Clone(conf.output, filename, epsg)
            featList = out[name]
            featLen = len(featList)
            if featLen == 0:
                continue

            count = int(featLen * data[0] / 100.0)

            if data[1]:
                count = max(count, data[1])
            if data[2]:
                count = min(count, data[2])

            for i, l in featList.iteritems():
                total = len(l)
                sample = [l[i] for i in sorted(random.sample(xrange(total), min(total, count)))]
                [cloned.AddFeature(layer.Feature(i)) for i in sample]

            print "Layer:"+os.path.join(conf.output, filename)
        finally:
            if layer != None:
                layer.Release()
            if cloned != None: cloned.Release()

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
