#include "cvcloudsampledetail.h"

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

CVCloudSampleDetail::CVCloudSampleDetail(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	setAcceptDrops(true);

	title(tr("Nuvola area di test"));
	description(tr("Nuvola di punti"));

	
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

CVCloudSampleDetail::~CVCloudSampleDetail() {

}

void CVCloudSampleDetail::clearAll() {
	controller()->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		QLabel* lab = _labels.at(i);
		lab->setText("");
	}
}

void CVCloudSampleDetail::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa nuovola di punti area di test"),
		Core::CVSettings::get("/paths/search").toString(),
        "(*.shp)"
    );
	if (!uri.isEmpty()) {
		QFileInfo shp(uri);
		Core::CVSettings::set("/paths/search", shp.absolutePath());
		importAll(QStringList() << shp.absolutePath() + QDir::separator() + shp.baseName());
	}
}

void CVCloudSampleDetail::importAll(QStringList& uri) {
	controller()->persist();
	_labels.at(0)->setText(tr("Dati inseriti"));
}

void CVCloudSampleDetail::dragEnterEvent(QDragEnterEvent* ev) {
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

void CVCloudSampleDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVCloudSampleDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVCloudSampleDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	importAll(QStringList() << _file->absolutePath() + QDir::separator() + _file->baseName());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV