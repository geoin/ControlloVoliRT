#include "cvflyattitudedetail.h"

#include "core/cvcore_utils.h"

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
#include <QDialogButtonBox>
#include <QTableWidget>

namespace CV {
namespace GUI {
namespace Details {

CVFlyAttitudeDetail::CVFlyAttitudeDetail(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	setAcceptDrops(true);

	title(tr("Assetti di volo"));
	description(tr("File degli assetti"));
	
    QFormLayout* form = new QFormLayout;

	QLabel* n = NULL;
	QLabel* info = NULL;

	createRow(this, tr("Numero strisciate"), n, info);
	_labels << info;
	form->addRow(n, info);
	
	createRow(this, tr("Numero fotogrammi"), n, info);
	_labels << info;
	form->addRow(n, info);

	_angle = new QComboBox(this);
	_angle->addItem("DEG");
	_angle->addItem("GON");
	form->addRow("Unità di misura", _angle); 
	connect(_angle, SIGNAL(currentIndexChanged(int)), this, SLOT(onAngleChanged(int)));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(form);

	//TODO: hide horizontal header
	_table = new QTableWidget(this);
	lay->addWidget(_table, 2);

	body(lay);

	if (controller()->isValid()) {
		Core::CVFlyAttitude::Angle_t u = layer()->angleUnit();
		_angle->setCurrentIndex(int(u));
		QList<QStringList> lines = layer()->readFirstLines(10);
		
		_table->clear();
		_table->setRowCount(lines.size());

		for (int i = 0; i < lines.size(); i++) {
			const QStringList& line = lines.at(i);
			_table->setColumnCount(line.size());
			for (int j = 0; j < line.size(); j++) {
				QTableWidgetItem* itm = new QTableWidgetItem(line.at(j));
				_table->setItem(i, j, itm);
			}
		}

		QStringList info = layer()->data();
		for (int i = 0; i < info.size(); ++i) {
			QLabel* lab = _labels.at(i);
			lab->setText(info.at(i));
		}
	} else {
		_table->hide();
		_angle->hide();
	}

	connect(controller(), SIGNAL(persisted()), this, SLOT(onDataPersisted()));
	connect(controller(), SIGNAL(itemInserted(int)), this, SLOT(onItemInserted(int)));
}

CVFlyAttitudeDetail::~CVFlyAttitudeDetail() {

}

void CVFlyAttitudeDetail::importAll(QStringList& uri) {
	layer()->origin(uri.at(0));

	CV::GUI::CVScopedCursor cur;

	res = QtConcurrent::run(controller(), &CV::Core::CVObject::persist);

	_dialog.setWindowTitle(tr("Caricamento assetti in corso.."));
	_dialog.resize(260, 100);
	_dialog.resizeBarWidth(230);
	_dialog.exec();

	_table->show();
	_angle->show();

	Core::CVFlyAttitude::Angle_t u = layer()->angleUnit();
	_angle->setCurrentIndex(int(u));
	QList<QStringList> lines = layer()->readFirstLines(10);

	_table->clear();
	_table->setRowCount(lines.size());

	for (int i = 0; i < lines.size(); i++) {
		const QStringList& line = lines.at(i);
		_table->setColumnCount(line.size());
		for (int j = 0; j < line.size(); j++) {
			QTableWidgetItem* itm = new QTableWidgetItem(line.at(j));
			_table->setItem(i, j, itm);
		}
	}

	info();
}

void CVFlyAttitudeDetail::onItemInserted(int el) {
	_dialog.setLabelText(tr("Elementi inseriti") + ": " + QString::number(el));
}

void CVFlyAttitudeDetail::onDataPersisted() {
	if (res.result()) {
		QStringList& info = layer()->data();
		for (int i = 0; i < _labels.size(); ++i) {
			QLabel* lab = _labels.at(i);
			lab->setText(info.at(i));
		}
	}
	_dialog.setLabelText("");
	_dialog.close();
}

void CVFlyAttitudeDetail::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa assetti"),
		Core::CVSettings::get(CV_PATH_SEARCH).toString(),
        ""
    );
	if (!uri.isEmpty()) {
		QFileInfo info(uri);
		Core::CVSettings::set(CV_PATH_SEARCH, info.absolutePath());
		importAll(QStringList() << uri);
	}
}

void CVFlyAttitudeDetail::clearAll() {
	controller()->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		QLabel* lab = _labels.at(i);
		lab->setText("");
	}
	
	_table->hide();
	_angle->hide();
}

void CVFlyAttitudeDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        QString uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(uri));
		ev->accept();
    }
}

void CVFlyAttitudeDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVFlyAttitudeDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVFlyAttitudeDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	importAll(QStringList() << _file->absoluteFilePath());
    _file.reset(NULL);
}

void CVFlyAttitudeDetail::onAngleChanged(int c) {
	layer()->setAngleUnit(Core::CVFlyAttitude::Angle_t(c));
}

} // namespace Details
} // namespace GUI
} // namespace CV
