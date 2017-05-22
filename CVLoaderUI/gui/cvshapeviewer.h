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
        ShapeViewer(QWidget* p);
signals:

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
