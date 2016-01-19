#ifndef CV_SHAPE_VIEWER_H
#define CV_SHAPE_VIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QShowEvent>
#include <QGraphicsPixmapItem>

namespace CV {
namespace GUI {

class ShapeViewer : public QGraphicsView {
	Q_OBJECT
public: 
	ShapeViewer(QWidget* p) : QGraphicsView(p) , _scene(new QGraphicsScene(this)), _item(new QGraphicsPixmapItem) {
		 setAcceptDrops(false);

		_scene->setBackgroundBrush(QBrush(Qt::black, Qt::SolidPattern));
		_scene->addItem(_item);

		setScene(_scene);
		setDragMode(ScrollHandDrag);
	}

public slots:
	void loadFromShp(const QString& path);
	void loadFromSpatialite(const QString& layer);

	virtual void showEvent(QShowEvent*);

private:
	QGraphicsScene* _scene;
	QGraphicsPixmapItem *_item;

};

}}

#endif