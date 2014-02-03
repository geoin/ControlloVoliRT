#ifndef CV_GUI_DETAILS_CVMISSIONDETAIL_H
#define CV_GUI_DETAILS_CVMISSIONDETAIL_H

#include "gui/cvbasetabwidget.h"
#include "core/categories/cvcontrol.h"
#include "core/categories/cvmissionobject.h"

#include <QUuid>

namespace CV {
namespace GUI {
namespace Details {

class CVMissionDetail : public CVBaseTabWidget {
    Q_OBJECT
public:
    explicit CVMissionDetail(QWidget*, Core::CVMissionObject*, TabPosition = East);

    inline QString key() const { return _mission->id(); }

    inline QString name() const { return _mission->name(); }
    //inline void name(QString n) {  _name = n; }

signals:

public slots:

private:
    QString _key, _name;
	Core::CVMissionObject* _mission;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVMISSIONDETAIL_H
