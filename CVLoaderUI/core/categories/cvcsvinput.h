#ifndef CVCSVINPUT_H
#define CVCSVINPUT_H

#include "core/categories/cvcontrol.h"
#include "core/cvcore_utils.h"

#include <QStringList>
#include <QMap>
#include <QDialog>
#include <QCheckBox>

class QTableWidget;

namespace CV {
namespace Core {

class CVCsvInput : public CVObject {
	Q_OBJECT

public:
	CVCsvInput(QObject *parent);
	virtual ~CVCsvInput();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();	
	virtual bool remove();

	void setCsv(QString);

private:
	Csv::Ptr _csv;
};

class CsvParsingDialog : public QDialog {
	Q_OBJECT
public:
	CsvParsingDialog(QWidget* p);

	void setCsv(Csv::Ptr);

public slots:
	void onHeaderStateChanged(int);
	void onSeparatorChanged(const QString&);

private:
	void _tableFromCsv();

	QCheckBox* _header;
	QTableWidget* _table;

	Csv::Ptr _csv;
};

} // namespace Core
} // namespace CV

#endif // CVCSVINPUT_H
