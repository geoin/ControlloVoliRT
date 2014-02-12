#include "gui/cvmainwindow.h"

#include <QApplication>
#include <QResource>
#include <QTextStream>
#include <QSysInfo>
//#include <QStyleFactory>

#include "core/cvcore_utils.h"

using namespace CV::GUI;

#ifdef Q_WS_WIN
#include <QWindowsXPStyle>
#endif

int main(int argc, char *argv[]) {

#ifdef Q_WS_WIN 
	QString v(qVersion());
	if (QSysInfo::windowsVersion() > 0x0090) {
		QApplication::setStyle(new QWindowsXPStyle);
	}
#endif

    QApplication::setOrganizationName("Geoin");
    QApplication::setOrganizationDomain("CV");
    QApplication::setApplicationName("Controllo Voli");
    QApplication::setApplicationVersion("0.0.1");

    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

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
