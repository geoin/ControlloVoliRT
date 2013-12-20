#include "gui/cvmainwindow.h"

#include <QApplication>

using namespace CV::GUI;

int main(int argc, char *argv[]) {
    QApplication::setOrganizationName("Geoin");
    QApplication::setOrganizationDomain("CV");
    QApplication::setApplicationName("Controllo Voli");
    QApplication::setApplicationVersion("0.0.1");

    QApplication a(argc, argv);
    CVMainWindow w;
    w.show();

    return a.exec();
}
