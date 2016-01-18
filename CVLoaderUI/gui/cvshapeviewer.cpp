#include "cvshapeviewer.h"

#include <QProcess>

using namespace CV::GUI;

void ShapeViewer::loadShape(const QString& path) {
	QStringList args;
	args << "-burn 0" << "-burn 255" << "-burn 0" << "-of BMP" << "-ot Byte";
		
	QProcess::execute("gdal_rasterize");
}