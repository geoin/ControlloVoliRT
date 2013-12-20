#include "gui/cvappcontainer.h"
#include "gui/cvtreewidget.h"
#include "gui/cvnodedetails.h"

#include <QHBoxLayout>
#include <QSplitter>

namespace CV {
namespace GUI {

CVAppContainer::CVAppContainer(QWidget *parent) : QWidget(parent) {
    auto split = new QSplitter(this);
    split->setChildrenCollapsible(false);
    split->setOpaqueResize(true);

    split->addWidget(new CVTreeWidget(this));
    split->addWidget(new CVNodeDetails(this));

    auto box = new QHBoxLayout;
    box->setContentsMargins(0, 0, 0, 0);
    box->addWidget(split);
    setLayout(box);

    connect(split->widget(0), SIGNAL(itemClicked(QTreeWidgetItem*, int)), split->widget(1), SLOT(onProjectItemActivated(QTreeWidgetItem*, int)));
}

} // namespace GUI
} // namespace CV
