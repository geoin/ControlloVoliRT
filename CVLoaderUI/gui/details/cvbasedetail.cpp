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
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

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
	
    _file.reset(NULL);

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

void CVBaseDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        _uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(_uri));
        if (_file->suffix().toLower() != "xml") {
            _file.reset(NULL);
            ev->ignore();
        } else {
            ev->accept();
        }
    }
}

void CVBaseDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVBaseDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVBaseDetail::dropEvent(QDropEvent* ev) {
    ev->accept();

    _file.reset(NULL);
}


} // namespace Details
} // namespace GUI
} // namespace CV
