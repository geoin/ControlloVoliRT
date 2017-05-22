#include "cvcloudsampledetail.h"

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

CVCloudSampleDetail::CVCloudSampleDetail(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	setAcceptDrops(true);

    title(tr("Lidar test area"));
    description(tr("Cartella contentente le nuvole delle aree di test"));

	
    QFormLayout* form = new QFormLayout;

    QLabel* lab = new QLabel("", this);
    lab->setMinimumHeight(26);
    lab->setMaximumHeight(26);
    lab->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
    _labels << lab;

    QLabel* n = new QLabel(tr("Percorso"), this);
    n->setMinimumHeight(26);
    n->setMaximumHeight(26);
    n->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);

    form->addRow(n, lab);

    body(form);

    if (controller()->isValid()) {
        QStringList& data = folder()->data();
        assert(data.size() == _labels.size());
        for (int i = 0; i < data.size(); ++i) {
            _labels.at(i)->setText(data.at(i));
        }
    }

//	QLabel* lab, * info;
//	createRow(this, tr(""), lab, info);
//	_labels << info;
//	form->addRow(lab, info);

//	body(form);

//	if (controller()->isValid()) {
//		_labels.at(0)->setText(tr("Dati inseriti"));
//	}
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
//	QString uri = QFileDialog::getOpenFileName(
//        this,
//        tr("Importa nuovola di punti area di test"),
//		Core::CVSettings::get(CV_PATH_SEARCH).toString(),
//        ""
//    );
    QString uri = QFileDialog::getExistingDirectory(
        this,
        tr("Importa cartella nuvole di test"),
        Core::CVSettings::get(CV_PATH_SEARCH).toString()
    );
	if (!uri.isEmpty()) {
		QFileInfo shp(uri);
		Core::CVSettings::set(CV_PATH_SEARCH, shp.absolutePath());
		importAll(QStringList() << uri);
	}
}

void CVCloudSampleDetail::importAll(QStringList& uri) {
    CV::GUI::CVScopedCursor cur;

   folder()->origin(uri.at(0));
	//input()->origin(uri.at(0));
	//fld->origin( QString("pippo" ));
	if (controller()->persist()) {
        info();
        QStringList& data = folder()->data();
        assert(data.size() == _labels.size());
        for (int i = 0; i < data.size(); ++i) {
            _labels.at(i)->setText(data.at(i));
        }
    }

//	input()->origin(uri.at(0));

//	if (controller()->persist()) {
//		info();
//	}
//	_labels.at(0)->setText(tr("Dati inseriti"));
}

void CVCloudSampleDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        _uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(_uri));
        if (!_file->isDir()) {
            _file.reset();
            ev->ignore();
        } else {
            ev->accept();
        }
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
	importAll(QStringList() << _file->absoluteFilePath());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
