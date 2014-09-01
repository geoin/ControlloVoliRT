#ifndef CVSENSORDETAIL_H
#define CVSENSORDETAIL_H

#include "core/categories/cvsensor.h"

#include "cvbasedetail.h"

class QLineEdit;

namespace CV {
namespace GUI {
namespace Details {

class CVSensorDetail : public CVBaseDetail {
	Q_OBJECT

public:
	explicit CVSensorDetail(QWidget *parent, Core::CVObject*);
	virtual ~CVSensorDetail();

	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(QStringList&);

	inline Core::CVSensor* sensor() const { return static_cast<Core::CVSensor*>(controller()); }

	void save();
	void view();

public slots:
	void edit();
	void saveFormData();
	void onQuit();

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
	QString _keyToFieldName(const QString& key);

    QMap<QString, QLineEdit*> _params;
    QString _uri;

	bool _isEditing;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVSENSORDETAIL_H
