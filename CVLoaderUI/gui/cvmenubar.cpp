#include "gui/cvmenubar.h"

namespace CV {
namespace GUI {

CVMenuBar::CVMenuBar(QWidget *parent) : QMenuBar(parent) {
    setMinimumHeight(24);
    addMenu(new QMenu(tr("File"), this));
}

}
}
