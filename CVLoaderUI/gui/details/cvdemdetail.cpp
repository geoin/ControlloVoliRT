#include "cvdemdetail.h"

#include "core/cvcore_utils.h"
#include "gui/cvgui_utils.h"

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

#include <assert.h>

namespace CV {
namespace GUI {
namespace Details {

//TODO: needs cleanup, all details should use the same hooks

CVDemDetail::CVDemDetail(QWidget* p, Core::CVObject* l, Core::CVControl::Type t) : CVBaseDetail(p, l), _control(t) {
	title(tr("DEM"));
	description(tr("File DEM"));

	l->controlType(_control);
	
    QFormLayout* form = new QFormLayout;

	QLabel* lab = new QLabel("", this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
	_labels << lab;

	QLabel* n = new QLabel(tr("Colonne"), this);
	n->setMinimumHeight(26);
	n->setMaximumHeight(26);
	n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);

	form->addRow(n, lab);

	lab = new QLabel("", this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
	_labels << lab;

	n = new QLabel(tr("Righe"), this);
	n->setMinimumHeight(26);
	n->setMaximumHeight(26);
	n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
	form->addRow(n, lab);
	
	lab = new QLabel("", this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
	_labels << lab;

	n = new QLabel("X", this);
	n->setMinimumHeight(26);
	n->setMaximumHeight(26);
	n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
	form->addRow(n, lab);

	lab = new QLabel("", this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
	_labels << lab;

	n = new QLabel("Y", this);
	n->setMinimumHeight(26);
	n->setMaximumHeight(26);
	n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
	form->addRow(n, lab);

	lab = new QLabel("", this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
	_labels << lab;

	n = new QLabel(tr("Misura celle"), this);
	n->setMinimumHeight(26);
	n->setMaximumHeight(26);
	n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
	form->addRow(n, lab);

	body(form);

	if (controller()->isValid()) {
		QStringList& data = file()->data();
		assert(data.size() == _labels.size());
		for (int i = 0; i < data.size(); ++i) {
			_labels.at(i)->setText(data.at(i));
		}
	}
}

CVDemDetail::~CVDemDetail() {

}

void CVDemDetail::clearAll() {
	controller()->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		_labels.at(i)->setText("");
	}
}

void CVDemDetail::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa DEM"),
		Core::CVSettings::get(CV_PATH_SEARCH).toString(),
        ""
    );
	if (!uri.isEmpty()) {
		QFileInfo info(uri);
		Core::CVSettings::set(CV_PATH_SEARCH, info.absolutePath());
		importAll(QStringList() << uri);
	}
}

void CVDemDetail::importAll(QStringList& uri) {
	CV::GUI::CVScopedCursor cur;

	file()->origin(uri.at(0));
	if (controller()->persist()) {
		info();

		QStringList& data = file()->data();
		assert(data.size() == _labels.size());
		for (int i = 0; i < data.size(); ++i) {
			_labels.at(i)->setText(data.at(i));
		}
	}
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
	importAll(QStringList() << _file->absoluteFilePath());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
