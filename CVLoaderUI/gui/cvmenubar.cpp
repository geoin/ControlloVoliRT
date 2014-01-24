#include "gui/cvmenubar.h"

namespace CV {
namespace GUI {

CVMenuBar::CVMenuBar(QWidget *parent) : QMenuBar(parent) {
    setMinimumHeight(24);
}

QMenu* CVMenuBar::add(QString menu) {
    QMenu* m = new QMenu(menu, this);
    addMenu(m);
    return m;
}

}
}
