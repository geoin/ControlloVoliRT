#ifndef CV_GUI_CVAPPCONTAINER_H
#define CV_GUI_CVAPPCONTAINER_H

#include <QWidget>
#include <QApplication>

#include "core/cvprojectmanager.h"

#include "status/cvnodeinfo.h"

class QMenuBar;
class QToolBar;
class QStatusBar;
class QMenu;
class QIcon;

namespace CV {
namespace GUI {

class CVMenuBar;
class CVToolBar;
class CVStatusBar;
class CVTreeWidget;
class CVNodeDetails;

class CVAppContainer : public QWidget {
    Q_OBJECT
public:
    explicit CVAppContainer(QWidget *parent = 0);

    void link();

    QMenuBar* menu() const;
    QToolBar* toolbar() const;
    QStatusBar* statusbar() const;

signals:
    void controlAdded(CV::Core::CVControl::Type, Core::CVControl* = NULL);

public slots:
	void insertProject(Core::CVProject*);

private:
	void _addToMenuAndToolbar(QAction*, QMenu*, QToolBar*, QIcon icon, QString name = QString());

    CVTreeWidget* _tree;
    CVNodeDetails* _details;
    CVStatusBar* _status;
    CVMenuBar* _menu;
    CVToolBar* _toolbar;

    Core::CVProjectManager _prjManager;
};

} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVAPPCONTAINER_H
