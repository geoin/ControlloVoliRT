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

    /*QTableWidget* table = new QTableWidget(10, 4, this);
    //table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //TODO: QT5
    table->horizontalHeader()->setResizeMode(QHeaderView::Stretch); 
	table->setHorizontalHeaderLabels(QStringList()<< tr("Data") << tr("Note") << tr("Controllo") << tr("Oggetto"));
	table->verticalHeader()->hide();
    box->addWidget(table);*/

    setLayout(box);

    split->addWidget(_tree);
    split->addWidget(_details);
    split->setSizes(QList<int>() << _tree->minimumWidth() << _details->maximumWidth());
    split->setStretchFactor(0, 0);
    split->setStretchFactor(1, 1);

    //connect(_tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), _details, SLOT(onProjectItemActivated(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(controlAdded(CV::Core::CVControl::Type, Core::CVControl*)), _details, SLOT(onControlAdded(CV::Core::CVControl::Type, Core::CVControl*)));
    connect(&_prjManager, SIGNAL(addProject(Core::CVProject*)), this, SLOT(insertProject(Core::CVProject*)));

	Helper::CVSignalLinker* linker = Helper::CVSignalHandle::get();
	linker->add(Helper::ITEM_SELECTED, _tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)));
	linker->on(Helper::ITEM_SELECTED, _details, SLOT(onProjectItemActivated(QTreeWidgetItem*, int)));
}

void CVAppContainer::insertProject(Core::CVProject* proj) {
	Helper::CVActionsLinker* linker = Helper::CVActionHandle::get();
	linker->trig(Helper::CLOSE_PROJECT);
		
	CVTreeNode* root = _tree->insertProjectTree(proj->name, Core::CVProject::PHOTOGRAMMETRY);
	emit controlAdded(root->info()->type());

    CVTreeNode* node = NULL;
    node = _tree->insertNode(root, tr("Progetto di volo"), Core::CVControl::PLAN);
    emit controlAdded(node->info()->type(), proj->get(Core::CVControl::PLAN));

    node = _tree->insertNode(root, tr("Dati GPS"), Core::CVControl::GPS_DATA);
    emit controlAdded(node->info()->type(), proj->get(Core::CVControl::GPS_DATA));

    node = _tree->insertNode(root, tr("Volo effettuato"), Core::CVControl::FLY);
    emit controlAdded(node->info()->type(), proj->get(Core::CVControl::FLY));

	
    node = _tree->insertNode(root, tr("Orto immagini"), Core::CVControl::ORTO);
    emit controlAdded(node->info()->type(), proj->get(Core::CVControl::ORTO));
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
