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

	QWidget* w = _addFolderPicker("FINAL_RAW_STRIP_DATA");
	form->addRow("Dati grezzi LAS", w);
	_data["FINAL_RAW_STRIP_DATA"] = w;
	
	w = _addFolderPicker("FINAL_GROUND_ELL");
	form->addRow("Dati ground ellissoidici", w);
	_data["FINAL_GROUND_ELL"] = w;
	
	w = _addFolderPicker("FINAL_GROUND_ORTO");
	form->addRow("Dati ground ortometrici", w);
	_data["FINAL_GROUND_ORTO"] = w;
	
	w = _addFolderPicker("FINAL_OVERGROUND_ELL");
	form->addRow("Dati overground ellissoidici", w);
	_data["FINAL_OVERGROUND_ELL"] = w;
	
	w = _addFolderPicker("FINAL_OVERGROUND_ORTO");
	form->addRow("Dati overground ortometrici", w);
	_data["FINAL_OVERGROUND_ORTO"] = w;
	
	w = _addFolderPicker("FINAL_MDS");
	form->addRow("Dati mds", w);
	_data["FINAL_MDS"] = w;
	
	w = _addFolderPicker("FINAL_MDT");
	form->addRow("Dati mdt", w);
	_data["FINAL_MDT"] = w;
	
	w = _addFolderPicker("FINAL_INTENSITY");
	form->addRow("Dati intensità", w);
	_data["FINAL_INTENSITY"] = w;
	
	w = _addFolderPicker("FINAL_IGM_GRID");
	form->addRow("Grigliati", w);
	_data["FINAL_IGM_GRID"] = w;

	form->addRow("Misura tile griglia (Km)", new QSpinBox(this));

	body(form);

	/*
	if (controller()->isValid()) {
		QStringList& data = input()->data();
		assert(data.size() == _labels.size());
		for (int i = 0; i < data.size(); ++i) {
			_labels.at(i)->setText(data.at(i));
		}
	}
	*/
}

CVFinalDataDetail::~CVFinalDataDetail() {

}

QWidget* CVFinalDataDetail::_addFolderPicker(QString table, QString column) {
	QWidget* filePicker = new QWidget(this);
	QHBoxLayout* fbox = new QHBoxLayout;
	QLineEdit* path = new QLineEdit(this);
	fbox->addWidget(path, 2);
	QPushButton* fileBtn = new QPushButton(this);
	fbox->addWidget(fileBtn);
	fbox->setContentsMargins(0, 0, 0, 0);
	filePicker->setLayout(fbox);

	fileBtn->setProperty("TABLE", table);
	fileBtn->setProperty("COLUMN", column);
	
	filePicker->setFixedHeight(32);
	path->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);

	connect(fileBtn, SIGNAL(clicked()), this, SLOT(finalFolder()));

	return filePicker;
}

void CVFinalDataDetail::finalFolder() {
	QString ref = QFileDialog::getExistingDirectory(this, 
		tr("Trova cartella"), 
		Core::CVSettings::get(CV_PATH_PROJECT).toString()
	);

	if (ref.isEmpty()) {
		return;
	}

	QString table = sender()->property("TABLE").toString();
	QString folder = sender()->property("COLUMN").toString();
	
	qobject_cast<QLineEdit*>(_data[table])->setText(ref);

}

void CVFinalDataDetail::clearAll() {
	controller()->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		_labels.at(i)->setText("");
	}
}

void CVFinalDataDetail::searchFile() {
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

void CVFinalDataDetail::importAll(QStringList& uri) {
	CV::GUI::CVScopedCursor cur;

	input()->origin(uri.at(0));
	if (controller()->persist()) {
		QStringList& data = input()->data();
		assert(data.size() == _labels.size());
		for (int i = 0; i < data.size(); ++i) {
			_labels.at(i)->setText(data.at(i));
		}
	}
}

void CVFinalDataDetail::dragEnterEvent(QDragEnterEvent* ev) {}

void CVFinalDataDetail::dragMoveEvent(QDragMoveEvent* ev) {}

void CVFinalDataDetail::dragLeaveEvent(QDragLeaveEvent* ev) {}

void CVFinalDataDetail::dropEvent(QDropEvent* ev) {}

} // namespace Details
} // namespace GUI
} // namespace CV
