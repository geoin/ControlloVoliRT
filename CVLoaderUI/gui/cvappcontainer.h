#ifndef CV_GUI_CVAPPCONTAINER_H
#define CV_GUI_CVAPPCONTAINER_H

#include <QWidget>

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
    void controlAdded(CV::GUI::Status::CVNodeInfo::Type);

public slots:
	void insertPhotogrammetry(Core::CVProject*);

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
