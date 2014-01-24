#ifndef CV_GUI_DETAILS_CVMISSIONDETAIL_H
#define CV_GUI_DETAILS_CVMISSIONDETAIL_H

#include "gui/cvbasetabwidget.h"

#include <QUuid>

namespace CV {
namespace GUI {
namespace Details {

class CVMissionDetail : public CVBaseTabWidget {
    Q_OBJECT
public:
    explicit CVMissionDetail(QWidget* = 0, TabPosition = East);

    inline QString key() const { return _key; }

    inline QString name() const { return _name; }
    inline void name(QString n) {  _name = n; }

signals:

public slots:

private:
    QString _key, _name;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVMISSIONDETAIL_H
