#include "gui/cvmainwindow.h"

#include <QApplication>
#include <QResource>
#include <QTextStream>
//#include <QStyleFactory>

#include "core/cvcore_utils.h"

using namespace CV::GUI;

int main(int argc, char *argv[]) {

    QApplication::setOrganizationName("Geoin");
    QApplication::setOrganizationDomain("CV");
    QApplication::setApplicationName("Controllo Voli");
    QApplication::setApplicationVersion("0.0.1");

    //QApplication::setStyle(QStyleFactory::create("fusion"));
    QApplication app(argc, argv);

    CVMainWindow w;
	QResource style(":/style/style.qss");
	QFile res(style.absoluteFilePath());
	if (!res.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}
	QTextStream str(&res);
	w.setStyleSheet(str.readAll());

	w.setWindowTitle("Controllo Voli");
    w.show();

    int ret = app.exec();
    return ret;
}
