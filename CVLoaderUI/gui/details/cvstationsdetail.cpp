#include "cvstationsdetail.h"

#include "gui/cvgui_utils.h"
#include "core/cvcore_utils.h"

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
	
	QMenu* m = detailMenu();
	connect(m->addAction(QIcon(""), tr("Rimuovi")), SIGNAL(triggered()), this, SLOT(clearAll()));

	QVBoxLayout* l = new QVBoxLayout;
	_stations = new QListWidget(this);
	_stations->setFrameStyle(QFrame::NoFrame);

	_details = new QListWidget(this);
	_details->setMaximumHeight(100);
	_details->setVisible(false);
	_details->setSelectionMode(QAbstractItemView::NoSelection);
	_details->setAlternatingRowColors(true);
	_details->setFocusPolicy(Qt::NoFocus);

	connect(_stations, SIGNAL(currentRowChanged(int)), this, SLOT(onStationSelected(int)));

	l->addWidget(_stations);
	l->addWidget(_details);

	body(l);

	for (int i = 0; i < _handler->count(); ++i) {
		addItem(_handler->at(i)->name());
	}
}

CVStationsDetail::~CVStationsDetail() {
}

void CVStationsDetail::clearAll() {
	_handler->remove();
	_stations->clear();
	_details->clear();
}

void CVStationsDetail::onStationSelected(int item) {
	if (item < 0 || !_handler->count()) {
		_details->clear();
		_details->setVisible(false);
	} else if (item < _handler->count()) {
		CV::GUI::CVScopedCursor cur;

		_details->clear();
		Core::CVStation* i = _handler->at(item);
		QStringList data;
		i->list(data);
		foreach (const QString& f, data) {
			QListWidgetItem* it = new QListWidgetItem(_details);
			it->setSizeHint(QSize(0, 26));
			it->setText(f);
			_details->insertItem(0, it);
		}
		_details->setVisible(true);
	} 
}

void CVStationsDetail::onRemoveStation(int r) {
	_handler->removeAt(r);
}

CVStationDelegate* CVStationsDetail::addItem(const QString& name) {
	CVStationDelegate* d = new CVStationDelegate(this); 
	d->title(name);
	CVStationListItem* item = new CVStationListItem(_stations, d);
	connect(item, SIGNAL(removeStation(int)), this, SLOT(onRemoveStation(int)));
	_stations->setItemWidget(item, d);
	item->setSizeHint(QSize(0, 72));
	_stations->insertItem(0, item);
	_stations->scrollToItem(item);
	//_stations->setCurrentItem(item);
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

	CV::GUI::CVScopedCursor cur;

	CV::Core::CVScopedTmpDir tmpDir(QFileInfo(_handler->uri()).absolutePath());

	//need a tmp dir
	const QString& tmp = tmpDir.toString();
	if (tmp.isEmpty()) {
		return;
	}
	
	//check if this station is already in for actual mission, if so get the zip and decompress
	QString id;
	QString zipToUpdate = _handler->getZipFromStation(_station, tmp, id);
	if (!zipToUpdate.isEmpty()) {
		Core::CVZip::unzip(zipToUpdate.toStdString(), tmp.toStdString());
		tmpDir.dir().remove(zipToUpdate);
	}

	//if a zip is dragged, decompress it in tmp
	if (_base == "zip") {
		foreach (const QString& f, _files) {
			Core::CVZip::unzip(f.toStdString(), tmp.toStdString());
		}
		QStringList tmpFiles = tmpDir.dir().entryList(QDir::Files);

		_files.clear();
		foreach (const QString& n, tmpFiles) {
			_files.append(tmp + QDir::separator() + n);
		}
	} else { 
		foreach (const QString& f, _files) {
			QFileInfo info(f);
			QFile::copy(f, tmp + QDir::separator() + info.fileName());// .n .o in tmp 
		}
	}

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
	Core::CVStation::Ptr r(id.isEmpty() ? new Core::CVStation(this) : new Core::CVStation(this, id));
	r->mission(_handler->mission());
	r->origin(info.absoluteFilePath());
	r->uri(_handler->uri());
	if (!r->persist()) {
		return;
	}

	if (id.isEmpty()) {
		CVStationDelegate* del = addItem(rinex);
		//TODO: other info
		_handler->add(r);
	} 

	//clean up
	
    _files.clear();
	_base = QString();
	_station = QString();

	if (_details->isVisible()) {
		int index = _stations->currentRow();
		onStationSelected(index);
	}
}

} // namespace Details
} // namespace GUI
} // namespace CV