#include "gui/cvappcontainer.h"
#include "gui/cvtreewidget.h"
#include "gui/cvtreenode.h"
#include "gui/cvnodedetails.h"
#include "gui/cvmenubar.h"
#include "gui/cvtoolbar.h"
#include "gui/cvstatusbar.h"
#include "gui/helper/cvactionslinker.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>

namespace CV {
namespace GUI {

using namespace Status;

CVAppContainer::CVAppContainer(QWidget* parent) : QWidget(parent) {
	Helper::CVSignalHandle::create(parentWidget());
	Helper::CVActionHandle::create(parentWidget());

    _menu = new CVMenuBar(this);
    _toolbar = new CVToolBar(this);
    _status = new CVStatusBar(this);
    _tree = new CVTreeWidget(this);
    _details = new CVNodeDetails(this);

    QSplitter* split = new QSplitter(this);
    split->setChildrenCollapsible(false);
    split->setOpaqueResize(true);

    QVBoxLayout* box = new QVBoxLayout;
    box->setContentsMargins(2, 2, 2, 2);
    box->setSpacing(2);
    box->addWidget(split, 1);

    /*QTableWidget* table = new QTableWidget(this);
    //table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //TODO: QT5
    table->horizontalHeader()->setResizeMode(QHeaderView::Stretch); //TODO: QT5
    box->addWidget(table);*/
    setLayout(box);

    split->addWidget(_tree);
    split->addWidget(_details);
    split->setSizes(QList<int>() << _tree->minimumWidth() << _details->maximumWidth());
    split->setStretchFactor(0, 0);
    split->setStretchFactor(1, 1);

    //connect(_tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), _details, SLOT(onProjectItemActivated(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(controlAdded(CV::GUI::Status::CVNodeInfo::Type, Core::CVCategory*)), _details, SLOT(onControlAdded(CV::GUI::Status::CVNodeInfo::Type, Core::CVCategory*)));
    connect(&_prjManager, SIGNAL(addProject(Core::CVProject*)), this, SLOT(insertProject(Core::CVProject*)));

	Helper::CVSignalLinker* linker = Helper::CVSignalHandle::get();
	linker->add(Helper::ITEM_SELECTED, _tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)));
	linker->on(Helper::ITEM_SELECTED, _details, SLOT(onProjectItemActivated(QTreeWidgetItem*, int)));
}

void CVAppContainer::insertProject(Core::CVProject* proj) {
	Helper::CVActionsLinker* linker = Helper::CVActionHandle::get();
	linker->trig(Helper::CLOSE_PROJECT);

    CVNodeInfo* info = NULL;

	QString type;
	type = proj->type == Core::CVProject::PHOTOGRAMMETRY ? tr("Fotogrammetria") : tr("Lidar");
		
    // TODO: remove logic from here
	// TODO: use core enums
	CVTreeNode* root = _tree->insertProjectTree(type + ": " + proj->name);
    info = root->info();
    info->type(CVNodeInfo::PHOTOGRAMMETRY);
	emit controlAdded(info->type());

    CVTreeNode* node = NULL;
    node = _tree->insertNode(root, tr("Progetto di volo"));
    info = node->info();
    info->type(CVNodeInfo::FLY_PLAN);
    emit controlAdded(info->type(), proj->get(Core::CVCategory::PLAN));

    node = _tree->insertNode(root, tr("Dati GPS"));
    info = node->info();
    info->type(CVNodeInfo::GPS_DATA);
    emit controlAdded(info->type(), proj->get(Core::CVCategory::GPS_DATA));

    node = _tree->insertNode(root, tr("Volo effettuato"));
    info = node->info();
    info->type(CVNodeInfo::FLY);
    emit controlAdded(info->type(), proj->get(Core::CVCategory::FLY));

	
    node = _tree->insertNode(root, tr("Orto immagini"));
    info = node->info();
    info->type(CVNodeInfo::ORTO);
    emit controlAdded(info->type(), proj->get(Core::CVCategory::ORTO));
}

void CVAppContainer::link() {
    QMenu* projects = _menu->add(tr("Progetti"));

	Helper::CVActionsLinker* linker = Helper::CVActionHandle::get();

	QAction* newProj = linker->add(Helper::NEW_PROJECT);
    linker->on(Helper::NEW_PROJECT, &_prjManager, SLOT(onNewProject()));
	_addToMenuAndToolbar(newProj, projects, _toolbar, QIcon(""), tr("Nuovo"));
	
	QAction* loadProj = linker->add(Helper::LOAD_PROJECT);
    linker->on(Helper::LOAD_PROJECT, &_prjManager, SLOT(onLoadProject()));
	_addToMenuAndToolbar(loadProj, projects, _toolbar, QIcon(""), tr("Apri"));

	projects->addSeparator();

	QAction* closeProj = linker->add(Helper::CLOSE_PROJECT);
    linker->on(Helper::CLOSE_PROJECT, &_prjManager, SLOT(onCloseProject()));
    linker->on(Helper::CLOSE_PROJECT, _tree, SLOT(onCloseProject()));
    linker->on(Helper::CLOSE_PROJECT, _details, SLOT(onClear()));
	_addToMenuAndToolbar(closeProj, projects, _toolbar, QIcon(""), tr("Chiudi"));

	QAction* newMission = linker->add(Helper::NEW_MISSION);
    linker->on(Helper::NEW_MISSION, &_prjManager, SLOT(onCreateMission()));
}

void CVAppContainer::_addToMenuAndToolbar(QAction* a, QMenu* m, QToolBar* t, QIcon icon, QString name) {
	a->setText(name);
    a->setIcon(icon);

    m->insertAction(NULL, a);
    t->insertAction(NULL, a);

    QWidget* w = t->widgetForAction(a); //WORK_AROUND(TO FIX): icon only in toolbar
    static_cast<QPushButton*>(w)->setText("");
}

QMenuBar* CVAppContainer::menu() const { return _menu; }

QToolBar* CVAppContainer::toolbar() const { return _toolbar; }

QStatusBar* CVAppContainer::statusbar() const { return _status; }

} // namespace GUI
} // namespace CV
