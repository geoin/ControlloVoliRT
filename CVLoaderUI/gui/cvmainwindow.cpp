#include "cvmainwindow.h"
#include "cvappcontainer.h"

#include "core/cvcore_utils.h"

#include <QPalette>

namespace CV {
namespace GUI {

CVMainWindow::CVMainWindow(QWidget *parent) : QMainWindow(parent) {
	int w = Core::CVSettings::get("/app/width", 640).toInt();
	int h = Core::CVSettings::get("/app/height", 480).toInt();
    setMinimumSize(QSize(640, 480));
	resize(w, h);

    CVAppContainer* app = new CVAppContainer(this);
    addToolBar(app->toolbar());
    setMenuBar(app->menu());
    setStatusBar(app->statusbar());
    setCentralWidget(app);

    //setStyleSheet("QToolTip { color: #111; background-repeat:no-repeat; background-position:center; background-color: #eee; padding: 2px;}");
    app->link();
}

CVMainWindow::~CVMainWindow() {
	Core::CVSettings::set("/app/width", width());
	Core::CVSettings::set("/app/height", height());
}

}
}
