#ifndef CVFINALDATADETAIL_P_H
#define CVFINALDATADETAIL_P_H

#include "core/categories/cvlidarfinalinput.h"

#include "cvbasedetail.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>
#include <QShowEvent>

class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVFinalDataDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVFinalDataDetail(QWidget *parent, Core::CVObject*);
	~CVFinalDataDetail();

	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(QStringList&);
	
	inline Core::CVLidarFinalInput* input() const { return static_cast<Core::CVLidarFinalInput*>(controller()); }

public slots:
	void finalFolder();

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
	QWidget* _addFolderPicker(QString, QString = "FOLDER");

    QScopedPointer<QFileInfo> _file;
	QString _uri;
	QList<QLabel*> _labels;

	QMap<QString, QWidget*> _data;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVFINALDATADETAIL_P_H
