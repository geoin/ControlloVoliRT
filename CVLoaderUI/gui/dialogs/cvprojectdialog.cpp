#include "cvprojectdialog.h"

#include "core/cvcore_utils.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QTextDocument>
#include <QTextBlock>

namespace CV {
namespace GUI {
namespace Dialogs {

CVProjectDialog::CVProjectDialog(QWidget* p) : QDialog(p) {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);

    QVBoxLayout* box = new QVBoxLayout;

    QWidget* w = new QWidget(this);
    box->addWidget(w);

    QDialogButtonBox* buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel
	);
    box->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(box);

    _name = new QLineEdit(this);
    _path = new QLineEdit(this);
    _conf = new QLineEdit(this);
    _note = new QPlainTextEdit(this);
    _type = new QComboBox(this);
    _type->addItem(tr("Fotogrammetria"));
    _type->addItem(tr("Lidar"));
	connect(_type, SIGNAL(currentIndexChanged(int)), this, SLOT(changeProjectType(int)));

	_scale = new QComboBox(this);
	_scale->addItem(tr("2000"));
	_scale->addItem(tr("10000"));

	_epsg = new QComboBox(this);
	_epsg->setValidator(new QIntValidator);
	_epsg->setEditable(true);
	_epsg->setInsertPolicy(QComboBox::NoInsert);
	_epsg->addItems(QStringList() << "6704" << "6705" << "6706" << "6707" << "6708" << "4936" << "4937" << "4258" << "25832" << "25833" << "4265" << "3003" << "3004");

    QFormLayout* form = new QFormLayout;
    form->addRow(tr("Nome"), _name);

    QWidget* filePicker = new QWidget(this);
    QHBoxLayout* fbox = new QHBoxLayout;
    fbox->addWidget(_path, 2);
    QPushButton* fileBtn = new QPushButton(this);
    fbox->addWidget(fileBtn);
    fbox->setContentsMargins(0, 0, 0, 0);
    filePicker->setLayout(fbox);

    connect(fileBtn, SIGNAL(clicked()), this, SLOT(selectProjectFolder()));

	QWidget* confPicker = new QWidget(this);
    fbox = new QHBoxLayout;
    fbox->addWidget(_conf, 2);
    fileBtn = new QPushButton(this);
    fbox->addWidget(fileBtn);
    fbox->setContentsMargins(0, 0, 0, 0);
    confPicker->setLayout(fbox);

    connect(fileBtn, SIGNAL(clicked()), this, SLOT(selectConfiguration()));

    form->addRow(tr("Cartella"), filePicker);
    form->addRow(tr("Configurazione"), confPicker);
    form->addRow(tr("Tipo"), _type);
    form->addRow(tr("Scala"), _scale);
    form->addRow(tr("EPSG"), _epsg);
    form->addRow(tr("Note"), _note);
    w->setLayout(form);
}

void CVProjectDialog::changeProjectType(int idx) {
	_scale->setDisabled(idx != 0);
}

void CVProjectDialog::selectConfiguration() {
	QString ref = QFileDialog::getOpenFileName(this, 
		tr("Importa valori di riferimento"), 
		Core::CVSettings::get(CV_PATH_PROJECT).toString(),
		tr("*.xml")
	);
	_conf->setText(ref);
}

void CVProjectDialog::selectProjectFolder() {
	QDir d(Core::CVSettings::get(CV_PATH_PROJECT).toString());
	d.cdUp();

	QString dir = QFileDialog::getExistingDirectory(
		this,
		tr("Selezionare cartella"),
		d.absolutePath()
	);
	if (!dir.isEmpty()) {
		_path->setText(dir);
	}
}

bool CVProjectDialog::getInput(Core::CVProject& proj) {
    proj.name = _name->text();
	proj.path = _path->text() + QDir::separator() + proj.name;
    proj.type = _type->currentIndex() == 0 ? Core::CVProject::PHOTOGRAMMETRY : Core::CVProject::LIDAR;
	proj.refPath = _conf->text();
	if (proj.type == Core::CVProject::PHOTOGRAMMETRY) {
		proj.scale = _scale->currentText();
	}

	proj.datum = _epsg->currentText().toInt();

	QTextDocument* doc = _note->document();
	QStringList l;
	for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
         l << it.text();
	}
	proj.notes = l.join("\n");
	return !proj.name.isEmpty() && !proj.path.isEmpty() && !proj.refPath.isEmpty();
}

} // namespace Dialogs
} // namespace GUI
} // namespace CV
