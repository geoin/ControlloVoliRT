#include "cvstationsdetail.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
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

CVStationsDetail::CVStationsDetail(QWidget* p) : CVBaseDetail(p) {
	title(tr("Stazioni permanenti"));
	description(tr("File zip o coppie (.n .o)"));

	QVBoxLayout* l = new QVBoxLayout;
	_stations = new QListWidget(this);
	_stations->setFrameStyle(QFrame::NoFrame);
	l->addWidget(_stations);

	body(l);
}

CVStationsDetail::~CVStationsDetail() {
}

CVStationDelegate* CVStationsDetail::addItem(const QString& name) {
	CVStationDelegate* d = new CVStationDelegate(this); 
	d->title(name);
	QListWidgetItem* item = new CVStationListItem(_stations, d);
	_stations->setItemWidget(item, d);
	item->setSizeHint(QSize(0, 72));
	_stations->insertItem(0, item);
	_stations->scrollToItem(item);

	return d;
}

void CVStationsDetail::dragEnterEvent(QDragEnterEvent* ev) {
	_files.clear();

    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    for (int i = 0; i < list.size(); i++) {
        QString uri = list.at(i).toLocalFile();
		QFileInfo info(uri);
        if (info.suffix().toLower() != "zip") {
            ev->ignore();
			_files.clear();
			return;
        } else {
			_files.insert(uri);
        }
    }
    ev->accept();
}

void CVStationsDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVStationsDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVStationsDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	foreach (const QString& f, _files) {
		QFileInfo info(f);
		QString rinex = info.baseName();
		CVStationDelegate* d = addItem(rinex);
		//TODO: other info
	}
    _files.clear();
}

} // namespace Details
} // namespace GUI
} // namespace CV
