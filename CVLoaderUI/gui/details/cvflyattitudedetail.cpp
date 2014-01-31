#include "cvflyattitudedetail.h"

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

CVFlyAttitudeDetail::CVFlyAttitudeDetail(QWidget* p, Core::CVFlyAttitude* l) : CVBaseDetail(p) {
	setAcceptDrops(true);

	title(tr("Assetti di volo"));
	description(tr("File degli assetti"));

	QMenu* m = detailMenu();
	connect(m->addAction(QIcon(""), tr("Rimuovi")), SIGNAL(triggered()), this, SLOT(clearAll()));

    /*QMenu* menu = new QMenu(this);
    QAction* add = menu->addAction(QIcon(""), "Carica");*/
	
    QFormLayout* form = new QFormLayout;

	QLabel* n = NULL;
	QLabel* info = NULL;
	createRow(this, tr("Numero strisciate"), n, info);
	_labels << info;
	form->addRow(n, info);

	
	createRow(this, tr("Numero fotogrammi"), n, info);
	_labels << info;
	form->addRow(n, info);

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

CVFlyAttitudeDetail::~CVFlyAttitudeDetail() {

}

void CVFlyAttitudeDetail::importAll(const QStringList&) {

}

void CVFlyAttitudeDetail::clearAll() {
	_layer->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		QLabel* lab = _labels.at(i);
		lab->setText("");
	}
}

void CVFlyAttitudeDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        QString uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(uri));
		ev->accept();
    }
}

void CVFlyAttitudeDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVFlyAttitudeDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVFlyAttitudeDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	_layer->origin(_file->absoluteFilePath());
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
