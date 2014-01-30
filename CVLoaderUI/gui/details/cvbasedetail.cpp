#include "cvbasedetail.h"

#include "gui/helper/cvactionslinker.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPalette>

#include <QMimeData>

#include <QFile>
#include <QUrl>
#include <QPushButton>

#include <QFileDialog>

//#include <QStandardPaths>

namespace CV {
namespace GUI {
namespace Details {

CVBaseDetail::CVBaseDetail(QWidget* p) : QWidget(p) {
    setAcceptDrops(true);

    QHBoxLayout* hLayout = new QHBoxLayout;
    _title = new QLabel(this);
    _title->setMaximumHeight(36);
    _title->setStyleSheet("padding: 4px; font: bold;");

    QPushButton* menuBtn = new QPushButton(tr(""), this);

    menuBtn->setMaximumSize(20, 26);
    _menu = new QMenu(this);
    menuBtn->setMenu(_menu);
    hLayout->addWidget(menuBtn);
    hLayout->addWidget(_title);
    QWidget* header = new QWidget(this);
    header->setLayout(hLayout);

    _descr = new QLabel(this);
    _descr->setIndent(10);
    _descr->setMaximumHeight(36);
    _descr->setStyleSheet("padding: 4px;");

    _body = new QWidget(this);

    QVBoxLayout* box = new QVBoxLayout;
    box->addWidget(header);
    box->addWidget(_descr);
    box->addWidget(_body, 2);
    setLayout(box);
}


} // namespace Details
} // namespace GUI
} // namespace CV
