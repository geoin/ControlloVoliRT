#include "cvstatusbar.h"

#include <QProgressBar>
#include <QLabel>
#include <QSpacerItem>

namespace CV {
namespace GUI {

CVStatusBar::CVStatusBar(QWidget *parent) : QStatusBar(parent) {
    //setSizeGripEnabled(true);

    //QProgressBar* progress = new QProgressBar(this);
    //progress->setMinimumWidth(160);
    //addPermanentWidget(progress);

    setContentsMargins(4, 0, 4, 2);
}

} // namespace GUI
} // namespace CV
