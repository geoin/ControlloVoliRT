#include "gui/cvmainwindow.h"
#include "gui/cvmenubar.h"
#include "gui/cvtoolbar.h"
#include "gui/cvappcontainer.h"

namespace CV {
namespace GUI {

CVMainWindow::CVMainWindow(QWidget *parent) : QMainWindow(parent) {
    setMinimumSize(QSize(500, 400));

    addToolBar(new CVToolBar(this));
    setMenuBar(new CVMenuBar(this));
    setCentralWidget(new CVAppContainer(this));
}

CVMainWindow::~CVMainWindow() {

}

}
}
