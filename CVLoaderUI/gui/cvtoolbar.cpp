#include "gui/cvtoolbar.h"

namespace CV {
namespace GUI {

CVToolBar::CVToolBar(QWidget *parent) : QToolBar(parent) {
    setMovable(false);
    setFloatable(false);
    setMinimumHeight(28);
}

}
}
