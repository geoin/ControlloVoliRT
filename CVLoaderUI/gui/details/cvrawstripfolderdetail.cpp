#include "cvrawstripfolderdetail.h"

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

CVFolderDetail::CVFolderDetail(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	title(tr("Strip lidar - RAW"));
	description(tr("Cartella contentente le nuvole delle strisciate"));
	
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
}

CVFolderDetail::~CVFolderDetail() {

}

void CVFolderDetail::clearAll() {
	controller()->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		_labels.at(i)->setText("");
	}
}

void CVFolderDetail::searchFile() {
	QString uri = QFileDialog::getExistingDirectory(
        this,
        tr("Importa cartella"),
		Core::CVSettings::get(CV_PATH_SEARCH).toString()
    );
	if (!uri.isEmpty()) {
		QFileInfo info(uri);
		Core::CVSettings::set(CV_PATH_SEARCH, info.absolutePath());
		importAll(QStringList() << uri);
	}
}

void CVFolderDetail::importAll(QStringList& uri) {
	CV::GUI::CVScopedCursor cur;

	folder()->origin(uri.at(0));
	if (controller()->persist()) {
		info();
		QStringList& data = folder()->data();
		assert(data.size() == _labels.size());
		for (int i = 0; i < data.size(); ++i) {
			_labels.at(i)->setText(data.at(i));
		}
	}
}

void CVFolderDetail::dragEnterEvent(QDragEnterEvent* ev) {
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

void CVFolderDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVFolderDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVFolderDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	importAll(QStringList() << _file->absoluteFilePath());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
