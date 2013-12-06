
#ifndef DROPSITEWINDOW_H
#define DROPSITEWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QMimeData;
class QPushButton;
class QTreeWidget;
QT_END_NAMESPACE
class DropArea;

class DropSiteWindow : public QWidget {
    Q_OBJECT
public:
    DropSiteWindow();

public slots:
    void updateFormatsTable(const QMimeData *mimeData);

private:
    DropArea*	_drop_area;
    QTreeWidget* _prj_tree;

    QPushButton *clearButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
};


#endif
