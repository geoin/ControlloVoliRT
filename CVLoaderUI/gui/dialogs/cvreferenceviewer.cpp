#include "cvreferenceviewer.h"

#include <QTableWidget>
#include <QHeaderView>

#include <QVBoxLayout>
#include <QDialogButtonBox>

namespace CV {
namespace GUI {
namespace Dialogs {

void CVReferenceViewer::showEvent(QShowEvent* ev) {
	QDialog::showEvent(ev);
}

CVReferenceViewer::CVReferenceViewer(QWidget* p) : QDialog(p) {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	resize(600, 600);

    QVBoxLayout* box = new QVBoxLayout;
    QWidget* w = new QWidget(this);
    box->addWidget(w);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    box->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));

    setLayout(box);

    box = new QVBoxLayout;
    w->setLayout(box);

	_viewer = new QTableWidget(0, 4, this);
	_viewer->setHorizontalHeaderLabels(QStringList()<< tr("Nome") << tr("Unità di misura") << tr("Valore") << tr("Descrizione"));
	_viewer->verticalHeader()->hide();
	_viewer->horizontalHeader()->setStretchLastSection(true);
    _viewer->horizontalHeader()->setDefaultSectionSize(120);

	for (int i = 0; i < _viewer->columnCount() - 1; ++i) {
		_viewer->horizontalHeader()->setResizeMode(i, QHeaderView::Interactive);
	}

    box->addWidget(_viewer);

	_title = QString(tr("Valori di riferimento - %1"));
}

void CVReferenceViewer::updateFrom(const Core::CVProject& proj) {
	QDomDocument doc("refval");
	QFile file(proj.path + QDir::separator() + "refval.xml");
	if (!file.open(QIODevice::ReadOnly)) {
		return;
	}
	if (!doc.setContent(&file)) {
		file.close();
		return;
	}
	file.close();

	QDomElement docElem = doc.documentElement();

	QString targetNode;
	if (proj.type == Core::CVProject::LIDAR) { 
		setWindowTitle(_title.arg("lidar"));
		targetNode = "Lidar";
	} else {
		setWindowTitle(_title.arg("fotogrammetria (scala " + proj.scale + ")"));
		if (proj.scale == "2000") {
			targetNode = "RefScale_2000";
		} else {
			targetNode = "RefScale_10000";
		}
	}
	_viewTarget(docElem, targetNode);
}

void CVReferenceViewer::_viewTarget(const QDomElement& docElem, const QString& targetNode) {
	QDomNodeList list = docElem.elementsByTagName(targetNode);
	QDomNode target = list.at(0);
	QDomElement e = target.toElement(); 
	list = e.childNodes();
	if (!list.size()) {
		return;
	}

	target = list.at(0);
	_viewer->setRowCount(list.size());

	int row = 0;
	while (!target.isNull()) {
		QDomElement e = target.toElement(); 
		if (!e.isNull()) {
			int col = 0;
			QTableWidgetItem* item = NULL;

			QString name = e.tagName();
			QString descr = e.attributeNode("descr").value();
			QString uom = e.attributeNode("unimis").value();
			QString val = e.text();

			item = new QTableWidgetItem(name);
			item->setTextAlignment(Qt::AlignLeft);
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			_viewer->setItem(row, col++, item);
			
			item = new QTableWidgetItem(uom);
			item->setTextAlignment(Qt::AlignCenter);
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			_viewer->setItem(row, col++, item);

			item = new QTableWidgetItem(val);
			item->setTextAlignment(Qt::AlignRight);
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			_viewer->setItem(row, col++, item);

			item = new QTableWidgetItem(descr);
			item->setTextAlignment(Qt::AlignLeft);
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			_viewer->setItem(row, col++, item);

			row++;
		}
		target = target.nextSibling();
	}
	_viewer->adjustSize();
}

} // namespace Dialogs
} // namespace GUI
} // namespace CV
