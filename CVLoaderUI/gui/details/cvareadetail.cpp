#include "cvareadetail.h"

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

CVAreaDetail::CVAreaDetail(QWidget* p, Core::CVObject* l, Core::CVControl::Type t) : CVBaseDetail(p, l), _control(t){
	setAcceptDrops(true);

	title(tr("Aree da cartografare"));
	description(tr("File shape"));

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

	if (controller()->isValid()) {
		lab->setText(QString::number(layer()->rows()));
	}
}

CVAreaDetail::~CVAreaDetail() {

}

void CVAreaDetail::clearAll() {
	controller()->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		QLabel* lab = _labels.at(i);
		lab->setText("");
	}
}

void CVAreaDetail::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa aree da cartografare"),
		Core::CVSettings::get(CV_PATH_SEARCH).toString(),
        "(*.shp)"
    );
	if (!uri.isEmpty()) {
		QFileInfo shp(uri);
		Core::CVSettings::set(CV_PATH_SEARCH, shp.absolutePath());
		importAll(QStringList() << shp.absolutePath() + QDir::separator() + shp.baseName());
	}
}

void CVAreaDetail::importAll(QStringList& uri) {
	layer()->shape(uri.at(0));
	controller()->persist();
	_labels.at(0)->setText(QString::number(layer()->rows()));
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
	importAll(QStringList() << _file->absolutePath() + QDir::separator() + _file->baseName());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
