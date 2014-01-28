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

#include <assert.h>

namespace CV {
namespace GUI {
namespace Details {

CVStationsDetail::CVStationsDetail(QWidget* p, Core::CVStations* s) : CVBaseDetail(p) {
	assert(s != NULL);
	_handler = s;

	title(tr("Stazioni permanenti"));
	description(tr("File zip o coppie (.n .o)"));

	QVBoxLayout* l = new QVBoxLayout;
	_stations = new QListWidget(this);
	_stations->setFrameStyle(QFrame::NoFrame);
	l->addWidget(_stations);

	body(l);

	for (int i = 0; i < _handler->count(); ++i) {
		addItem(_handler->at(i)->name());
	}
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

	QRegExp reg(".*n|.*o"); //TODO: move to class "\\..*n?o?"

    for (int i = 0; i < list.size(); i++) {
        QString uri = list.at(i).toLocalFile();
		QFileInfo info(uri);
		QString ext = info.suffix().toLower(); 
		if (_base.isEmpty()) {
			_base = ext;
			_station = info.baseName().left(7);
		}

		bool ok = false;
		if ( ext == "zip") {
			if (_base == "zip") {
				ok = true;
			} 
		} else if (_base != "zip"){
			int pos = reg.indexIn(ext);
			if (pos == 0) {
				ok = true;
			}
		}

		if (!ok) {
            ev->ignore();
			_base = QString();
			_station = QString();
			_files.clear();
			return;
		} else {
			_files.append(uri);
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

//TODO: handle this in controller
void CVStationsDetail::dropEvent(QDropEvent* ev) {
    ev->accept();

	//need a tmp dir
	QFileInfo db(_handler->uri());
	QString tmp = db.absolutePath() + QDir::separator() + "tmp";
	QDir d;
	d.mkpath(tmp);
	bool cd = d.cd(tmp);
	if (!cd) {
		return;
	}
	
	//station name
	
	//check if this station is already in for actual mission, if so get the zip and decompress
	QString id;
	QString zipToUpdate = _handler->getZipFromStation(_station, tmp, id);
	if (!zipToUpdate.isEmpty()) {
		Core::CVZip::unzip(zipToUpdate.toStdString(), tmp.toStdString());
		d.remove(zipToUpdate);
	}

	//if a zip is dragged, decompress it in tmp
	if (_base == "zip") {
		foreach (const QString& f, _files) {
			Core::CVZip::unzip(f.toStdString(), tmp.toStdString());
		}
		QStringList tmpFiles = d.entryList(QDir::Files);

		_files.clear();
		foreach (const QString& n, tmpFiles) {
			_files.append(tmp + QDir::separator() + n);
		}
	} //else { copy .n .o in tmp }

	std::vector<std::string> files;
	foreach (const QString& f, _files) {
		files.push_back(f.toStdString());
	}

	//new zip creation
	QString z(tmp + QDir::separator() + _station + ".zip");
	Core::CVZip::zip(files, z.toStdString());

	//update view
	QFileInfo info(z);
	QString rinex = info.baseName();
	
	//TODO (FIX): needs to handle station id inside controller
	Core::CVStation* r = id.isEmpty() ? new Core::CVStation(this) : new Core::CVStation(this, id);
	r->mission(_handler->mission());
	r->origin(info.absoluteFilePath());
	r->uri(_handler->uri());
	if (!r->persist()) {
		r->deleteLater();
		return;
	}

	if (!_items.contains(rinex) && id.isEmpty()) {
		_items << rinex;
		/*CVStationDelegate* del = */addItem(rinex);
		//TODO: other info
		_handler->add(r);
	} else {
		r->deleteLater();
	}

	//clean up
	d.setNameFilters(QStringList() << "*.*");
	d.setFilter(QDir::Files);
	foreach (const QString& dirFile, d.entryList()) {
		d.remove(dirFile);
	}
	d.cdUp();
	d.rmdir(tmp);

    _files.clear();
	_base = QString();
	_station = QString();
}

} // namespace Details
} // namespace GUI
} // namespace CV
