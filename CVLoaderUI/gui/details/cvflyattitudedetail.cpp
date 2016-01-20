#include "cvflyattitudedetail.h"

#include "core/cvcore_utils.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMimeData>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QUrl>
#include <QDir>
#include <QDialogButtonBox>
#include <QTableWidget>

namespace CV {
namespace GUI {
namespace Details {

class AttitudeDialog : public QDialog {
public:
	AttitudeDialog(Core::CVFlyAttitude* ctrl, QWidget* p = NULL) : QDialog(p) {
		QVBoxLayout* l = new QVBoxLayout;
		setLayout(l);

		QFormLayout* form = new QFormLayout;
		l->addLayout(form);

		QComboBox* angle = new QComboBox(this);
		angle->addItem("DEG");
		angle->addItem("GON");
		form->addRow("Unità di misura", angle); 

		Core::CVFlyAttitude::Angle_t u = ctrl->angleUnit();
		angle->setCurrentIndex(int(u));
		
		//TODO: hide horizontal header
		QList<QStringList> lines = ctrl->readFirstLines(10);
		QTableWidget* wid = new QTableWidget(10, lines.size(), this);
		l->addWidget(wid, 2);

		for (int i = 0; i < lines.size(); i++) {
			const QStringList& line = lines.at(i);
			for (int j = 0; j < line.size(); j++) {
				QTableWidgetItem* itm = new QTableWidgetItem(line.at(j));
				wid->setItem(i, j, itm);
			}
		}

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
		connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
		connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
		l->addWidget(buttonBox);
	}
};

CVFlyAttitudeDetail::CVFlyAttitudeDetail(QWidget* p, Core::CVObject* l) : CVBaseDetail(p, l) {
	setAcceptDrops(true);

	title(tr("Assetti di volo"));
	description(tr("File degli assetti"));
	
    QFormLayout* form = new QFormLayout;

	QLabel* n = NULL;
	QLabel* info = NULL;

	createRow(this, tr("Numero strisciate"), n, info);
	_labels << info;
	form->addRow(n, info);
	
	createRow(this, tr("Numero fotogrammi"), n, info);
	_labels << info;
	form->addRow(n, info);

	body(form);

	if (controller()->isValid()) {
		QStringList info = layer()->data();
		for (int i = 0; i < info.size(); ++i) {
			QLabel* lab = _labels.at(i);
			lab->setText(info.at(i));
		}
	}

	connect(controller(), SIGNAL(persisted()), this, SLOT(onDataPersisted()));
	connect(controller(), SIGNAL(itemInserted(int)), this, SLOT(onItemInserted(int)));
}

CVFlyAttitudeDetail::~CVFlyAttitudeDetail() {

}

void CVFlyAttitudeDetail::importAll(QStringList& uri) {
	layer()->origin(uri.at(0));

	AttitudeDialog dialog(layer());
	dialog.exec();

	CV::GUI::CVScopedCursor cur;

	res = QtConcurrent::run(controller(), &CV::Core::CVObject::persist);

	_dialog.setWindowTitle(tr("Caricamento assetti in corso.."));
	_dialog.resize(260, 100);
	_dialog.resizeBarWidth(230);
	_dialog.exec();
}

void CVFlyAttitudeDetail::onItemInserted(int el) {
	_dialog.setLabelText(tr("Elementi inseriti") + ": " + QString::number(el));
}

void CVFlyAttitudeDetail::onDataPersisted() {
	if (res.result()) {
		QStringList& info = layer()->data();
		for (int i = 0; i < _labels.size(); ++i) {
			QLabel* lab = _labels.at(i);
			lab->setText(info.at(i));
		}
	}
	_dialog.setLabelText("");
	_dialog.close();
}

void CVFlyAttitudeDetail::searchFile() {
	QString uri = QFileDialog::getOpenFileName(
        this,
        tr("Importa assetti"),
		Core::CVSettings::get(CV_PATH_SEARCH).toString(),
        ""
    );
	if (!uri.isEmpty()) {
		QFileInfo info(uri);
		Core::CVSettings::set(CV_PATH_SEARCH, info.absolutePath());
		importAll(QStringList() << uri);
	}
}

void CVFlyAttitudeDetail::clearAll() {
	controller()->remove();
	for (int i = 0; i < _labels.size(); ++i) {
		QLabel* lab = _labels.at(i);
		lab->setText("");
	}
}

void CVFlyAttitudeDetail::dragEnterEvent(QDragEnterEvent* ev) {
    const QMimeData* mime = ev->mimeData();
    QList<QUrl> list = mime->urls();

    if (list.size() != 1) {
        ev->ignore();
    } else {
        QString uri = list.at(0).toLocalFile();
        _file.reset(new QFileInfo(uri));
		ev->accept();
    }
}

void CVFlyAttitudeDetail::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVFlyAttitudeDetail::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVFlyAttitudeDetail::dropEvent(QDropEvent* ev) {
    ev->accept();
	importAll(QStringList() << _file->absoluteFilePath());
    _file.reset(NULL);
}

} // namespace Details
} // namespace GUI
} // namespace CV
