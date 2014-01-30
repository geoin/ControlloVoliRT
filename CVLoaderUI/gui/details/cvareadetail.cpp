#include "cvareadetail.h"

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

CVAreaDetail::CVAreaDetail(QWidget* p, Core::CVShapeLayer* l) : CVBaseDetail(p) {
	setAcceptDrops(true);

	title(tr("Aree da cartografare"));
	description(tr("File shape"));

	QMenu* m = detailMenu();
	connect(m->addAction(QIcon(""), tr("Rimuovi")), SIGNAL(triggered()), this, SLOT(clearAll()));

    /*QMenu* menu = new QMenu(this);
    QAction* add = menu->addAction(QIcon(""), "Carica");*/
	
    QFormLayout* form = new QFormLayout;

	QLabel* lab = new QLabel("", this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
	_labels << lab;

	QLabel* n = new QLabel("Record inseriti", this);
	n->setMinimumHeight(26);
	n->setMaximumHeight(26);
	n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);

	form->addRow(n, lab);

	body(form);

	_layer = l;

	if (_layer->isValid()) {
		QStringList info = _layer->data();
		for (int i = 0; i < info.size(); ++i) {
			QLabel* lab = _labels.at(i);
			lab->setText(info.at(i));
		}
	}
}

CVAreaDetail::~CVAreaDetail() {

}

void CVAreaDetail::clearAll() {
	_layer->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		QLabel* lab = _labels.at(i);
		lab->setText("");
	}
}

void CVAreaDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        _uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(_uri));
        if (_file->suffix().toLower() != "shp") {
            _file.reset();
            ev->ignore();
        } else {
            ev->accept();
        }
    }
}

void CVAreaDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVAreaDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVAreaDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	_layer->shape(_file->absolutePath() + QDir::separator() + _file->baseName());
	if (_layer->persist()) {
		QStringList& info = _layer->data();
		for (int i = 0; i < _labels.size(); ++i) {
			QLabel* lab = _labels.at(i);
			lab->setText(info.at(i));
		}
	}
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
