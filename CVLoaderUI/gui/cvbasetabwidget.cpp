#include "cvbasetabwidget.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

#include <QTabBar>

namespace CV {
namespace GUI {

CVBaseTabWidget::CVBaseTabWidget(QWidget* p, TabPosition pos) : QTabWidget(p) {
    setAcceptDrops(true);
    setTabPosition(pos);
	setIconSize(QSize(32, 32));
}

void CVBaseTabWidget::dragEnterEvent(QDragEnterEvent* ev) {
    ev->accept();
}

void CVBaseTabWidget::dragMoveEvent(QDragMoveEvent* ev) {
    QPoint position = ev->pos();
    if (tabPosition() == QTabWidget::East) { //maybe a bug
        position.setX(width() - position.x());
    } else if (tabPosition() == QTabWidget::South) {
        position.setY(height() - position.y());
    }
    int index = tabBar()->tabAt(position);
    if (index >= 0) {
        setCurrentIndex(index);
    }
    ev->ignore();
}

} // namespace GUI
} // namespace CV
