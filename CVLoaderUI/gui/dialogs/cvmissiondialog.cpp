#include "cvmissiondialog.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QDialogButtonBox>

namespace CV {
namespace GUI {
namespace Dialogs {

CVMissionDialog::CVMissionDialog(QWidget* p) : QDialog(p) {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout* box = new QVBoxLayout;

    QWidget* w = new QWidget(this);
    box->addWidget(w);

    QDialogButtonBox* buttons = new QDialogButtonBox(
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    box->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    setLayout(box);

    QFormLayout* form = new QFormLayout;
    w->setLayout(form);

    _name = new QLineEdit(this);
    _note = new QPlainTextEdit(this);
    form->addRow(tr("Nome missione:"), _name);
    form->addRow(tr("Note:"), _note);

    setWindowTitle(tr("Nuova missione"));
}

void CVMissionDialog::getInput(QString& name, QString& note) {
    name = _name->text();
    note = _note->toPlainText().simplified(); //TODO: voglio rich text?
}

} // namespace Dialogs
} // namespace GUI
} // namespace CV
