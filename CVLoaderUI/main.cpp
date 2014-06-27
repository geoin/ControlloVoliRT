#include "gui/cvmainwindow.h"

#include <QApplication>
#include <QResource>
#include <QTextStream>
#include <QSysInfo>
//#include <QStyleFactory>

#include "core/cvcore_utils.h"

using namespace CV::GUI;

#include "cv_version.h"

#ifdef Q_WS_WIN
#include <QWindowsXPStyle>
#endif

int main(int argc, char *argv[]) {

#ifdef linux
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
#endif

#ifdef Q_WS_WIN 
	QString v(qVersion());
	if (QSysInfo::windowsVersion() > 0x0090) {
		QApplication::setStyle(new QWindowsXPStyle);
	}
#endif

    QApplication::setOrganizationName("Geoin");
    QApplication::setOrganizationDomain("CV");
    QApplication::setApplicationName("Controllo Voli");
	QApplication::setApplicationVersion(CHECK_VERSION);

    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

    //QApplication::setStyle(QStyleFactory::create("fusion"));
    QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":graphics/icons/logo.png"));

    CVMainWindow w;
	QResource style(":/style/style.qss");
	QFile res(style.absoluteFilePath());
	if (!res.open(QIODevice::ReadOnly | QIODevice::Text)) { 
		return false;
	}
	QTextStream str(&res);
	w.setStyleSheet(str.readAll());

	w.setWindowTitle(QString("Controllo Voli - ")  + CHECK_VERSION);

	bool maximized = CV::Core::CVSettings::get("/app/maximized").toBool();
	if (maximized) {
		w.showMaximized();
	} else {
		w.show();
	}

    int ret = app.exec();
    return ret;
}
