#include "cvrinexdetail.h"

#include "gui/helper/cvactionslinker.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

#include <QMimeData>

#include <QFile>
#include <QUrl>

#include <QFileDialog>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

namespace CV {
namespace GUI {
namespace Details {

CVRinexDetail::CVRinexDetail(QWidget* p, Core::CVRinex* rinex) : CVBaseDetail(p) {
	assert(rinex != NULL);
	_rinex = rinex;

    setAcceptDrops(true);

	title(tr("Rinex aereo"));
	description(tr("File rinex"));
}

void CVRinexDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        QString uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(uri));
        if (_file->suffix().toLower() != "zip") {
            _file.reset(NULL);
            ev->ignore();
        } else {
            ev->accept();
        }
    }
}

void CVRinexDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVRinexDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVRinexDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	_rinex->origin(_file->absoluteFilePath());
	_rinex->persist();
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
