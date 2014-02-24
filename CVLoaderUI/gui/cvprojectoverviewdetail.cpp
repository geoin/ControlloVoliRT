#include "cvprojectoverviewdetail.h"

#include <QPlainTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QTextDocument>
#include <QTextBlock>
#include <QComboBox>

#include "gui/cvgui_utils.h"

namespace CV {
namespace GUI {
namespace Details {

CVProjectOverviewDetail::CVProjectOverviewDetail(QWidget* p, Core::CVProject* c) : QWidget(p), _controller(c) {
	QVBoxLayout* l = new QVBoxLayout;
	setLayout(l);

	addProjectInformations(l);
	addNotesEditor(l);
}

void CVProjectOverviewDetail::addProjectInformations(QVBoxLayout* l) {
	QWidget* form = new QWidget(this);
	QFormLayout* info = new QFormLayout;
	form->setLayout(info);

	_addDescr(info, tr("Nome progetto"), _controller->name);
	_addDescr(info, tr("Tipo"), _controller->type == Core::CVProject::PHOTOGRAMMETRY ? tr("Fotogrammetria") : tr("Lidar"));
	_addDescr(info, tr("Data creazione"), _controller->creationDate().toString());
	_lastModified = _addDescr(info, tr("Ultima modifica"), _controller->lastModificationDate().toString());

	_addScaleCombo(info);

	l->addWidget(form);
}

void CVProjectOverviewDetail::_addScaleCombo(QFormLayout* form) {
	_scale = new QComboBox(this);
	_scale->addItem(tr("2000"));
	_scale->addItem(tr("10000"));
	if (_controller->scale == "2000") {
		_scale->setCurrentIndex(0);
	} else {
		_scale->setCurrentIndex(1);
	}
	_scale->setDisabled(true);
	form->addRow(tr("Scala"), _scale);
}

QLabel* CVProjectOverviewDetail::_addDescr(QFormLayout* form, QString label, QString descr) {
	QLabel* lab = new QLabel(label, this);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);

	QLabel* info = new QLabel(descr, this);
	info->setMinimumHeight(26);
	info->setMaximumHeight(26);
	info->setAlignment(Qt::AlignRight | Qt::AlignHCenter);

	form->addRow(lab, info);
	return info;
}

void CVProjectOverviewDetail::addNotesEditor(QVBoxLayout* l) {
	QWidget* container = new QWidget(this);
	QVBoxLayout* box = new QVBoxLayout;
	container->setLayout(box);

	_notes = new QPlainTextEdit(this);
	_notes->setPlainText(_controller->notes);
	_notes->setReadOnly(true);

	QBrush back = palette().foreground();
    const QColor& color = back.color();
	_notes->setStyleSheet(QString("QPlainTextEdit {background-color: rgba(%1, %2, %3, %3)").arg(QString::number(color.red() + 6), QString::number(color.green() + 6), QString::number(color.blue() + 6)) + "}");
	
	QWidget* toolbar = new QWidget(this);
	QHBoxLayout* tBox = new QHBoxLayout;
	tBox->setContentsMargins(0, 0, 0, 0);
	toolbar->setLayout(tBox);

	QPushButton* edit = new QPushButton("", this);
	edit->setMaximumSize(QSize(36, 28));
	edit->setMinimumSize(QSize(36, 28));
	edit->setToolTip(tr("Modifica"));
	edit->setCheckable(true);
    edit->setIcon(QIcon(":/graphics/icons/edit.png"));
	
	connect(edit, SIGNAL(toggled(bool)), this, SLOT(onEditProject(bool)));

	tBox->addWidget(edit);
	tBox->addStretch(2);
	
	QPushButton* btn = new QPushButton("", this);
	btn->setMaximumSize(QSize(36, 28));
	btn->setMinimumSize(QSize(36, 28));
	btn->setVisible(false);
	btn->setToolTip(tr("Salva"));
    btn->setIcon(QIcon(":/graphics/icons/save.png"));
	tBox->addWidget(btn);

	connect(btn, SIGNAL(clicked()), this, SLOT(onSaveProject()));
	connect(edit, SIGNAL(toggled(bool)), btn, SLOT(setVisible(bool)));
	connect(btn, SIGNAL(clicked()), edit, SLOT(toggle()));

	box->addWidget(_notes, 2);
	box->addWidget(toolbar);
	l->addWidget(container);

	connect(this, SIGNAL(updateNotes(const QString&)), _notes, SLOT(setPlainText(const QString&)));
	connect(this, SIGNAL(updateScale(int)), _scale, SLOT(setCurrentIndex(int)));
}

void CVProjectOverviewDetail::onSaveProject() {
	QTextDocument* doc = _notes->document();
	QStringList l;
	for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
         l << it.text();
	}
	_controller->notes = l.join("\n");
	_controller->scale = _scale->currentText();
	_controller->persist();
}

void CVProjectOverviewDetail::showEvent(QShowEvent* ev) {
	_lastModified->setText(_controller->lastModificationDate().toString());
}

void CVProjectOverviewDetail::onEditProject(bool b) {
	_notes->setReadOnly(!b);
	_scale->setDisabled(!b);
	if (b) {
		_notes->setStyleSheet("");
	} else {
		GUI::CVScopedCursor c;

		_lastModified->setText(_controller->lastModificationDate().toString());

		emit updateNotes(_controller->notes);
		emit updateScale(_controller->scale == "2000" ? 0 : 1);
		QBrush back = palette().foreground();
		const QColor& color = back.color();
		_notes->setStyleSheet(QString("QPlainTextEdit {background-color: rgba(%1, %2, %3, %3)").arg(QString::number(color.red() + 6), QString::number(color.green() + 6), QString::number(color.blue() + 6)) + "}");
	}
}

}
} // namespace GUI
} // namespace CV
