#ifndef CVMAINWINDOW_H
#define CVMAINWINDOW_H

#include <QMainWindow>

namespace CV {
namespace GUI {

class CVMainWindow : public QMainWindow {
    Q_OBJECT
public:
    CVMainWindow(QWidget *parent = 0);
    ~CVMainWindow();
};

}
}

#endif // MAINWINDOW_H
