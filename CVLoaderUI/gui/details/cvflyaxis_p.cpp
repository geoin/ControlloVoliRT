#include "cvflyaxis_p.h"
#include "core/cvcore_utils.h"
#include "gui/cvgui_utils.h"

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
	table = new QTableWidget(body());

	if (controller()->isValid()) {
		_populateTable();
	}

	form->addWidget(table);
	body(form);
}

void CVFlyAxis_p::_populateTable() {
	CVScopedCursor cur;

	QStringList fields = layer()->fields();

	QMap<QString, QString> meta = layer()->refColumns();
	
	bool editable = false;

	//Check reference columns
	if (meta.size()) {
		QStringList keys = meta.keys();
		int count = 0;
		Q_FOREACH(QString k, keys) {
			if (fields.contains(k)) {
				count++;
			}
		}

		editable = count < keys.size();
	}

	//Check that actual targets columns are valid in this shape, if not clear it
	QStringList targets = meta.values();
	Q_FOREACH(QString v, targets) { 
		if (fields.contains(v) == false) {
			layer()->edit(meta.key(v), QString());
		}
	}

	QStringList vals;
	vals << "";

	QMap<QString, QString>::const_iterator i = meta.constBegin();
	while (i != meta.constEnd()) {
		vals << i.key();
		++i;
	}
		
	table->clear();
	_editors.clear();
	
	table->setColumnCount(editable ? 2 : 1);
	table->setRowCount(fields.size());
	table->setVerticalHeaderLabels(fields);
	table->setHorizontalHeaderLabels(QStringList() << "Valori di esempio" << "Riferimento");

	table->horizontalHeader()->setStretchLastSection(!editable);
	table->horizontalHeader()->setDefaultSectionSize(120);

	for (int i = 0; i < table->columnCount() - 1; ++i) {
		table->horizontalHeader()->setResizeMode(i, QHeaderView::Stretch);
	}
	
	int colIdx = 0;
	QStringList& info = layer()->data();
	for (int i = 0; i < info.size(); ++i) {
		QTableWidgetItem* item = new QTableWidgetItem(info.at(i));
		item->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		table->setItem(i, colIdx, item);
	}
	colIdx++;

	if (editable) {
		for (int i = 0; i < table->rowCount(); ++i) {
			table->verticalHeader()->setResizeMode(i, QHeaderView::Fixed);
			QComboBox* box = new QComboBox(table);
			box->setObjectName(fields.at(i));
			box->addItems(vals);
			table->setCellWidget(i, colIdx, box);

			QString k = meta.key(fields.at(i));
			if (!k.isEmpty()) {
				box->setCurrentIndex(box->findText(k));
			}
		
			_editors << box;
			connect(box, SIGNAL(currentIndexChanged (const QString&)), this, SLOT(onComboSelected(const QString&)));
		}

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
