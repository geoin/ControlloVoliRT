#include "cvbasedetail.h"

#include "gui/helper/cvactionslinker.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPalette>

#include <QMimeData>

#include <QFile>
#include <QUrl>
#include <QPushButton>

#include <QFileDialog>

//#include <QStandardPaths>

namespace CV {
namespace GUI {
namespace Details {

CVBaseDetail::CVBaseDetail(QWidget* p, Core::CVObject* c) : QWidget(p), _controller(c) {
    setAcceptDrops(true);

    QHBoxLayout* hLayout = new QHBoxLayout;
    _title = new QLabel(this);
    _title->setMaximumHeight(36);
    _title->setStyleSheet("padding: 4px; font: bold;");

    QPushButton* menuBtn = new QPushButton(tr(""), this);

    menuBtn->setMaximumSize(26, 26);
    _menu = new QMenu(this);
    menuBtn->setMenu(_menu);
    hLayout->addWidget(menuBtn);
    hLayout->addWidget(_title);
    QWidget* header = new QWidget(this);
    header->setLayout(hLayout);

    _descr = new QLabel(this);
    _descr->setIndent(10);
    _descr->setMaximumHeight(36);
    _descr->setStyleSheet("padding: 4px;");

    _body = new QWidget(this);

    QVBoxLayout* box = new QVBoxLayout;
    box->addWidget(header);
    box->addWidget(_descr);
    box->addWidget(_body, 2);
    setLayout(box);

	QMenu* m = detailMenu();
	connect(m->addAction(QIcon(""), tr("Apri")), SIGNAL(triggered()), this, SLOT(searchFile()));
	connect(m->addAction(QIcon(""), tr("Rimuovi")), SIGNAL(triggered()), this, SLOT(clearAll()));
}

void CVBaseDetail::createRow(QWidget* p, const QString& label, QLabel*& lab, QLabel*& info) {
	lab = new QLabel(label, p);
	lab->setMinimumHeight(26);
	lab->setMaximumHeight(26);
	lab->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);

	info = new QLabel("", p);
	info->setMinimumHeight(26);
	info->setMaximumHeight(26);
	info->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
}


} // namespace Details
} // namespace GUI
} // namespace CV
