#include "cvcsvinput.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include "core/cvcore_utils.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>

#include <QGroupBox>
#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>

#define CV_TAB_SPACE_PH "TAB/SPACE"

namespace CV {
namespace Core {

CVCsvInput::CVCsvInput(QObject* p) : CVObject(p) { }

CVCsvInput::~CVCsvInput() { }

bool CVCsvInput::isValid() const {
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	bool ret = cnn.is_valid();
	if (!ret) {
		return false;
	}
	
	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	CV::Util::Spatialite::Recordset set = q->select(
		QStringList() << "count(*)",
		QStringList() << "CONTROL_POINTS", 
		QStringList(),
		QVariantList()
	);

	int cnt = 0;
	
	if (!set.eof()) {
		cnt = set[0].toInt();
	}

	return cnt != 0;
}

bool CVCsvInput::remove() { 
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	bool ret = cnn.is_valid();
	if (!ret) {
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	q->remove("CONTROL_POINTS", QStringList(), QVariantList());
	return false;
}

bool CVCsvInput::persist() {
	remove();

	CsvParsingDialog p(NULL);
	p.setCsv(_csv);
	int ret = p.exec();
	if (ret == QDialog::Accepted) {
		int x = p.x();
		int y = p.y();
		int z = p.z();
		int name = p.name();

		_csv->seek(0);
		if (p.hasHeader()) {
			_csv->readLine();
		}

		CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
		bool ret = cnn.is_valid();
		if (!ret) {
			return false;
		}

		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		cnn.begin_transaction();

		while (!_csv->atEnd()) {
			QString line = _csv->readLine();
			QStringList out;
			_csv->splitAndFormat(line, out);
			
			bool ret = q->insert(
				"CONTROL_POINTS", 
				QStringList() << "X" << "Y" << "Z" << "NAME",
				QStringList() << "?1" << "?2" << "?3" << "?4",
				QVariantList() << out.at(x - 1).toDouble() << out.at(y - 1).toDouble() << out.at(z - 1).toDouble() << (name != 0 ? out.at(name - 1) : "")
			);
		}

		cnn.commit_transaction();
		return true;
	} else {
		return load();
	}
}

bool CVCsvInput::load() {
	_samples.clear();

	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	bool ret = cnn.is_valid();
	if (!ret) {
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	CV::Util::Spatialite::Recordset set = q->select(
		QStringList() << "X" << "Y" << "Z" << "NAME",
		QStringList() << "CONTROL_POINTS",
		QStringList(), QVariantList(), QStringList(),
		10
	);

	while (!set.eof()) {
		double x = set["X"].toDouble(), y = set["Y"].toDouble(), z = set["Z"].toDouble();
		QString name(set["NAME"].toString().c_str());

		_samples << (QStringList() << QString::number(x, 'f', 4) << QString::number(y, 'f', 4) << QString::number(z, 'f', 4) << name);

		set.next();
	}
		
	return _samples.size();
}

void CVCsvInput::setCsv(QString csv) {
	_csv = Csv::Ptr(new Csv(csv));
}

CsvParsingDialog::CsvParsingDialog(QWidget* p) : QDialog(p) {
	QFormLayout* gbox = new QFormLayout;
	
	QString val = CVSettings::get(CV_CSV_SEPARATOR, CV_TAB_SPACE_PH).toString();
	
	_edit = new QComboBox(this);
	_edit->addItems(QStringList() << "," << ";" << "|" << CV_TAB_SPACE_PH);
	_edit->setCurrentIndex(_edit->findText(val));

	_header = new QCheckBox(this);
	
	int v = CVSettings::get(CV_CSV_HEADER).toInt();
	_header->setChecked(v != 0);
	
	gbox->addRow(tr("Separatore"), _edit);
	gbox->addRow(tr("Intestazione"), _header);
	gbox->setHorizontalSpacing(52);
	
	QGroupBox* box = new QGroupBox(tr("Proprietà csv"), this);
	box->setLayout(gbox);

	_table = new QTableWidget(10, 10, this);

	
	gbox = new QFormLayout;
	_x =  new QComboBox(this);
	_y =  new QComboBox(this);
	_z =  new QComboBox(this);
	_name =  new QComboBox(this);

	gbox->addRow(tr("X"), _x);
	gbox->addRow(tr("Y"), _y);
	gbox->addRow(tr("Z"), _z);
	gbox->addRow(tr("Nome"), _name);
	gbox->setHorizontalSpacing(64);

	QGroupBox* info = new QGroupBox(tr("Colonne di riferimento"), this);
	info->setLayout(gbox);
	
	QVBoxLayout* main = new QVBoxLayout;
	main->addWidget(box);
	main->addWidget(_table);
	main->addWidget(info);

	QPushButton* save = new QPushButton("Salva", this);
	save->setFixedWidth(96);
	main->addWidget(save);

	setLayout(main);

	resize(500, 500);

	connect(_edit, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onSeparatorChanged(const QString&)));
	connect(_header, SIGNAL(stateChanged(int)), this, SLOT(onHeaderStateChanged(int)));
	connect(save, SIGNAL(pressed()), this, SLOT(onSave()));
}

void CsvParsingDialog::setCsv(Csv::Ptr csv) {
	_csv = csv;

	onSeparatorChanged(_edit->currentText());
	_tableFromCsv();
}

void CsvParsingDialog::onHeaderStateChanged(int i) {
	CVSettings::set(CV_CSV_HEADER, i);
	_tableFromCsv();
}
	
void CsvParsingDialog::onSeparatorChanged(const QString& s) {
	if (s.size() == 0) {
		return;
	}
	
	CVSettings::set(CV_CSV_SEPARATOR, s);

	if (s == QString(CV_TAB_SPACE_PH)) {
		_csv->setSeparator("[\\t*\\s*]");
	} else {
		_csv->setSeparator(s);
	}
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

	QStringList sel;
	sel << "";

	_table->setColumnCount(out.size());

	int i = 0;
	if (_header->isChecked()) {
		_table->setHorizontalHeaderLabels(out);

		for (int k = 0; k < out.size(); k++) {
			sel << out.at(k);
		}

	} else {
		for (int j = 0; j < out.size(); j++) {
			QTableWidgetItem* item = new QTableWidgetItem(out.at(j));
			item->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			_table->setItem(i, j, item);
		}
		i++;

		for (int k = 0; k < out.size(); k++) {
			sel << QString::number(k + 1);
		}
	}

	_x->clear();
	_x->addItems(sel);
	
	_y->clear();
	_y->addItems(sel);
	
	_z->clear();
	_z->addItems(sel);

	_name->clear();
	_name->addItems(sel);
	
	int rows = 10;
	_table->setRowCount(rows);

	_table->verticalHeader()->hide();
	_table->horizontalHeader()->setStretchLastSection(true);

	for (; i < rows; i++) {
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

void CsvParsingDialog::onSave() {
	int xVal = x();
	int yVal = y();
	int zVal = z();
	if (xVal != 0 && yVal != 0 && zVal != 0) {
		if (xVal != yVal && xVal != zVal && yVal != zVal) {
			accept();
		}
	} 
}

} // namespace Core
} // namespace CV
