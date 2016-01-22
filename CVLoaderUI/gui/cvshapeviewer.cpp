#include "cvshapeviewer.h"

#include "core/sql/querybuilder.h"
#include "gui/cvgui_utils.h"

#include <QProcess>
#include <QTemporaryFile>
#include <QImage>
#include <QGraphicsPixmapItem>

using namespace CV::GUI;

void ShapeViewer::loadFromShp(const QString& path) {
	CVScopedCursor cur;

	QTemporaryFile output;
	output.open();

	QStringList args;
	args << "convert.py" << path + ".shp" << output.fileName();
	QProcess::execute("python", args);

	QImage im(output.fileName(), "BMP");
	_item->setPixmap(QPixmap::fromImage(im));
	_item->setTransformationMode(Qt::SmoothTransformation);
	
	_scene->setSceneRect(_item->boundingRect());
	fitInView(_item, Qt::KeepAspectRatioByExpanding);
}

void ShapeViewer::loadFromSpatialite(const QString& layer) {
	CVScopedCursor cur;

	QTemporaryFile output;
	output.open();

	QStringList args;
	args << "convert.py" << Core::SQL::Database::path() << output.fileName() << layer;
	QProcess::execute("python", args);

	QImage im(output.fileName(), "BMP");
	_item->setPixmap(QPixmap::fromImage(im));
	_item->setTransformationMode(Qt::SmoothTransformation);
}

void ShapeViewer::showEvent(QShowEvent*) {
	_scene->setSceneRect(_item->boundingRect());
	fitInView(_item, Qt::KeepAspectRatioByExpanding);
}