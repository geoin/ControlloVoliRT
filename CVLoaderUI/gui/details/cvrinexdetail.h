#ifndef CV_GUI_DETAILS_CVRINEXDETAIL_H
#define CV_GUI_DETAILS_CVRINEXDETAIL_H

#include "cvbasedetail.h"
#include "core/categories/cvrinex.h"
#include "gui/cvgui_utils.h"

#include <QtConcurrentRun>
#include <QFuture>

class QListWidget;
class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVRinexDetail : public CVBaseDetail {
    Q_OBJECT
public:
	explicit CVRinexDetail(QWidget *parent, Core::CVObject*);

	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(const QStringList&);

	inline Core::CVRinex* rinex() const { return static_cast<Core::CVRinex*>(controller()); }

signals:
	void persisted();
	void updateStatus(QString);
	void importQueued(const QStringList&);

public slots:
	void onDataPersisted();
	void onUpdateStatus(QString);

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
	bool _importAllAsync(Core::CVScopedTmpDir&, QStringList&);

	QLabel* _name;
	QListWidget* _details;
	
	QString _base, _station;
    QStringList _files;
	
	CVProgressDialog _dialog;
	QFuture<bool> res;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVRINEXDETAIL_H
