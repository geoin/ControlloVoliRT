import ConfigParser, glob, os


class Data:
    def __init__(self, path):
        config = ConfigParser.ConfigParser()
        # remove lowercase convrtion
        # config.optionxform = str
        config.read(path)
        self.input = config.get("DATA", "IN")
        self.output = config.get("DATA", "OUT")
        self.zone = os.path.abspath(config.get("DATA", "ZONE"))
        self.items = config.items("CODE")

        try:
            os.mkdir(self.output)
        except:
            return

    def Shapes(self):
        """map of dir and filename"""
        files = glob.glob(os.path.join(self.input, "*.shp"))
        if self.zone in files:
            files.remove(self.zone)

        data = [os.path.split(f) for f in files]
        return map(lambda x: (x[0], os.path.splitext(os.path.basename(x[1]))[0]), data)

    def Samples(self):
        """list shapes with related percentage configuration"""
        return [(f[0], [0 if not x else int(x.replace("%", "")) for x in f[1].split(",")]) for f in self.items]

    def Zone(self):
        """zobe shape dir and filename"""
        if os.path.isabs(self.zone) == False:
            return None

        splitted = os.path.split(self.zone)
        return splitted[0], os.path.splitext(os.path.basename(splitted[1]))[0]
