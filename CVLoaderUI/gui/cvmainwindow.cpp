#include "gui/cvmainwindow.h"
#include "gui/cvappcontainer.h"

#include <QPalette>

namespace CV {
namespace GUI {

CVMainWindow::CVMainWindow(QWidget *parent) : QMainWindow(parent) {
    setMinimumSize(QSize(800, 600));

    CVAppContainer* app = new CVAppContainer(this);
    addToolBar(app->toolbar());
    setMenuBar(app->menu());
    setStatusBar(app->statusbar());
    setCentralWidget(app);

    //setStyleSheet("QToolTip { color: #111; background-repeat:no-repeat; background-position:center; background-color: #eee; padding: 2px;}");
    app->link();
}

CVMainWindow::~CVMainWindow() {

}

}
}
