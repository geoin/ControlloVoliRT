#include "cvcameradetail.h"

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

CVCameraDetail::CVCameraDetail(QWidget* p) : QWidget(p) {
    _file.reset(NULL);

    setAcceptDrops(true);

    QHBoxLayout* hLayout = new QHBoxLayout;
    QLabel* h = new QLabel(tr("Fotocamera"), this);
    h->setMaximumHeight(36);
    h->setStyleSheet("padding: 4px; font: bold;");

    QPushButton* menuBtn = new QPushButton(tr(""), this);

    menuBtn->setMaximumSize(20, 26);
    QMenu* menu = new QMenu(this);
    QAction* newCam = menu->addAction(QIcon(""), "Carica");
    QAction* editCam = menu->addAction(QIcon(""), "Modifica");
    QAction* clearCam = menu->addAction(QIcon(""), "Cancella");
    menuBtn->setMenu(menu);
    hLayout->addWidget(menuBtn);
    hLayout->addWidget(h);
    QWidget* header = new QWidget(this);
    header->setLayout(hLayout);

    QLabel* s = new QLabel(tr("Dati di progetto"), this);
    s->setIndent(10);
    s->setMaximumHeight(36);
    s->setStyleSheet("padding: 4px;");

    QFormLayout* form = new QFormLayout(this);
    QWidget* body = new QWidget(this);

    QPalette pal = palette();

    _params.insert("FOC", lineEdit(this, pal));
    _params.insert("DIMX", lineEdit(this, pal));
    _params.insert("DIMY", lineEdit(this, pal));
    _params.insert("DPIX", lineEdit(this, pal));
    _params.insert("XP", lineEdit(this, pal));
    _params.insert("YP", lineEdit(this, pal));

    foreach (QLineEdit* i, _params) {
        form->addRow(_params.key(i), i);
    }

    QLabel* descr = new QLabel("Descrizione:", this);
    _note = new QPlainTextEdit(this);
    _note->setReadOnly(true);
    form->addRow(descr, _note);
    body->setLayout(form);

    QVBoxLayout* box = new QVBoxLayout;
    box->addWidget(header);
    box->addWidget(s);
    box->addWidget(body, 2);
    setLayout(box);

    //TODO: move to controller
    connect(this, SIGNAL(cameraInput(QString)), SLOT(onCameraInput(QString)));
    connect(newCam, SIGNAL(triggered()), this, SLOT(onLoadCamParameters()));
    connect(editCam, SIGNAL(triggered()), this, SLOT(onEditCamParameters()));
    connect(clearCam, SIGNAL(triggered()), this, SLOT(onClearCamParameters()));
}

QLineEdit* CVCameraDetail::lineEdit(QWidget* p, const QPalette& pal) {
    QBrush back = pal.foreground();
    const QColor& color = back.color();
    QLineEdit* line = new QLineEdit("", p);
    line->setContextMenuPolicy(Qt::NoContextMenu);
    line->setAlignment(Qt::AlignRight);
    line->setReadOnly(true);
    line->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %3)").arg(QString::number(color.red() + 2), QString::number(color.green() + 2), QString::number(color.blue() + 2)));
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
    emit cameraInput(_file->absoluteFilePath());
    _file.reset(NULL);
}

void CVCameraDetail::onCameraInput(const QString& uri) {
    QFile file(uri);
    bool open = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!open) {
        return;
    }
    QXmlStreamReader xml(&file);
    while(!xml.atEnd()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::StartDocument) { //TODO: Rivedere
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
}

void CVCameraDetail::onLoadCamParameters() {
    QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa parametri fotocamera"),
        "",//QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0),
        "(*.xml)"
    );
    emit cameraInput(uri);
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
