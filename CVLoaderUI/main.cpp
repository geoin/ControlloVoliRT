#include "gui/cvmainwindow.h"

#include <QApplication>
#include <QStyleFactory>

using namespace CV::GUI;

int main(int argc, char *argv[]) {

    QApplication::setOrganizationName("Geoin");
    QApplication::setOrganizationDomain("CV");
    QApplication::setApplicationName("Controllo Voli");
    QApplication::setApplicationVersion("0.0.1");

    QApplication::setStyle(QStyleFactory::create("fusion"));
    QApplication app(argc, argv);
    QApplication::setLibraryPaths(QStringList());

    CVMainWindow w;
    w.show();

    int ret = app.exec();
    return ret;
}
