#include "cvstationdelegate.h"

#include <QVBoxLayout>
#include <QFormLayout>

#include <QMenu>

namespace CV {
namespace GUI {
namespace Details {

CVStationDelegate::CVStationDelegate(QWidget* p) : QWidget(p) {
	setContextMenuPolicy(Qt::DefaultContextMenu);

	QVBoxLayout* l = new QVBoxLayout;

	QWidget* body = new QWidget(this);
	QFormLayout* data = new QFormLayout;
	body->setLayout(data);

	_title = new QLabel(this);
    _title->setStyleSheet("padding: 4px; font: bold;");

	l->addWidget(_title);
	l->addWidget(body, 2);

	setLayout(l);
}

void CVStationDelegate::contextMenuEvent(QContextMenuEvent* ev) {
	QMenu menu;
	QAction* action = menu.addAction(tr("Rimuovi"));
	connect(action, SIGNAL(triggered()), this, SIGNAL(remove()));
	menu.move(ev->globalPos());
	menu.exec();

}

} // namespace Details
} // namespace GUI
} // namespace CV
