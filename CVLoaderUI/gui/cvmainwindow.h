#ifndef CVMAINWINDOW_H
#define CVMAINWINDOW_H

#include <QMainWindow>


namespace CV {
namespace GUI {

	class CVAppContainer;

class CVMainWindow : public QMainWindow {
    Q_OBJECT
public:
    CVMainWindow(QWidget *parent = 0);
    ~CVMainWindow();

protected:
	virtual void closeEvent(QCloseEvent*);

private:
	CVAppContainer* app;
};

}
}

#endif // MAINWINDOW_H
