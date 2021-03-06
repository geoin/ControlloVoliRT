#include "cvuniondetail.h"

#include "core/cvcore_utils.h"

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

CVUnionDetail::CVUnionDetail(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	setAcceptDrops(true);

	title(tr("Quadro di unione"));
	description(tr("File shape"));

	_shape = new ShapeViewer(this);
	
	QVBoxLayout* box = new QVBoxLayout;
    QFormLayout* form = new QFormLayout;

	QLabel* lab = new QLabel("", this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
	_labels << lab;

	QLabel* n = new QLabel("", this);
	n->setMinimumHeight(26);
	n->setMaximumHeight(26);
	n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);

	form->addRow(n, lab);
	box->addLayout(form);
	box->addWidget(_shape, 2);

	body(box);

	if (controller()->isValid()) {
		lab->setText(QString::number(layer()->rows()));

		_shape->loadFromSpatialite(layer()->table());
	} else {
		_shape->hide();
	}
}

CVUnionDetail::~CVUnionDetail() {

}

void CVUnionDetail::clearAll() {
	controller()->remove();
	_shape->hide();
	for (int i = 0; i < _labels.size(); ++i) {
		QLabel* lab = _labels.at(i);
		lab->setText("");
	}
}

void CVUnionDetail::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa quadro di unione"),
		Core::CVSettings::get(CV_PATH_SEARCH).toString(),
        "(*.shp)"
    );
	if (!uri.isEmpty()) {
		QFileInfo shp(uri);
		Core::CVSettings::set(CV_PATH_SEARCH, shp.absolutePath());
		importAll(QStringList() << shp.absolutePath() + QDir::separator() + shp.baseName());
	}
}

void CVUnionDetail::importAll(QStringList& uri) {
	layer()->shape(uri.at(0));
	if (controller()->persist()) {
		info();

		_shape->show();
		_shape->loadFromShp(uri.at(0));
	}
	_labels.at(0)->setText("Fogli inseriti" + QString::number(layer()->rows()));
}

void CVUnionDetail::dragEnterEvent(QDragEnterEvent* ev) {
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

void CVUnionDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVUnionDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVUnionDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	importAll(QStringList() << _file->absolutePath() + QDir::separator() + _file->baseName());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
