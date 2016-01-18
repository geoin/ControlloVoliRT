#ifndef CV_SHAPE_VIEWER_H
#define CV_SHAPE_VIEWER_H

#include <QWidget>

namespace CV {
namespace GUI {

class ShapeViewer : public QWidget {
	Q_OBJECT
public: 
	ShapeViewer(QWidget* p) : QWidget(p) {}

public slots:
	void loadShape(const QString& path);

};

}}

#endif