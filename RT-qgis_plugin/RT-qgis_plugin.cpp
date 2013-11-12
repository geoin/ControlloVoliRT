
#include "RT-qgis_plugin.h"
#include "qgisinterface.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <qgsvectorfilewriter.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QPushButton>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QMenu>
//#include "Poco/Process.h"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

dbox::dbox(QWidget* parent)
{
	setWindowTitle("GEOIN");
	QPushButton* bok = new QPushButton;
	bok->setText("OK");
	connect(bok, SIGNAL(clicked(bool)), this, SLOT(esegui(bool)));

	QVBoxLayout* qvb = new QVBoxLayout;
	qvb->addWidget(bok);
	setLayout(qvb);
}
void dbox::esegui(bool b)
{
	QMessageBox::information(0, tr("premuto ok"), tr("GEOIN PLUGIN"), QMessageBox::Ok);
	done(0);
}

const QString icon_path("C:/Google_Drive/Regione Toscana Tools/icons");

/*******************************************/
QgsRTtoolsPlugin::QgsRTtoolsPlugin(QgisInterface* iface): mIface(iface)
{
	mAction.resize(10);
}
QgsRTtoolsPlugin::~QgsRTtoolsPlugin()
{
}
void QgsRTtoolsPlugin::initGui()
{
	// prende il menu principale
	QMenu* qm = (QMenu*) mIface->layerMenu()->parent();
	// appende il menu di collaudo
	qmb = (QMenu*) qm->addMenu("Collaudo");
	// appende la toolbar di collaudo
	qtb = mIface->addToolBar( "Collaudo" );

    QString icon = icon_path + "/projects.png";
    mAction[0] = qmb->addAction(QIcon(icon), "Gestione progetto");
	connect(mAction[0], SIGNAL(activated()), this, SLOT(set_prj()));
	qtb->addAction(mAction[0]);

    icon = icon_path + "/gps.png";
    mAction[1] = qmb->addAction(QIcon(icon), "Verifica dati GPS");
	connect(mAction[1], SIGNAL(activated()), this, SLOT(ver_gps()));
	qtb->addAction(mAction[1]);

    icon = icon_path + "/volop.png";
    mAction[2] = qmb->addAction(QIcon(icon), "Verifica Progetto volo aerofotogrammetrico");
	connect(mAction[2], SIGNAL(activated()), this, SLOT(ver_proj_volo()));
	qtb->addAction(mAction[2]);

    icon = icon_path + "/volo.png";
    mAction[3] = qmb->addAction(QIcon(icon), "Verifica volo aerofotogrammetrico");
	connect(mAction[3], SIGNAL(activated()), this, SLOT(ver_volo()));
	qtb->addAction(mAction[3]);

    icon = icon_path + "/tria.png";
    mAction[4] = qmb->addAction(QIcon(icon), "Verifica Triangolazione aerea");
	connect(mAction[4], SIGNAL(activated()), this, SLOT(ver_tria()));
	qtb->addAction(mAction[4]);

    icon = icon_path + "/ortho.png";
    mAction[5] = qmb->addAction(QIcon(icon), "Verifica orto immagini");
	connect(mAction[5], SIGNAL(activated()), this, SLOT(ver_ortho()));
	qtb->addAction(mAction[5]);

    icon = icon_path + "/voloPL.png";
    mAction[6] = qmb->addAction(QIcon(icon), "Verifica Progetto volo LIDAR");
	connect(mAction[6], SIGNAL(activated()), this, SLOT(ver_proj_lidar()));
	qtb->addAction(mAction[6]);

    icon = icon_path + "/voloL.png";
    mAction[7] = qmb->addAction(QIcon(icon), "Verifica volo LIDAR");
	connect(mAction[7], SIGNAL(activated()), this, SLOT(ver_lidar()));
	qtb->addAction(mAction[7]);

    icon = icon_path + "/Lidar.png";
    mAction[8] = qmb->addAction(QIcon(icon), "Verifica dati grezzi LIDAR");
	connect(mAction[8], SIGNAL(activated()), this, SLOT(ver_raw_lidar()));
	qtb->addAction(mAction[8]);

    icon = icon_path + "/Lidar1.png";
    mAction[9] = qmb->addAction(QIcon(icon), "Verifica elaborati LIDAR");
	connect(mAction[9], SIGNAL(activated()), this, SLOT(ver_prod_lidar()));
	qtb->addAction(mAction[9]);
}
void QgsRTtoolsPlugin::unload()
{
	for (int i = 0; i < 10; i++)
		delete mAction[i];
	qtb->deleteLater();
	qmb->deleteLater();

}
/*********************** SLOTS attivazione comandi ************/
void QgsRTtoolsPlugin::set_prj()
{
//	std::auto_ptr<Poco::ProcessHandle> ph;
//	Poco::Process::Args args;
//	ph.reset( new Poco::ProcessHandle(Poco::Process::launch("C:/Regione Toscana Tools/check_gps/Debug/check_gps.exe", args)) );
}
void QgsRTtoolsPlugin::ver_gps()
{
}
void QgsRTtoolsPlugin::ver_proj_volo()
{
}
void QgsRTtoolsPlugin::ver_volo()
{
}
void QgsRTtoolsPlugin::ver_tria()
{
}
void QgsRTtoolsPlugin::ver_ortho()
{
}
void QgsRTtoolsPlugin::ver_proj_lidar()
{
}
void QgsRTtoolsPlugin::ver_lidar()
{
}
void QgsRTtoolsPlugin::ver_raw_lidar()
{
}
void QgsRTtoolsPlugin::ver_prod_lidar()
{
}

/*********************** INTERFACCIA PIANA ********************/
QGISEXTERN QgisPlugin* classFactory(QgisInterface* iface)
{
	return new QgsRTtoolsPlugin(iface);
}
QGISEXTERN QString icon()
{
    QString icon = icon_path + "/regione.png";
    return icon.toStdString().c_str(); //"C:/Google Drive/Regione Toscana Tools/icons/regione.png";
}
QGISEXTERN QString name()
{
	return "Regione Toscana tools";
}
QGISEXTERN QString category()
{
	return "Plugin";
}
QGISEXTERN QString description()
{
	return "Regione Toscana tools per il collaudo di riprese aerofotogrammetriche e LIDAR";
}
QGISEXTERN QString version()
{
	return "0.00001";
}
// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
	return QgisPlugin::UI;
}
// Delete ourself
QGISEXTERN void unload(QgisPlugin* theQgsPointConverterPluginPointer)
{
	delete theQgsPointConverterPluginPointer;
}
/********************************************************************/
/*
_declspec (dllexport) void volo()
{
	QgsVectorLayer* qvf1 = new QgsVectorLayer("C:/assistenza/Rossi/13-aug-06/shp/linee_o.shp", "myLayer", "ogr");
	QgsVectorFileWriter::writeAsVectorFormat(qvf1, "C:/assistenza/Rossi/13-aug-06/shp/outofqgis.shp", "CP1250", 0, "ESRI Shapefile");
}
QgsVectorLayer* qvf;
void QgsRTtoolsPlugin::ver_gps()
{
	//QFileDialog qf;
	//QString fileName = qf.getOpenFileName(0, "Select a file:", "", "*.shp *.gml");

	qvf = mIface->addVectorLayer("C:/assistenza/Rossi/13-aug-06/shp/linee_o.shp", "myLayer", "ogr");
	//QMessageBox::information(0, tr("Regione Toscana"), tr("Verifica dati GPS"), QMessageBox::Ok);
}
void QgsRTtoolsPlugin::ver_proj_volo()
{
	QgsVectorFileWriter::writeAsVectorFormat(qvf, "C:/assistenza/Rossi/13-aug-06/shp/qgis.shp", "CP1250", 0, "ESRI Shapefile");
	QMessageBox::information(0, tr("Regione Toscana"), tr("Verifica progetto di volo"), QMessageBox::Ok);
}
void QgsRTtoolsPlugin::ver_volo()
{
	QgsFieldMap fields;
	QgsField field("id", QVariant::Int);
	fields[0] = field;
	field = QgsField ("nome", QVariant::String);
	fields[1] = field;

	QgsVectorFileWriter writer("C:/assistenza/Rossi/13-aug-06/shp/ppp.shp", "CP1250", fields, QGis::WKBPoint, 0, "ESRI Shapefile");

	QgsFeature fet;

	fet.setGeometry(QgsGeometry::fromPoint(QgsPoint(653136, 4856108)));
	fet.addAttribute(0, QVariant(1));
	fet.addAttribute(1, QVariant("pippo"));
	writer.addFeature(fet);

	fet.setGeometry(QgsGeometry::fromPoint(QgsPoint(651025,4855169)));
	fet.addAttribute(0, QVariant(2));
	fet.addAttribute(1, QVariant("pluto"));
	writer.addFeature(fet);

	fet.setGeometry(QgsGeometry::fromPoint(QgsPoint(649934,4855553)));
	fet.addAttribute(0, QVariant(3));
	fet.addAttribute(1, QVariant("paperino"));
	writer.addFeature(fet);

	QMessageBox::information(0, tr("Regione Toscana"), tr("Verifica volo"), QMessageBox::Ok);
}*/

