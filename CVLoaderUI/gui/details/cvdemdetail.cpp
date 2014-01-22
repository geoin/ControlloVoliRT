#include "cvdemdetail.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMimeData>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QUrl>
#include <QDir>

namespace CV {
namespace GUI {
namespace Details {

//TODO: needs cleanup, all details should use the same hooks

CVDemDetail::CVDemDetail(QWidget* p, Core::CVFileInput* l) : QWidget(p) {
	setAcceptDrops(true);

    QHBoxLayout* hLayout = new QHBoxLayout;
    QLabel* h = new QLabel(tr("DEM"), this);
    h->setMaximumHeight(36);
    h->setStyleSheet("padding: 4px; font: bold;");

    QPushButton* menuBtn = new QPushButton(tr(""), this);

    menuBtn->setMaximumSize(20, 26);
    QMenu* menu = new QMenu(this);
    QAction* add = menu->addAction(QIcon(""), "Carica");
    QAction* edit = menu->addAction(QIcon(""), "Modifica");
    menuBtn->setMenu(menu);
    hLayout->addWidget(menuBtn);
    hLayout->addWidget(h);
    QWidget* header = new QWidget(this);
    header->setLayout(hLayout);

	QVBoxLayout* box = new QVBoxLayout;
    box->addWidget(header);
	
    QFormLayout* form = new QFormLayout(this);
    QWidget* body = new QWidget(this);

	QLabel* lab = new QLabel("", this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
	_labels << lab;

	QLabel* n = new QLabel("Ente", this);
	n->setMinimumHeight(26);
	n->setMaximumHeight(26);
	n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);

	form->addRow(n, lab);

	lab = new QLabel("", this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
	_labels << lab;

	n = new QLabel("DT", this);
	n->setMinimumHeight(26);
	n->setMaximumHeight(26);
	n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
	form->addRow(n, lab);
	
	lab = new QLabel("", this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
	_labels << lab;

	n = new QLabel("RID", this);
	n->setMinimumHeight(26);
	n->setMaximumHeight(26);
	n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
	form->addRow(n, lab);

	body->setLayout(form);

    box->addWidget(body, 2);
    setLayout(box);
    
	_file.reset();
	_layer = l;

	if (_layer->isValid()) {
		
	}
}

CVDemDetail::~CVDemDetail() {

}

void CVDemDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        _uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(_uri));
        if (_file->suffix().toLower() != "asc") {
            _file.reset();
            ev->ignore();
        } else {
            ev->accept();
        }
    }
}

void CVDemDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVDemDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVDemDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	if (_layer->persist()) {
		
	}
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
