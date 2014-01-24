#ifndef CV_GUI_CVTREENODEDELEGATE_H
#define CV_GUI_CVTREENODEDELEGATE_H

#include <QWidget>

namespace CV {
namespace GUI {

class CVTreeNodeDelegate : public QWidget {
    Q_OBJECT
public:
    explicit CVTreeNodeDelegate(QWidget* p, const QStringList& = QStringList());
    explicit CVTreeNodeDelegate(QWidget* p, QWidget* view);

signals:

public slots:

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
    void _addWidget(QWidget*);

};

} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVTREENODEDELEGATE_H
