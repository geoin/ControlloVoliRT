#ifndef CV_GUI_CVBASETABWIDGET_H
#define CV_GUI_CVBASETABWIDGET_H

#include <QTabWidget>

namespace CV {
namespace GUI {

class CVBaseTabWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit CVBaseTabWidget(QWidget* = 0, TabPosition = East);

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
};

} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVBASETABWIDGET_H
