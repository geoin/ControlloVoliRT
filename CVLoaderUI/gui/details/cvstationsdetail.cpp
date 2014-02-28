#include "cvstationsdetail.h"

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
#include <QMimeData>

#include <assert.h>

namespace CV {
namespace GUI {
namespace Details {

CVStationsDetail::CVStationsDetail(QWidget* p, Core::CVObject* s) : CVBaseDetail(p, s) {
	assert(s != NULL);

	title(tr("Stazioni permanenti"));
	description(tr("File zip o coppie (.n .o)"));

	QVBoxLayout* l = new QVBoxLayout;
	_stations = new QListWidget(this);
	_stations->setFrameStyle(QFrame::NoFrame);

	_details = new QListWidget(this);
	_details->setVisible(false);
	_details->setSelectionMode(QAbstractItemView::NoSelection);
	_details->setAlternatingRowColors(true);
	_details->setFocusPolicy(Qt::NoFocus);

	connect(_stations, SIGNAL(currentRowChanged(int)), this, SLOT(onStationSelected(int)));

	l->addWidget(_stations, 3);
	l->addWidget(_details, 2);

	body(l);

	for (int i = 0; i < stations()->count(); ++i) {
		addItem(stations()->at(i)->name());
	}
	
	connect(this, SIGNAL(persisted()), this, SLOT(onDataPersisted()));
	connect(this, SIGNAL(importQueued(const QStringList&)), this, SLOT(importAll(const QStringList&)), Qt::QueuedConnection);
}

CVStationsDetail::~CVStationsDetail() {
}

void CVStationsDetail::clearAll() {
	controller()->remove();
	_stations->clear();
	_details->clear();
}

void CVStationsDetail::onUpdateStatus(QString str) {
	_dialog.setLabelText(str);
}

void CVStationsDetail::onDataPersisted() {
	_dialog.setLabelText(tr("Salvataggio in corso.."));

	QFileInfo info = res.result();
	QString rinex = info.baseName();
	Core::CVStation::Ptr r(_id.isEmpty() ? new Core::CVStation(this) : new Core::CVStation(this, _id));
	r->mission(stations()->mission());
	r->origin(info.absoluteFilePath());
	r->uri(stations()->uri());
	if (r->persist()) {
		if (_id.isEmpty()) {
			CVStationDelegate* del = addItem(rinex);
			//TODO: other info
			stations()->add(r);
		} 
		

		if (_details->isVisible()) {
			int index = _stations->currentRow();
			onStationSelected(index);
		}
	}
	
	_base = QString();
	_station = QString();
	_id = QString();
	_dialog.setLabelText("");
	_dialog.close();
}

void CVStationsDetail::searchFile() {
	QStringList uri = QFileDialog::getOpenFileNames(
        this,
        tr("Importa dati GPS"),
		Core::CVSettings::get("/paths/search").toString(),
        "(*.*n *.*o *.zip)"
    );

	QSet<QString> ext;
	if (!uri.isEmpty()) {
		for (int i = 0; i < uri.size(); ++i) {
			QFileInfo info(uri.at(i));
			ext << info.completeSuffix().toLower();
			if (_station.isEmpty()) {
				_station = info.baseName().left(7);
				Core::CVSettings::set("/paths/search", info.absolutePath());
			}
		}
		if (ext.size() == 1 && ext.contains("zip")) { //one or more zip
			_base = "zip";
		} else if ((uri.size() % 2) || ext.contains("zip")) { //.n .o, but not valid
			return;
		}

		emit importQueued(uri);
	}
}

//TODO: move to controller
QFileInfo CVStationsDetail::_importAllAsync(Core::CVScopedTmpDir& tmpDir, QStringList& uri) {
	QString tmp = tmpDir.toString();
	if (tmp.isEmpty()) {
		emit persisted();
		return QFileInfo();
	}

	emit onUpdateStatus(tr("Analisi dei dati in corso.."));
	
	//check if this station is already in for actual mission, if so get the zip and decompress
	QString zipToUpdate = stations()->getZipFromStation(_station, tmp, _id);
	if (!zipToUpdate.isEmpty()) {
		Core::CVZip::unzip(zipToUpdate.toStdString(), tmp.toStdString());
		tmpDir.dir().remove(zipToUpdate);
	}

	//if a zip is dragged, decompress it in tmp
	if (_base == "zip") {
		foreach (const QString& f, uri) {
			Core::CVZip::unzip(f.toStdString(), tmp.toStdString());
		}
	} else { 
		foreach (const QString& f, uri) {
			QFileInfo info(f);
			QFile::copy(f, tmp + QDir::separator() + info.fileName());// .n .o in tmp 
		}
	}
	
	QStringList tmpFiles = tmpDir.dir().entryList(QDir::Files);

	uri.clear();
	foreach (const QString& n, tmpFiles) {
		uri.append(tmp + QDir::separator() + n);
	}

	std::vector<std::string> files;
	foreach (const QString& f, uri) {
		files.push_back(f.toStdString());
	}

	
	emit onUpdateStatus(tr("Creazione pacchetto.."));

	//new zip creation
	QString z(tmp + QDir::separator() + _station + ".zip");
	bool ok = Core::CVZip::zip(files, z.toStdString());
	if (!ok) {
		emit persisted();
		return QFileInfo();
	}

	//update view
	QFileInfo info(z);
	emit persisted();
	//TODO (FIX): needs to handle station id inside controller
	return info;
}

void CVStationsDetail::importAll(const QStringList& uri) {
	GUI::CVScopedCursor cur;
	
	Core::CVScopedTmpDir tmpDir(QFileInfo(stations()->uri()).absolutePath());
	res = QtConcurrent::run(this, &CVStationsDetail::_importAllAsync, tmpDir, uri);

	_dialog.setWindowTitle(tr("Caricamento stazione in corso.."));
	_dialog.resize(260, 100);
	_dialog.resizeBarWidth(230);
	_dialog.exec();
}

void CVStationsDetail::onStationSelected(int item) {
	if (item < 0 || !stations()->count()) {
		_details->clear();
		_details->setVisible(false);
	} else if (item < stations()->count()) {
		CV::GUI::CVScopedCursor cur;

		_details->clear();
		Core::CVStation* i = stations()->at(item);
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
	stations()->removeAt(r);
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
	emit importQueued(_files);
}

} // namespace Details
} // namespace GUI
} // namespace CV
