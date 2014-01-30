#include "cvprojectdialog.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>

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
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    box->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(box);

    _name = new QLineEdit(this);
    _path = new QLineEdit(this);
    _note = new QPlainTextEdit(this);
    _type = new QComboBox(this);
    _type->addItem(tr("Fotogrammetria"));
    _type->addItem(tr("Lidar"));

    QFormLayout* form = new QFormLayout;
    form->addRow(tr("Nome"), _name);

    QWidget* filePicker = new QWidget(this);
    QHBoxLayout* fbox = new QHBoxLayout;
    fbox->addWidget(_path, 2);
    QPushButton* fileBtn = new QPushButton(this);
    fbox->addWidget(fileBtn);
    fbox->setContentsMargins(0, 0, 0, 0);
    filePicker->setLayout(fbox);

    form->addRow(tr("Cartella"), filePicker);
    form->addRow(tr("Tipo"), _type);
    form->addRow(tr("Note"), _note);
    w->setLayout(form);

    connect(fileBtn, SIGNAL(clicked()), this, SLOT(selectProjectFolder()));
}

void CVProjectDialog::selectProjectFolder() {
	QString dir = QFileDialog::getExistingDirectory(
		NULL,
		tr("Selezionare cartella"),
		""
	);
	_path->setText(dir);
}

void CVProjectDialog::getInput(Core::CVProject& proj) {
    proj.name = _name->text();
	proj.path = _path->text() + QDir::separator() + proj.name;
    proj.notes = _note->toPlainText().simplified();
    proj.type = _type->currentIndex() == 0 ? Core::CVProject::PHOTOGRAMMETRY : Core::CVProject::LIDAR;

}

} // namespace Dialogs
} // namespace GUI
} // namespace CV
