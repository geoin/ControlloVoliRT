#include "cvsensordetail.h"

#include "gui/cvgui_utils.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QXmlStreamReader>
#include <QMimeData>
#include <QUrl>
#include <QPushButton>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include "gui/helper/cvactionslinker.h"

#include <limits>

#define CV_MAX_DECIMALS 3

namespace CV {
namespace GUI {
namespace Details {

CVSensorDetail::CVSensorDetail(QWidget* p, Core::CVObject* cam) : CVBaseDetail(p, cam), _isEditing(false) {
	Helper::CVSignalLinker* linker = Helper::CVSignalHandle::get();
	linker->on(Helper::QUIT, this, SLOT(onQuit()));

	setAcceptDrops(true);
	title(tr("Sensore lidar"));
	
	QFormLayout* form = new QFormLayout;
    body(form);

    QPalette pal = palette();
	QLineEdit* line = NULL;

	line = lineEdit(this, pal);
	line->setValidator(new QDoubleValidator(0.0, std::numeric_limits<qreal>::max(), CV_MAX_DECIMALS, line));
    _params.insert("FOV", line);

	line = lineEdit(this, pal);
	line->setValidator(new QDoubleValidator(0.0, std::numeric_limits<qreal>::max(), CV_MAX_DECIMALS, line));
    _params.insert("FREQ", line);
	
	line = lineEdit(this, pal);
	line->setValidator(new QDoubleValidator(0.0, std::numeric_limits<qreal>::max(), CV_MAX_DECIMALS, line));
    _params.insert("SCAN_RATE", line);
	
	line = lineEdit(this, pal);
	line->setValidator(new QDoubleValidator(0.0, std::numeric_limits<qreal>::max(), CV_MAX_DECIMALS, line));
    _params.insert("IFOV", line);

	if (sensor()->isPlanning()) {
		line = lineEdit(this, pal);
		line->setValidator(new QDoubleValidator(0.0, std::numeric_limits<qreal>::max(), CV_MAX_DECIMALS, line));
		_params.insert("SPEED", line);
	}

    foreach (QLineEdit* i, _params) {
		QString field = _keyToFieldName(_params.key(i));
		if (field.isEmpty()) {
			continue;
		}
        form->addRow(field, i);
		i->setMinimumHeight(26);
		i->setMaximumHeight(26);
    }
	
	QPushButton* b = new QPushButton(tr("Salva"), this);
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

void CVSensorDetail::onQuit() {
	if (!_isEditing) {
		return;
	}

	int ret = CVMessageBox::message(this, tr("<b>Modifiche parametri sensore pendenti.</b>"), tr("Salvare ora?"), 260, QString(), QMessageBox::Yes| QMessageBox::No, QMessageBox::Yes);
	if (ret == QMessageBox::Ok) {
		CVScopedCursor cur;
		save();
	}
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
		Core::CVSettings::get(CV_PATH_SEARCH).toString(),
        "(*.xml)"
    );
	if (!uri.isEmpty()) {
		QFileInfo info(uri);
		Core::CVSettings::set(CV_PATH_SEARCH, info.absolutePath());
		importAll(QStringList() << uri);
	}
}

void CVSensorDetail::edit() { 
	foreach (QLineEdit* i, _params) {
		enableLineEdit(i);
    }
	
	_isEditing = true;
}

void CVSensorDetail::saveFormData() {
	CVScopedCursor cur;

	qobject_cast<QWidget*>(sender())->hide();

	foreach (QLineEdit* i, _params) {
		disableLineEdit(this, i);
    }

	save();

	_isEditing = false;
}

void CVSensorDetail::save() { 
	Core::CVSensor::SensorData& s = sensor()->data();
	s.fov = _params.value("FOV")->text().toDouble();
	s.ifov = _params.value("IFOV")->text().toDouble();
	s.freq = _params.value("FREQ")->text().toDouble();
	s.scan_rate = _params.value("SCAN_RATE")->text().toDouble();
	
	if (sensor()->isPlanning()) {
		s.speed = _params.value("SPEED")->text().toDouble();
	}
	if (controller()->persist()) {
		info();
	}
}

void CVSensorDetail::view() {
	Core::CVSensor::SensorData& s = sensor()->data();
	_params.value("FOV")->setText(QString::number(s.fov, 'f', CV_MAX_DECIMALS));
	_params.value("IFOV")->setText(QString::number(s.ifov, 'f', CV_MAX_DECIMALS));
	_params.value("FREQ")->setText(QString::number(s.freq, 'f', CV_MAX_DECIMALS));
	_params.value("SCAN_RATE")->setText(QString::number(s.scan_rate, 'f', CV_MAX_DECIMALS));
	
	if (sensor()->isPlanning()) {
		_params.value("SPEED")->setText(QString::number(s.speed, 'f', CV_MAX_DECIMALS));
	}
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
    if (i <= _params.size()) /*No speed for now*/ {
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

QString CVSensorDetail::_keyToFieldName(const QString& key) {
	if (key == "FOV") {
		return QString(tr("Angolo di scansione (deg)"));
	}
	if (key == "FREQ") {
		return QString(tr("Frequenza di campionamento (KHz)"));
	}
	if (key == "SCAN_RATE") {
		return QString(tr("Frequenza di scansione (Hz)"));
	}
	if (key == "SPEED") {
		return QString(tr("Velocità aereo (m/s)"));
	}
	if (key == "IFOV") {
		return QString(tr("Divergenza fascio"));
	}
	return QString();
}

} // namespace Details
} // namespace GUI
} // namespace CV
