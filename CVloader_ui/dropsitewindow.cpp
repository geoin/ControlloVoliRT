
#include <QtGui>

#include "droparea.h"
#include "dropsitewindow.h"

//! [constructor part1]
DropSiteWindow::DropSiteWindow()
{
    _drop_area = new DropArea;
    connect(_drop_area, SIGNAL(changed(const QMimeData*)),
            this, SLOT(updateFormatsTable(const QMimeData*)));

    QStringList labels;
    labels << tr("Format") << tr("Content");

    _prj_tree = new QTreeWidget;
	QTreeWidgetItem* qt = new QTreeWidgetItem;
	qt->setText(0, "Progetto");
	_prj_tree->addTopLevelItem(qt);

	QTreeWidgetItem* q1 = new QTreeWidgetItem;
	q1->setText(0, "Fotogrammetria");

	QList<QTreeWidgetItem*> qcl;
	QTreeWidgetItem* q2 = new QTreeWidgetItem;
	q2->setText(0, "Assi di volo progettati");
	qcl.push_back(q2);
	q2 = new QTreeWidgetItem;
	q2->setText(0, "Assi di volo");
	qcl.push_back(q2);
	q2 = new QTreeWidgetItem;
	q2->setText(0, "Aree da cartografare");
	qcl.push_back(q2);
	
	q1->addChildren(qcl);

	qt->addChild(q1);
	
	q1 = new QTreeWidgetItem;
	q1->setText(0, "Lidar");
	qt->addChild(q1);

    //formatsTable->setColumnCount(2);
    //formatsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //formatsTable->setHorizontalHeaderLabels(labels);
    //formatsTable->horizontalHeader()->setStretchLastSection(true);
//! [constructor part3]

//! [constructor part4]
    clearButton = new QPushButton(tr("Clear"));
    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(clearButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    connect(quitButton, SIGNAL(pressed()), this, SLOT(close()));
    connect(clearButton, SIGNAL(pressed()), _drop_area, SLOT(clear()));

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(_prj_tree);
    hl->addWidget(_drop_area);

    QVBoxLayout *vl = new QVBoxLayout;
	vl->addLayout(hl);
	vl->addWidget(buttonBox);
    setLayout(vl);

    setWindowTitle(tr("Drop Site"));
    setMinimumSize(350, 500);
}

//! [updateFormatsTable() part1]
void DropSiteWindow::updateFormatsTable(const QMimeData *mimeData)
{
//    formatsTable->setRowCount(0);
//    if (!mimeData)
//        return;
////! [updateFormatsTable() part1]
//
////! [updateFormatsTable() part2]        
//    foreach (QString format, mimeData->formats()) {
//        QTableWidgetItem *formatItem = new QTableWidgetItem(format);
//        formatItem->setFlags(Qt::ItemIsEnabled);
//        formatItem->setTextAlignment(Qt::AlignTop | Qt::AlignLeft);
////! [updateFormatsTable() part2]
//
////! [updateFormatsTable() part3]
//        QString text;
//        if (format == "text/plain") {
//            text = mimeData->text().simplified();
//        } else if (format == "text/html") {
//            text = mimeData->html().simplified();
//        } else if (format == "text/uri-list") {
//            QList<QUrl> urlList = mimeData->urls();
//            for (int i = 0; i < urlList.size() && i < 32; ++i)
//                text.append(urlList[i].toString() + " ");
//        } else {
//            QByteArray data = mimeData->data(format);
//            for (int i = 0; i < data.size() && i < 32; ++i) {
//                QString hex = QString("%1").arg(uchar(data[i]), 2, 16,
//                                                QChar('0'))
//                                           .toUpper();
//                text.append(hex + " ");
//            }
//        }
//
//		int row = formatsTable->rowCount();
//        formatsTable->insertRow(row);
//        formatsTable->setItem(row, 0, new QTableWidgetItem(format));
//        formatsTable->setItem(row, 1, new QTableWidgetItem(text));
//    }
//    
//    formatsTable->resizeColumnToContents(0);
}

