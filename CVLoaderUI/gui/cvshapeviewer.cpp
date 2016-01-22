#include "cvshapeviewer.h"

#include "core/sql/querybuilder.h"
#include "gui/cvgui_utils.h"

#include <QProcess>
#include <QTemporaryFile>
#include <QImage>
#include <QGraphicsPixmapItem>

using namespace CV::GUI;

void ShapeViewer::loadFromShp(const QString& path) {
	QString out;
	{
		QTemporaryFile script;
		script.open();
		out = script.fileName();
	}
	bool ok = QFile::copy(":/scripts/convert.py", out);

	CVScopedCursor cur;

	QTemporaryFile output;
	output.open();

	QStringList args;
	args << out << path + ".shp" << output.fileName();
	QProcess::execute("python", args);

	QImage im(output.fileName(), "BMP");
	_item->setPixmap(QPixmap::fromImage(im));
	_item->setTransformationMode(Qt::SmoothTransformation);
	
	_scene->setSceneRect(_item->boundingRect());
	fitInView(_item, Qt::KeepAspectRatioByExpanding);
}

void ShapeViewer::loadFromSpatialite(const QString& layer) {
	QString out;
	{
		QTemporaryFile script;
		script.open();
		out = script.fileName();
	}
	bool ok = QFile::copy(":/scripts/convert.py", out);

	CVScopedCursor cur;

	QTemporaryFile output;
	output.open();

	QStringList args;
	args << out << Core::SQL::Database::path() << output.fileName() << layer;
	QProcess::execute("python", args);

	QImage im(output.fileName(), "BMP");
	_item->setPixmap(QPixmap::fromImage(im));
	_item->setTransformationMode(Qt::SmoothTransformation);
}

void ShapeViewer::showEvent(QShowEvent*) {
	_scene->setSceneRect(_item->boundingRect());
	fitInView(_item, Qt::KeepAspectRatioByExpanding);
}