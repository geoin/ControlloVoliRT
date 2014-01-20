#ifndef CV_GUI_CVSTATUSBAR_H
#define CV_GUI_CVSTATUSBAR_H

#include <QStatusBar>

namespace CV {
namespace GUI {

class CVStatusBar : public QStatusBar {
    Q_OBJECT
public:
    explicit CVStatusBar(QWidget *parent = 0);

signals:

public slots:

};

} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVSTATUSBAR_H
