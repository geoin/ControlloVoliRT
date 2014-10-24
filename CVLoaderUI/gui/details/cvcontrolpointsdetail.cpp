#include "cvcontrolpointsdetail.h"

#include "core/cvcore_utils.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMimeData>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QUrl>
#include <QDir>
#include <QHeaderView>

namespace CV {
namespace GUI {
namespace Details {

CVControlPointsDetail::CVControlPointsDetail(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	setAcceptDrops(true);

	title(tr("Punti di controllo"));
	description(tr("Coordinate punti di controllo"));
	
    QVBoxLayout* form = new QVBoxLayout;

	_table = new QTableWidget(this);
	_table->setColumnCount(4);
	_table->horizontalHeader()->setStretchLastSection(true);
	_table->setHorizontalHeaderLabels(QStringList() << "X" << "Y" << "Z" << "Nome");

	form->addWidget(_table);

	body(form);

	if (controller()->isValid()) {
		controller()->load();
		updateTable();
	}
}

CVControlPointsDetail::~CVControlPointsDetail() {

}

void CVControlPointsDetail::updateTable() {
	int i = 0;
			
	_table->setRowCount(input()->data().size());

	Q_FOREACH(const QStringList& l, input()->data()) {
		for (int j = 0; j < l.size(); j++) {
			QTableWidgetItem* item = new QTableWidgetItem(l.at(j));
			item->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			_table->setItem(i, j, item);
		}
		i++;
	}
}

void CVControlPointsDetail::clearAll() {
	controller()->remove();
	_table->clear();
}

void CVControlPointsDetail::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa punti di controllo"),
		Core::CVSettings::get(CV_PATH_SEARCH).toString(),
        ""
    );

	if (!uri.isEmpty()) {
		QFileInfo csv(uri);
		Core::CVSettings::set(CV_PATH_SEARCH, csv.absolutePath());
		importAll(QStringList() << csv.absoluteFilePath());
	}
}

void CVControlPointsDetail::importAll(QStringList& uri) {
	input()->setCsv(uri.at(0));
	if (controller()->persist()) {
		if (controller()->isValid()) {
			controller()->load();
			updateTable();
		}
	}
}

void CVControlPointsDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        _uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(_uri));
		ev->accept();
    }
}

void CVControlPointsDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVControlPointsDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVControlPointsDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	importAll(QStringList() << _file->absoluteFilePath());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
