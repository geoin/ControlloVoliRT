#include "cvflyaxis_p.h"
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

//TODO: needs cleanup, field generation

CVFlyAxis_p::CVFlyAxis_p(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	title(tr("Assi di volo"));
	description(tr("File shape"));

    QFormLayout* form = new QFormLayout;

	QLabel* n = NULL;
	QLabel* info = NULL;
	
	createRow(this, tr("Record inseriti"), n, info);
	_labels << info;
	form->addRow(n, info);

	QStringList fields = layer()->fields();
	Q_FOREACH(QString val, fields) {
		createRow(this, val, n, info);
		_labels << info;
		form->addRow(n, info);

	}

	body(form);

	if (controller()->isValid()) {
		QStringList& info = layer()->data();
		_labels.at(0)->setText(QString::number(layer()->rows()));
		for (int i = 0; i < info.size(); ++i) {
			QLabel* lab = _labels.at(i + 1);
			lab->setText(info.at(i));
		}
	}
}

void CVFlyAxis_p::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa assi di volo"),
		Core::CVSettings::get("/paths/search").toString(),
        "(*.shp)"
    );
	if (!uri.isEmpty()) {
		QFileInfo shp(uri);
		Core::CVSettings::set("/paths/search", shp.absolutePath());
		importAll(QStringList() << shp.absolutePath() + QDir::separator() + shp.baseName());
	}
}

void CVFlyAxis_p::importAll(QStringList& uri) {
	layer()->shape(uri.at(0));
	if (controller()->persist()) {
		QStringList& info = layer()->data();
		for (int i = 0; i < info.size(); ++i) {
			QLabel* lab = _labels.at(i + 1);
			lab->setText(info.at(i));
		}
	}
	_labels.at(0)->setText(QString::number(layer()->rows()));
}
 
void CVFlyAxis_p::clearAll() {
	controller()->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		QLabel* lab = _labels.at(i);
		lab->setText("");
	}
}

CVFlyAxis_p::~CVFlyAxis_p() {

}

void CVFlyAxis_p::dragEnterEvent(QDragEnterEvent* ev) {
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

void CVFlyAxis_p::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVFlyAxis_p::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVFlyAxis_p::dropEvent(QDropEvent* ev) {
    ev->accept();
	importAll(QStringList() << _file->absolutePath() + QDir::separator() + _file->baseName());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
