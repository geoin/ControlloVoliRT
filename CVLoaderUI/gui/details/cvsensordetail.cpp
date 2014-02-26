#include "cvsensordetail.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QXmlStreamReader>

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
    _params.insert("SCANRATE", lineEdit(this, pal));

    foreach (QLineEdit* i, _params) {
        form->addRow(_params.key(i), i);
		i->setMinimumHeight(26);
		i->setMaximumHeight(26);
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

void CVSensorDetail::importAll(QStringList& uri) {
	QFile file(uri.at(0));
    bool open = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!open) {
        return;
    }
    QXmlStreamReader xml(&file);
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
                it->setText(xml.text().toString());
            }
        }
    }
    /*if(xml.hasError()) {
    }*/
}

void CVSensorDetail::dragEnterEvent(QDragEnterEvent*) {

}

void CVSensorDetail::dragMoveEvent(QDragMoveEvent*) {

}

void CVSensorDetail::dragLeaveEvent(QDragLeaveEvent*) {

}
 
void CVSensorDetail::dropEvent(QDropEvent*) {

}

} // namespace Details
} // namespace GUI
} // namespace CV
