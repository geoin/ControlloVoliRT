#ifndef CVSTATION_DETAILS_P_H
#define CVSTATION_DETAILS_P_H

#include "cvbasedetail.h"
#include "cvstationdelegate.h"
#include "core/categories/cvstations.h"

#include <QListWidget>
#include <QScopedPointer>
#include <QSet>

#include <assert.h>

class QLabel;
class QListWidget;

namespace CV {
namespace GUI {
namespace Details {

class CVStationListItem : public QObject, public QListWidgetItem {
	Q_OBJECT
public:
	CVStationListItem(QListWidget* p, QWidget* d) : QListWidgetItem(p), _delegate(d) {
		assert(d != NULL);
		assert(p != NULL);

		connect(_delegate, SIGNAL(remove()), this, SLOT(onRemove()));
	}

public slots:
	void onRemove() {
		QListWidget* list = listWidget();
		int row = list->row(this);
		emit removeStation(row);
		QScopedPointer<QListWidgetItem>(list->takeItem(row));
	}

signals:
	void removeStation(int);

private:
	QWidget* _delegate;
};

class CVStationsDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVStationsDetail(QWidget *parent, Core::CVObject*);
	~CVStationsDetail();

	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(QStringList&);

	CVStationDelegate* addItem(const QString&);

	inline Core::CVStations* stations() const { return static_cast<Core::CVStations*>(controller()); }

public slots:
	void onStationSelected(int);
	void onRemoveStation(int);

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
	QString _base, _station;
    QStringList _files;

	QListWidget* _stations, * _details;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVSTATION_DETAILS_P_H
