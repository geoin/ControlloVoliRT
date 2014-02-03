#ifndef CVTREEWIDGET_H
#define CVTREEWIDGET_H

#include <QTreeWidget>

#include "core/cvproject.h"
#include "core/categories/cvcontrol.h"

namespace CV {
namespace GUI {

class CVTreeNode;

class CVTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit CVTreeWidget(QWidget *parent = 0);

	CVTreeNode* insertProjectTree(const QString&, Core::CVProject::Type);
	CVTreeNode* insertNode(CVTreeNode* parent, const QString&, Core::CVControl::Type);

public slots:
	void onCloseProject();
};

}
}

#endif // CV_GUI_CVTREEWIDGET_H
