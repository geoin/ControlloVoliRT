#include "missionlistdetails.h"
#include "cvmissiondetail.h"

#include "gui/dialogs/cvmissiondialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QMenu>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>

#include <assert.h>

namespace CV {
namespace GUI {
namespace Details {

CVMissionListToolbar::CVMissionListToolbar(QWidget* p) : QWidget(p) {
   _back = new QPushButton("<", this);
   _back->setMaximumSize(28, 28);
   _back->setDisabled(true);
   connect(_back, SIGNAL(pressed()), this, SIGNAL(viewPrevious()));

   _next = new QPushButton(">", this);
   _next->setMaximumSize(28, 28);
   _next->setDisabled(true);
   connect(_next, SIGNAL(pressed()), this, SIGNAL(viewNext()));

   QMenu* menu = new QMenu(this);
   _menu = new QPushButton(tr(""), this);
   _menu->setMaximumSize(20, 28);
   _menu->setMenu(menu);

   _title = new QLabel(tr("Nessuna missione attiva"), this);

   QHBoxLayout* box = new QHBoxLayout;
   box->setContentsMargins(0, 0, 0, 0);

   box->addSpacing(4);
   box->addWidget(_title);
   box->addStretch(2);
   box->addWidget(_back);
   box->addWidget(_next);
   box->addWidget(_menu);
   box->addSpacing(4);

   setLayout(box);
}

QMenu* CVMissionListToolbar::menu() const {
    return _menu->menu();
}

void CVMissionListToolbar::title(const QString& t) {
    _title->setText(t);
}

QString CVMissionListToolbar::title() const {
    return _title->text();
}

CVMissionListBody::CVMissionListBody(QWidget* p) : QWidget(p) {
    QStackedLayout* box = new QStackedLayout;
    setLayout(box);
}

void CVMissionListDetails::onPreviousMission() {
    QStackedLayout* stack = static_cast<QStackedLayout*>(_body->layout());

    int index = stack->currentIndex();
    stack->setCurrentIndex(!index ? stack->count() - 1 : (--index % stack->count()));

    CVMissionDetail* mission = static_cast<CVMissionDetail*>(stack->currentWidget());
    _bar->title(mission->name());
}

void CVMissionListDetails::onNextMission() {
    QStackedLayout* stack = static_cast<QStackedLayout*>(_body->layout());

    int index = stack->currentIndex();
    stack->setCurrentIndex(++index % stack->count());

    CVMissionDetail* mission = static_cast<CVMissionDetail*>(stack->currentWidget());
    _bar->title(mission->name());
}

CVMissionListDetails::CVMissionListDetails(QWidget* p, CV::Core::CVCategory* cat) : QWidget(p) {
	assert(cat != NULL);
	_category = cat;

   _bar = new CVMissionListToolbar(this);
   _body = new CVMissionListBody(this);

   connect(_bar, SIGNAL(viewPrevious()), this, SLOT(onPreviousMission()));
   connect(_bar, SIGNAL(viewNext()), this, SLOT(onNextMission()));

   //TODO Action 
   QMenu* menu = _bar->menu();
   QAction* create = menu->addAction(QIcon(""), tr("Crea missione"));
   connect(create, SIGNAL(triggered()), this, SLOT(onAddMission()));
   QAction* remove = menu->addAction(QIcon(""), tr("Rimuovi missione"));
   connect(remove, SIGNAL(triggered()), this, SLOT(onRemoveMission()));
   menu->addSeparator();

   QVBoxLayout* box = new QVBoxLayout;
   box->addWidget(_bar);
   box->addWidget(_body, 2);
   box->setContentsMargins(0, 0, 0, 0);

   setLayout(box);
}

void CVMissionListDetails::onAddMission() {
    Dialogs::CVMissionDialog dialog(_body);
    int ret = dialog.exec();
    if (ret != QDialog::Accepted) {
        return;
    }
	
    QString name, note;
    dialog.getInput(name, note);
	//TODO note;

    CVMissionDetail* item = new CVMissionDetail(this);
    item->name(name);

    QStackedLayout* stack = static_cast<QStackedLayout*>(_body->layout());
    stack->addWidget(item);
    stack->setCurrentIndex(stack->count() - 1);

    QMenu* menu = _bar->menu();
    QAction* mission = menu->addAction(QIcon(""), item->name());
    mission->setData(item->key());

	connect(mission, SIGNAL(triggered()), this, SLOT(onMissionChange()));

    _bar->title(item->name());
    _bar->previous()->setEnabled(stack->count() > 1);
    _bar->next()->setEnabled(stack->count() > 1);
}

void CVMissionListDetails::onMissionChange() {
	QAction* origin = static_cast<QAction*>(sender());
	QString key = origin->data().toString();
	
    QStackedLayout* stack = static_cast<QStackedLayout*>(_body->layout());
	for (int i = 0; i < stack->count(); ++i) {
		CVMissionDetail* m = static_cast<CVMissionDetail*>(stack->widget(i));
		if (m->key() == key) {
			stack->setCurrentWidget(m);
			_bar->title(m->name());
			return;
		}
	}
}

void CVMissionListDetails::onRemoveMission() {
    QStackedLayout* stack = static_cast<QStackedLayout*>(_body->layout());
    QWidget* actual = stack->currentWidget();
    if (actual) {
        CVMissionDetail* w = static_cast<CVMissionDetail*>(actual);
        QString key = w->key();
        stack->removeWidget(w);
        w->deleteLater();

        QList<QAction*> actions = _bar->menu()->actions();
        foreach (QAction* action, actions) {
            QString data = action->data().toString();
            if (data == key) {
                _bar->menu()->removeAction(action);
                break;
            }
        }
    }

    if (stack->currentWidget() == NULL) {
        _bar->title(tr("Nessuna missione attiva"));
    } else {
        CVMissionDetail* mission = static_cast<CVMissionDetail*>(stack->currentWidget());
        _bar->title(mission->name());
    }

    _bar->previous()->setDisabled(stack->count() <= 1);
    _bar->next()->setDisabled(stack->count() <= 1);
}

} // namespace Details
} // namespace GUI
} // namespace CV
