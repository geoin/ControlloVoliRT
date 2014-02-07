#include "cvstatusbar.h"

#include <QProgressBar>
#include <QLabel>
#include <QSpacerItem>

namespace CV {
namespace GUI {

CVStatusBar::CVStatusBar(QWidget *parent) : QStatusBar(parent) {
    setSizeGripEnabled(false);

    addPermanentWidget(new QWidget(this), 1);

    //QProgressBar* progress = new QProgressBar(this);
    //progress->setMinimumWidth(160);
    //addPermanentWidget(progress);

    setContentsMargins(4, 0, 4, 2);
	setStyleSheet("QStatusBar { background-color: rgba(236, 29, 37, 85%) }");
}

} // namespace GUI
} // namespace CV
