#include "cvfinaldatadetail.h"

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
#include <QLineEdit>
#include <QSpinBox>

#include <assert.h>

namespace CV {
namespace GUI {
namespace Details {

CVFinalDataDetail::CVFinalDataDetail(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	title(tr("Dati grezzi"));
	description(tr(""));
	
    QFormLayout* form = new QFormLayout;

    QWidget* w = nullptr;
//    w = _addFolderPicker("FINAL_RAW_STRIP_DATA");
//    form->addRow("****Dati grezzi LAS", w);
//    _data["FINAL_RAW_STRIP_DATA"] = w;
	
	w = _addFolderPicker("FINAL_GROUND_ELL");
    form->addRow("Tile GROUND", w);
	_data["FINAL_GROUND_ELL"] = w;
	
//    w = _addFolderPicker("FINAL_GROUND_ORTO");
//    form->addRow("****Dati ground ortometrici", w);
//    _data["FINAL_GROUND_ORTO"] = w;
	
	w = _addFolderPicker("FINAL_OVERGROUND_ELL");
    form->addRow("Tile OVERGROUND", w);
	_data["FINAL_OVERGROUND_ELL"] = w;
	
//    w = _addFolderPicker("FINAL_OVERGROUND_ORTO");
//    form->addRow("***Dati overground ortometrici", w);
//    _data["FINAL_OVERGROUND_ORTO"] = w;
	
	w = _addFolderPicker("FINAL_MDS");
	form->addRow("Grid MDS ortometrico", w);
	_data["FINAL_MDS"] = w;
	
	w = _addFolderPicker("FINAL_MDT");
	form->addRow("Grid MDT ortometrico", w);
	_data["FINAL_MDT"] = w;
	
	w = _addFolderPicker("FINAL_INTENSITY");
	form->addRow("Modello intensità", w);
	_data["FINAL_INTENSITY"] = w;
	
    w = _addFilePicker("FINAL_IGM_GRID");
    form->addRow("Grigliati IGMI", w);
    _data["FINAL_IGM_GRID"] = w;

//    QSpinBox* box = new QSpinBox(this);
//    box->setRange(0, 10000);
//    box->setSingleStep(1000);

//    form->addRow("Misura tile griglia (m)", box);

//	connect(box, SIGNAL(valueChanged(int)), this, SLOT(onTileSizeChanged(int)));

	body(form);

    if (controller()->isValid()) {
        QStringList& data = input()->data();
        QStringList tables = input()->tables();
		//for( int i = 0; i < tables.size(); i++ ) {
		//	std::string ss = tables.at( i ).toStdString();
		//	int a = 1;
		//}
		//for( int i = 0; i < data.size(); i++ ) {
		//	std::string ss = data.at( i ).toStdString();
		//	int a = 1;
		//}
		int kk = _editors.size();

        assert(data.size() >= _editors.size());
        //for (int i = 0; i < _editors.size(); ++i) {
		for( int i = 0; i < data.size(); ++i ) {
			std::string ss = data.at( i ).toStdString();
			if( ss.empty() )
				continue;
			std::string ss1 = tables.at( i ).toStdString();
            _editors[tables.at(i)]->setText(data.at(i));
        }

//		box->setValue(input()->tileSize());
    }
}

CVFinalDataDetail::~CVFinalDataDetail() {

}

void CVFinalDataDetail::onTileSizeChanged(int size) {
	input()->set("FINAL_RAW_STRIP_DATA", "TILE_SIZE", size);
	if (input()->persist()) {
		info();
	}
}

QWidget* CVFinalDataDetail::_addFolderPicker(QString table, QString column) {
	QWidget* filePicker = new QWidget(this);
	QHBoxLayout* fbox = new QHBoxLayout;
	
	QLineEdit* path = new QLineEdit(this);
	path->setReadOnly(true);
	_editors[table] = path;

	fbox->addWidget(path, 2);
	QPushButton* fileBtn = new QPushButton(this);
	fbox->addWidget(fileBtn);
	fbox->setContentsMargins(0, 0, 0, 0);
	filePicker->setLayout(fbox);

	fileBtn->setProperty("TABLE", table);
	fileBtn->setProperty("COLUMN", column);
	fileBtn->setProperty("INDEX", _editors.size() - 1);
	
	filePicker->setFixedHeight(32);
	path->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);

	connect(fileBtn, SIGNAL(clicked()), this, SLOT(finalFolder()));

	return filePicker;
}

QWidget* CVFinalDataDetail::_addFilePicker(QString table, QString column) {
	QWidget* filePicker = new QWidget(this);
	QHBoxLayout* fbox = new QHBoxLayout;
	
	QLineEdit* path = new QLineEdit(this);
	path->setReadOnly(true);
	_editors[table] = path;

	fbox->addWidget(path, 2);
	QPushButton* fileBtn = new QPushButton(this);
	fbox->addWidget(fileBtn);
	fbox->setContentsMargins(0, 0, 0, 0);
	filePicker->setLayout(fbox);

	fileBtn->setProperty("TABLE", table);
	fileBtn->setProperty("COLUMN", column);
	fileBtn->setProperty("INDEX", _editors.size() - 1);
	
	filePicker->setFixedHeight(32);
	path->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);

	connect(fileBtn, SIGNAL(clicked()), this, SLOT(finalFile()));

	return filePicker;
}

void CVFinalDataDetail::finalFolder() {
	QString ref = QFileDialog::getExistingDirectory(this, 
		tr("Trova cartella"), 
		Core::CVSettings::get(CV_PATH_SEARCH).toString()
	);

	if (!ref.isEmpty()) {
		QFileInfo in(ref);
		Core::CVSettings::set(CV_PATH_SEARCH, in.absolutePath());

		QString table = sender()->property("TABLE").toString();
		QString folder = sender()->property("COLUMN").toString();
	

		input()->set(table, folder, ref);
		_editors[table]->setText(ref);
		if (controller()->persist()) {
			info();
		}
	}
}

void CVFinalDataDetail::finalFile() {
	QStringList ref = QFileDialog::getOpenFileNames(this, 
		tr("Trova file"), 
		Core::CVSettings::get(CV_PATH_SEARCH).toString()
	);

	if (ref.size() == 0) { return; }
	
	QString path = controller()->uri();
	QString file;
	if (ref.size() == 1) {
		QFileInfo in(ref.at(0));
		file = path + QDir::separator() + in.fileName();
		QFile::copy(ref.at(0), file);

	} else {
		QFileInfo f(ref.at(0));
		//QString path = f.absoluteDir().absolutePath();
	
		QFile out(path + QDir::separator() + "list.txt");
		out.open(QIODevice::WriteOnly);
		foreach(QString in, ref) {
			out.write(QString(in + "\n").toUtf8());
		}

		file = out.fileName();
	}

	QFileInfo in(file);
	Core::CVSettings::set(CV_PATH_SEARCH, in.absolutePath());

	QString table = sender()->property("TABLE").toString();
	QString folder = sender()->property("COLUMN").toString();
	
		
	input()->set(table, folder, in.fileName());
	_editors[table]->setText(file);
	if (controller()->persist()) {
		info();
	}
}


void CVFinalDataDetail::clearAll() {
	controller()->remove();
	QMap<QString, QLineEdit*>::iterator it = _editors.begin();
	QMap<QString, QLineEdit*>::iterator end = _editors.end();
	for (; it != end; it++) {
		it.value()->setText("");
	}
}

void CVFinalDataDetail::searchFile() {}

void CVFinalDataDetail::importAll(QStringList& uri) {}

void CVFinalDataDetail::dragEnterEvent(QDragEnterEvent* ev) {}

void CVFinalDataDetail::dragMoveEvent(QDragMoveEvent* ev) {}

void CVFinalDataDetail::dragLeaveEvent(QDragLeaveEvent* ev) {}

void CVFinalDataDetail::dropEvent(QDropEvent* ev) {}

} // namespace Details
} // namespace GUI
} // namespace CV
