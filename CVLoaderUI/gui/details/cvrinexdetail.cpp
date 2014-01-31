#include "cvrinexdetail.h"

#include "gui/helper/cvactionslinker.h"
#include "gui/cvgui_utils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

#include <QMimeData>

#include <QFile>
#include <QUrl>

#include <QFileDialog>

#include <QListWidget>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

#include "core/cvcore_utils.h"

namespace CV {
namespace GUI {
namespace Details {

	//TODO: STATIONS AND RINEX DETAIL MUST SHARE CODE, do it as soon as possible

CVRinexDetail::CVRinexDetail(QWidget* p, Core::CVRinex* rinex) : CVBaseDetail(p) {
	assert(rinex != NULL);
	_rinex = rinex;

    setAcceptDrops(true);

	QVBoxLayout* l = new QVBoxLayout;
	_name = new QLabel(this);

	_details = new QListWidget(this);
	_details->setSelectionMode(QAbstractItemView::NoSelection);
	_details->setAlternatingRowColors(true);
	_details->setFocusPolicy(Qt::NoFocus);

	l->addWidget(_name);
	l->addWidget(_details, 2);

	body(l);

	title(tr("Rinex aereo"));
	description(tr("File rinex"));
	
	QMenu* m = detailMenu();
	connect(m->addAction(QIcon(""), tr("Rimuovi")), SIGNAL(triggered()), this, SLOT(clearAll()));

	if (_rinex->isValid()) {
		_name->setText(_rinex->name());

		QStringList data;
		_rinex->list(data);
		foreach (const QString& f, data) {
			QListWidgetItem* it = new QListWidgetItem(_details);
			it->setSizeHint(QSize(0, 26));
			it->setText(f);
			_details->insertItem(0, it);
		}
	}
}

void CVRinexDetail::dragEnterEvent(QDragEnterEvent* ev) {
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
			_station = info.baseName();
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

void CVRinexDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVRinexDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVRinexDetail::dropEvent(QDropEvent* ev) {
    ev->accept();

	CV::GUI::CVScopedCursor cur;

	//need a tmp dir
	
	CV::Core::CVScopedTmpDir tmpDir(QFileInfo(_rinex->uri()).absolutePath());
	const QString& tmp = tmpDir.toString();
	assert(!tmp.isEmpty());

	QDir& d = tmpDir.dir();

	/*
	TODO
	*/

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
	
	_rinex->origin(z);
	_rinex->persist();

	_name->setText(_rinex->name());
	QStringList data;
	_rinex->list(data);
	_details->clear();
	foreach (const QString& f, data) {
		QListWidgetItem* it = new QListWidgetItem(_details);
		it->setSizeHint(QSize(0, 26));
		it->setText(f);
		_details->insertItem(0, it);
	}

	_files.clear();
}

void CVRinexDetail::clearAll() {
	_rinex->remove();
	_name->setText("");
	_details->clear();
}

} // namespace Details
} // namespace GUI
} // namespace CV
