#ifndef CVCSVINPUT_H
#define CVCSVINPUT_H

#include "core/categories/cvcontrol.h"
#include "core/cvcore_utils.h"

#include <QStringList>
#include <QMap>
#include <QDialog>
#include <QCheckBox>
#include <QComboBox>

class QTableWidget;

namespace CV {
namespace Core {

class CVCsvInput : public CVObject {
	Q_OBJECT

public:
	CVCsvInput(QObject *parent);
	virtual ~CVCsvInput();

	void setTable(QString t) { _table = t; }

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();	
	virtual bool remove();

	void setCsv(QString);

	const QList<QStringList>& data() {
		return _samples;
	}

private:
	Csv::Ptr _csv;
	QList<QStringList> _samples;
	
	QString _table;
};

class CsvParsingDialog : public QDialog {
	Q_OBJECT
public:
	CsvParsingDialog(QWidget* p);

	void setCsv(Csv::Ptr);

	bool hasHeader() const { return _header->isChecked(); }
	
	int x() const { return _x->currentIndex(); }
	int y() const { return _y->currentIndex(); }
	int z() const { return _z->currentIndex(); }
	int name() const { return _name->currentIndex(); }

public slots:
	void onSave();
	void onHeaderStateChanged(int);
	void onSeparatorChanged(const QString&);

private:
	void _tableFromCsv();

	QComboBox* _x, *_y, *_z, *_name;
	QComboBox* _edit;
	QCheckBox* _header;
	QTableWidget* _table;

	Csv::Ptr _csv;
};

} // namespace Core
} // namespace CV

#endif // CVCSVINPUT_H
