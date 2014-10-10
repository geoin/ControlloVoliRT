#include "cvcsvinput.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>

namespace CV {
namespace Core {

CVCsvInput::CVCsvInput(QObject* p) : CVObject(p) { }

CVCsvInput::~CVCsvInput() { }

bool CVCsvInput::isValid() const {
	return _isValid;
}

bool CVCsvInput::remove() { 
	return false;
}

bool CVCsvInput::persist() {
	CsvParsingDialog p(NULL);
	p.setCsv(_csv);
	p.exec();
	return false;
}

bool CVCsvInput::load() {
	return false;
}

void CVCsvInput::setCsv(QString csv) {
	_csv = Csv::Ptr(new Csv(csv));
}

CsvParsingDialog::CsvParsingDialog(QWidget* p) : QDialog(p) {
	QFormLayout* gbox = new QFormLayout;

	QLineEdit* edit = new QLineEdit(this);
	edit->setMaxLength(1);
	gbox->addRow(tr("Separatore"), edit);
	
	connect(edit, SIGNAL(textChanged(const QString&)), this, SLOT(onSeparatorChanged(const QString&)));

	_header = new QCheckBox(this);
	gbox->addRow(tr("Intestazione"), _header);
	connect(_header, SIGNAL(stateChanged(int)), this, SLOT(onHeaderStateChanged(int)));

	QGroupBox* box = new QGroupBox(tr("Proprietà csv"), this);
	box->setLayout(gbox);

	_table = new QTableWidget(10, 10, this);
	
	QVBoxLayout* main = new QVBoxLayout;
	main->addWidget(box);
	main->addWidget(_table);

	setLayout(main);
}

void CsvParsingDialog::setCsv(Csv::Ptr csv) {
	_csv = csv;
	_tableFromCsv();
}

void CsvParsingDialog::onHeaderStateChanged(int) {
	_tableFromCsv();
}
	
void CsvParsingDialog::onSeparatorChanged(const QString& s) {
	if (s.size() == 0) {
		return;
	}
	_csv->setSeparator(s);
	_tableFromCsv();
}

void CsvParsingDialog::_tableFromCsv() {
	_table->clear();
	_table->setColumnCount(0);
	
	_csv->seek(0);

	_csv->open();
	if (_csv->atEnd()) {
		return;
	}

	QStringList out;
	_csv->splitAndFormat(_csv->readLine(), out);

	_table->setColumnCount(out.size());

	int i = 0;
	if (_header->isChecked()) {
		_table->setHorizontalHeaderLabels(out);
	} else {
		for (int j = 0; j < out.size(); j++) {
			QTableWidgetItem* item = new QTableWidgetItem(out.at(j));
			item->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			_table->setItem(i, j, item);
		}
		i++;
	}
	
	_table->setRowCount(4);
	
	for (int i = 0; i < _table->columnCount() - 1; ++i) {
		_table->horizontalHeader()->setResizeMode(i, QHeaderView::Stretch);
	}
	_table->horizontalHeader()->setStretchLastSection(true);

	for (; i < 4; i++) {
		if (_csv->atEnd()) {
			break;
		}

		_csv->splitAndFormat(_csv->readLine(), out);
		for (int j = 0; j < out.size(); j++) {
			QTableWidgetItem* item = new QTableWidgetItem(out.at(j));
			item->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			_table->setItem(i, j, item);
		}
	}
}

} // namespace Core
} // namespace CV
