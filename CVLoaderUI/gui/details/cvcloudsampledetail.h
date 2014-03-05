#ifndef CV_CLOUD_SAMPLE_DETAIL_H
#define CV_CLOUD_SAMPLE_DETAIL_H

#include "core/categories/cvfileinput.h"

#include "cvbasedetail.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>

class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVCloudSampleDetail : public CVBaseDetail {
	Q_OBJECT

public:
	CVCloudSampleDetail(QWidget *parent, Core::CVObject*);
	~CVCloudSampleDetail();

	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(QStringList&);
	
	inline Core::CVFileInput* input() const { return static_cast<Core::CVFileInput*>(controller()); }

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
    QScopedPointer<QFileInfo> _file;
	QString _uri;
	QList<QLabel*> _labels;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_CLOUD_SAMPLE_DETAIL_H
