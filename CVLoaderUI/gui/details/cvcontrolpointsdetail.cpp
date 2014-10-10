#include "cvcontrolpointsdetail.h"

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

CVControlPointsDetail::CVControlPointsDetail(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	setAcceptDrops(true);

	title(tr("Punti di controllo"));
	description(tr("File shape"));
	
    QFormLayout* form = new QFormLayout;

	QLabel* lab, * info;
	createRow(this, tr(""), lab, info);
	_labels << info;
	form->addRow(lab, info);

	body(form);

	if (controller()->isValid()) {
		_labels.at(0)->setText(tr("Dati inseriti"));
	}
}

CVControlPointsDetail::~CVControlPointsDetail() {

}

void CVControlPointsDetail::clearAll() {
	controller()->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		QLabel* lab = _labels.at(i);
		lab->setText("");
	}
}

void CVControlPointsDetail::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa punti di controllo"),
		Core::CVSettings::get("/paths/search").toString(),
        ""
    );

	if (!uri.isEmpty()) {
		QFileInfo csv(uri);
		Core::CVSettings::set("/paths/search", csv.absolutePath());
		importAll(QStringList() << csv.absoluteFilePath());
	}
}

void CVControlPointsDetail::importAll(QStringList& uri) {
	input()->setCsv(uri.at(0));
	if (controller()->persist()) {
		_labels.at(0)->setText(tr("Dati inseriti"));
	} else {
		_labels.at(0)->setText(tr(""));
	}
}

void CVControlPointsDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        _uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(_uri));
		ev->accept();
    }
}

void CVControlPointsDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVControlPointsDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVControlPointsDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	importAll(QStringList() << _file->absoluteFilePath());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
