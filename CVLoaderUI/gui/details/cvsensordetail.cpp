#include "cvsensordetail.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QXmlStreamReader>
#include <QMimeData>
#include <QUrl>
#include <QPushButton>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>

namespace CV {
namespace GUI {
namespace Details {

CVSensorDetail::CVSensorDetail(QWidget* p, Core::CVObject* cam) : CVBaseDetail(p, cam) {
	setAcceptDrops(true);
	title(tr("Sensore lidar"));
	
	QFormLayout* form = new QFormLayout;
    body(form);

    QPalette pal = palette();
    _params.insert("FOV", lineEdit(this, pal));
    _params.insert("IFOV", lineEdit(this, pal));
    _params.insert("FREQ", lineEdit(this, pal));
    _params.insert("SCAN_RATE", lineEdit(this, pal));

    foreach (QLineEdit* i, _params) {
        form->addRow(_params.key(i), i);
		i->setMinimumHeight(26);
		i->setMaximumHeight(26);
    }
	
	QPushButton* b = new QPushButton(tr("Save"), this);
	b->setMaximumWidth(64);
	b->setVisible(false);
	form->addWidget(b);
	
	connect(b, SIGNAL(pressed()), this, SLOT(saveFormData()));
	
	QAction* edit = detailMenu()->addAction(QIcon(""), tr("Edita"));
	connect(edit, SIGNAL(triggered()), this, SLOT(edit()));
	connect(edit, SIGNAL(triggered()), b, SLOT(show()));

	description(tr(sensor()->isPlanning() ? "Sensore lidar di progetto" : "Sensore lidar di missione"));
	if (controller()->isValid()) {
		view();
	}
}

CVSensorDetail::~CVSensorDetail() {

}

void CVSensorDetail::clearAll() {
	controller()->remove();
	
    foreach (QLineEdit* i, _params) {
        i->setText("");
    }
}

void CVSensorDetail::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa parametri sensore lidar"),
		Core::CVSettings::get("/paths/search").toString(),
        "(*.xml)"
    );
	if (!uri.isEmpty()) {
		QFileInfo info(uri);
		Core::CVSettings::set("/paths/search", info.absolutePath());
		importAll(QStringList() << uri);
	}
}

void CVSensorDetail::edit() { 
	foreach (QLineEdit* i, _params) {
		enableLineEdit(i);
    }
}

void CVSensorDetail::saveFormData() {
	qobject_cast<QWidget*>(sender())->hide();

	foreach (QLineEdit* i, _params) {
		disableLineEdit(this, i);
    }

	save();
}

void CVSensorDetail::save() { 
	Core::CVSensor::SensorData& s = sensor()->data();
	s.fov = _params.value("FOV")->text().toDouble();
	s.ifov = _params.value("IFOV")->text().toDouble();
	s.freq = _params.value("FREQ")->text().toDouble();
	s.scan_rate = _params.value("SCAN_RATE")->text().toDouble();

	controller()->persist();
}

void CVSensorDetail::view() {
	Core::CVSensor::SensorData& s = sensor()->data();
	_params.value("FOV")->setText(QString::number(s.fov, 'f', 2));
	_params.value("IFOV")->setText(QString::number(s.ifov, 'f', 2));
	_params.value("FREQ")->setText(QString::number(s.freq, 'f', 2));
	_params.value("SCAN_RATE")->setText(QString::number(s.scan_rate, 'f', 2));
}

void CVSensorDetail::importAll(QStringList& uri) {
	QFile file(uri.at(0));
    bool open = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!open) {
        return;
    }
    QXmlStreamReader xml(&file);
	int i = 0;
    while(!xml.atEnd()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::StartDocument) { //TODO: check
            continue;
        }

        if(token == QXmlStreamReader::StartElement) {
            QStringRef name = xml.name();
            xml.readNext();
            QString key = name.toString().toUpper();
            QLineEdit* it = _params.value(key);
            if (it != NULL) {
				i++;
                it->setText(xml.text().toString());
            }
        }
    }
    if(i == _params.size()) {
		save();
	}
}

void CVSensorDetail::dragEnterEvent(QDragEnterEvent* ev) {
	const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        _uri = list.at(0).toLocalFile();
        QFileInfo info(_uri);
        if (info.suffix().toLower() != "xml") {
            ev->ignore();
        } else {
            ev->accept();
        }
    }
}

void CVSensorDetail::dragMoveEvent(QDragMoveEvent* ev) {
	ev->accept();
}

void CVSensorDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
	ev->accept();
	_uri = QString();
}
 
void CVSensorDetail::dropEvent(QDropEvent* ev) {
	ev->accept();
    importAll(QStringList() << _uri);
	_uri = QString();
}

} // namespace Details
} // namespace GUI
} // namespace CV
