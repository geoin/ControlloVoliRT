#include "cvcameradetail.h"

#include "gui/helper/cvactionslinker.h"

#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPalette>

#include <QMimeData>

#include <QFile>
#include <QXmlStreamReader>
#include <QMenu>
#include <QUrl>
#include <QPushButton>

#include <QFileDialog>
//#include <QStandardPaths>

namespace CV {
namespace GUI {
namespace Details {

CVCameraDetail::CVCameraDetail(QWidget* p, Core::CVCamera* cam) : CVBaseDetail(p) {
    setAcceptDrops(true);

    QMenu* menu = detailMenu();
    QAction* open = menu->addAction(QIcon(""), tr("Apri"));
    QAction* clearCam = menu->addAction(QIcon(""), tr("Cancella"));
    //TODO: move to controller
    connect(this, SIGNAL(cameraInput(QString)), SLOT(onCameraInput(QString)));

    connect(open, SIGNAL(triggered()), this, SLOT(onLoadCamParameters()));
    connect(clearCam, SIGNAL(triggered()), this, SLOT(clearAll()));

    //QAction* editCam = menu->addAction(QIcon(""), tr("Modifica"));
    //connect(editCam, SIGNAL(triggered()), this, SLOT(onEditCamParameters()));

    QFormLayout* form = new QFormLayout;

    QPalette pal = palette();
    _params.insert("FOC", lineEdit(this, pal));
    _params.insert("DIMX", lineEdit(this, pal));
    _params.insert("DIMY", lineEdit(this, pal));
    _params.insert("DPIX", lineEdit(this, pal));
    _params.insert("XP", lineEdit(this, pal));
    _params.insert("YP", lineEdit(this, pal));

    foreach (QLineEdit* i, _params) {
        form->addRow(_params.key(i), i);
		i->setMinimumHeight(26);
		i->setMaximumHeight(26);
    }

	title(tr("Fotocamera"));

	QLabel* descr = new QLabel(tr("Descrizione:"), this);
    _note = new QPlainTextEdit(this);
    _note->setReadOnly(true);
    form->addRow(descr, _note);
    body(form);

	if (cam) {
		_cam = cam;
		bool plan = _cam->isPlanning(); 
		_cam->data().planning = plan;
		
		description(tr(plan ? "Fotocamera di progetto" : "Fotocamera di missione"));
		
		if (_cam->isValid()) {
			view();
		}
	}
}

QLineEdit* CVCameraDetail::lineEdit(QWidget* p, const QPalette& pal) {
    QBrush back = pal.foreground();
    const QColor& color = back.color();
    QLineEdit* line = new QLineEdit("", p);
    line->setContextMenuPolicy(Qt::NoContextMenu);
    line->setAlignment(Qt::AlignRight);
    line->setReadOnly(true);
    line->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %3)").arg(QString::number(color.red() + 6), QString::number(color.green() + 6), QString::number(color.blue() + 6)));
    return line;
}

void CVCameraDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        _uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(_uri));
        if (_file->suffix().toLower() != "xml") {
            _file.reset(NULL);
            ev->ignore();
        } else {
            ev->accept();
        }
    }
}

void CVCameraDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVCameraDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVCameraDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
    onCameraInput(_file->absoluteFilePath());
    _file.reset(NULL);
}

//TODO: move validation in control class
void CVCameraDetail::onCameraInput(const QString& uri) {
    QFile file(uri);
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
            if (key == "DESCR") {
                _note->setPlainText(xml.text().toString().simplified());
            } else {
                QLineEdit* it = _params.value(key);
                if (it != NULL) {
                    it->setText(xml.text().toString());
                }
            }
        }
    }
    if(xml.hasError()) {

    }

	save();
}

void CVCameraDetail::save() {
	Camera& c = _cam->data();
	c.foc = _params.value("FOC")->text().toDouble();
	c.dimx = _params.value("DIMX")->text().toDouble();
	c.dimy = _params.value("DIMY")->text().toDouble();
	c.dpix = _params.value("DPIX")->text().toDouble();
	c.xp = _params.value("XP")->text().toDouble();
	c.yp = _params.value("YP")->text().toDouble();
	
	c.descr = _note->toPlainText().toStdString();

	_cam->persist();
}

void CVCameraDetail::view() {
	Camera& c = _cam->data();
	_params.value("FOC")->setText(QString::number(c.foc, 'f', 4));
	_params.value("DIMX")->setText(QString::number(c.dimx, 'f', 4));
	_params.value("DIMY")->setText(QString::number(c.dimy, 'f', 4));
	_params.value("DPIX")->setText(QString::number(c.dpix, 'f', 4));
	_params.value("XP")->setText(QString::number(c.xp, 'f', 4));
	_params.value("YP")->setText(QString::number(c.yp, 'f', 4));

	 _note->setPlainText(c.descr.c_str());
}

void CVCameraDetail::clearAll() {
	_cam->remove();
	view();
}

void CVCameraDetail::onLoadCamParameters() {
    QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa parametri fotocamera"),
        "",//QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0), //QT5 only
        "(*.xml)"
    );
    onCameraInput(uri);
}

void CVCameraDetail::onEditCamParameters() {
    foreach (QLineEdit* i, _params) {
        i->setReadOnly(false);
        i->setStyleSheet("");
    }
    _note->setReadOnly(false);
}

void CVCameraDetail::onClearCamParameters() {
    foreach (QLineEdit* i, _params) {
        i->setText("");
    }
    _note->setPlainText(QString());
}


} // namespace Details
} // namespace GUI
} // namespace CV
