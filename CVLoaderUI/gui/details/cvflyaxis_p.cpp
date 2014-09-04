#include "cvflyaxis_p.h"
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

#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>

namespace CV {
namespace GUI {
namespace Details {

//TODO: needs cleanup, field generation

CVFlyAxis_p::CVFlyAxis_p(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	title(tr("Assi di volo"));
	description(tr("File shape"));
	
    QVBoxLayout* form = new QVBoxLayout;
	table = new QTableWidget(0, 2, body());

	if (controller()->isValid()) {
		_populateTable();
	}

	form->addWidget(table);
	body(form);

}

void CVFlyAxis_p::_populateTable() {
	QStringList fields = layer()->fields();

	Core::CVShapeLayerWithMeta::CVMetaColList meta = layer()->refColumns();
	QStringList vals;
	vals << "";
	Q_FOREACH(Core::CVShapeLayerWithMeta::MetaCol col, meta) {
		vals << col.ref;
	}
		
	table->clear();

	table->setRowCount(fields.size());
	table->setVerticalHeaderLabels(fields);
	table->setHorizontalHeaderLabels(QStringList() << "" << "Valori primo record");
	//table->horizontalHeader()->hide();
	table->horizontalHeader()->setStretchLastSection(true);
	table->horizontalHeader()->setDefaultSectionSize(120);
	for (int i = 0; i < table->columnCount() - 1; ++i) {
		table->horizontalHeader()->setResizeMode(i, QHeaderView::Interactive);
	}
	
	for (int i = 0; i < table->rowCount(); ++i) {
		table->verticalHeader()->setResizeMode(i, QHeaderView::Fixed);
		QComboBox* box = new QComboBox(table);
		box->setObjectName(fields.at(i));
		box->addItems(vals);
		_editors << box;
		table->setCellWidget(i, 0, box);

		connect(box, SIGNAL(currentIndexChanged (const QString&)), this, SLOT(onComboSelected(const QString&)));
	}

	QStringList& info = layer()->data();
	for (int i = 0; i < info.size(); ++i) {
		QTableWidgetItem* item = new QTableWidgetItem(info.at(i));
		item->setTextAlignment(Qt::AlignRight);
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		table->setItem(i, 1, item);

	}
}

void CVFlyAxis_p::onComboSelected(const QString& val) {
	if (val.isEmpty()) {
		return;
	}

	QComboBox* box = qobject_cast<QComboBox*>(sender());
	QString data = box->objectName();

	Q_FOREACH(QComboBox* b, _editors) {
		if (b != box && b->currentIndex() == box->currentIndex()) {
			b->setCurrentIndex(0);
		}
	}

	bool ret = layer()->edit(val, data);
}

void CVFlyAxis_p::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa assi di volo"),
		Core::CVSettings::get("/paths/search").toString(),
        "(*.shp)"
    );
	if (!uri.isEmpty()) {
		QFileInfo shp(uri);
		Core::CVSettings::set("/paths/search", shp.absolutePath());
		importAll(QStringList() << shp.absolutePath() + QDir::separator() + shp.baseName());
	}
}

void CVFlyAxis_p::importAll(QStringList& uri) {
	layer()->shape(uri.at(0));
	if (controller()->persist()) {
		_populateTable();
	}
}
 
void CVFlyAxis_p::clearAll() {
	controller()->remove();
	table->clear();
	table->setRowCount(0);
	table->setColumnCount(0);
	table->reset();
}

CVFlyAxis_p::~CVFlyAxis_p() {

}

void CVFlyAxis_p::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        _uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(_uri));
        if (_file->suffix().toLower() != "shp") {
            _file.reset();
            ev->ignore();
        } else {
            ev->accept();
        }
    }
}

void CVFlyAxis_p::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVFlyAxis_p::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVFlyAxis_p::dropEvent(QDropEvent* ev) {
    ev->accept();
	importAll(QStringList() << _file->absolutePath() + QDir::separator() + _file->baseName());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
