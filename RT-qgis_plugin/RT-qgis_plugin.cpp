/*
    File: RT-qgis_plugin.cpp
    Author:  F.Flamigni
    Date: 2013 November 28
    Comment:

    Disclaimer:
        This file is part of RT_Controllo_Voli.

        Tabula is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        Tabula is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with Tabula.  If not, see <http://www.gnu.org/licenses/>.


        Copyright (C) 2013 Geoin s.r.l.

*/

#include "RT-qgis_plugin.h"
#include "qgisinterface.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include <qgsvectorfilewriter.h>
#include <qgsdatasourceuri.h>
#include <QFileDialog>
//#include <QMessageBox>
#include <QTextStream>
#include <QPushButton>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

dbox::dbox(QgisInterface* mi): _mi(mi)
{
}
void dbox::_init(QVBoxLayout* qv)
{
    QVBoxLayout* qvb = new QVBoxLayout;

    // sezione iniziale comune a tutti
    QLabel* l1 = new QLabel("Cartella progetto:");
    _prj = new QLineEdit;
    _prj->setText("C:/Google_drive/Regione Toscana Tools/Dati_test/scarlino");
    QPushButton* b1 = new QPushButton("...");
    b1->setFixedWidth(20);
    connect(b1, SIGNAL(clicked(bool)), this, SLOT(_dirlist(bool)));
    QHBoxLayout* hl1 = new QHBoxLayout;
    hl1->addWidget(l1);
    hl1->addWidget(_prj);
    hl1->addWidget(b1);
    qvb->addLayout(hl1);

    // parte specifica del comando
    qvb->addLayout(qv);

    // parte finale comune a tutti
    _out = new QTextEdit(this);
     _out->setFixedHeight(200);
     qvb->addWidget(_out);

    QPushButton* bok = new QPushButton("Esegui");
    connect(bok, SIGNAL(clicked(bool)), this, SLOT(_esegui(bool)));
    QPushButton* bcanc = new QPushButton("Esci");
    connect(bcanc, SIGNAL(clicked(bool)), this, SLOT(_esci(bool)));
    QHBoxLayout* hl2 = new QHBoxLayout;
    hl2->addWidget(bok);
    hl2->addWidget(bcanc);

    qvb->addLayout(hl2);
    setLayout(qvb);

    connect(this, SIGNAL(finished(int)), this, SLOT(_chiudi(int)));
    connect(&_qp, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(_terminated(int, QProcess::ExitStatus)));
    connect(&_qp, SIGNAL(readyReadStandardOutput()), this, SLOT(_received()));

    QFileInfo qf(_executable);
    if ( !qf.exists() )
        _out->append(_executable + " non trovato");
}
bool dbox::_dirlist(bool)
{
    QFileDialog qf;
    QString dirName = _prj->text();
    dirName = qf.getExistingDirectory(this, tr("Directory"), dirName);
    if ( !dirName.isEmpty() ) {
        _prj->setText(dirName);
        _args[0] = QString("/d=") + dirName;
    }
    //QString fileName = qf.getOpenFileName(0, "Select a file:", "", "*.shp *.gml");
    return true;
}
void dbox::_chiudi(int result)
{
    deleteLater();
}
void dbox::_esci(bool b)
{
    done(0);
}
void dbox::_received()
{
    QByteArray qba = _qp.readAllStandardOutput();
    QString qs = qba.data();
    if ( !qs.isEmpty() ) {
        qs = qs.remove('\n');
        qs = qs.remove('\r');
        _out->append(qs);
        if ( qs.contains("Layer:", Qt::CaseInsensitive) ) {
            QStringList q =qs.split(':');
            if ( q.size() > 1 ) {
                _layers.push_back(q[1]);
            }
        }
    }
}
void dbox::_terminated(int exitCode, QProcess::ExitStatus a)
{
    _qm->done(0);
    delete _qm;
    _qm = NULL;
    QMessageBox::information(0, tr("GEOIN PLUGIN"), tr("tool terminated "), QMessageBox::Ok);
    _add_layers_to_legend();
}
void dbox::_add_layers_to_legend()
{


    QgsDataSourceURI uri;
    QString dir = _prj->text();
    QFileInfo qf(dir, "geo.sqlite");

    _out->append("====>" + dir + "," +qf.path());
    uri.setDatabase(qf.filePath());

    _out->append(qf.path());

    if ( _layers.size() ) {
        for (int i = 0; i < _layers.size(); i++) {

            uri.setDataSource("", _layers[i], "geom"); // schema, nome layer, nome colonna geografica
            _mi->addVectorLayer(uri.uri(), _layers[i], "spatialite"); // uri, nome lnella legenda, nome provider
        }
    }
}

void dbox::_esegui(bool b)
{
    _out->clear();
    _layers.clear();
    _out->append(_executable);
    for (int i = 0; i < _args.size(); i++)
        _out->append(_args[i]);

    _qp.start(_executable, _args);

    _qp.waitForStarted();
    if ( _qp.state() != QProcess::Running ) {
        _out->append("Impossibile avviare il processo");
        return;
    }

    _qm = new QMessageBox;
    _qm->setWindowTitle("GEOIN PLUGIN");
    _qm->setText("tool running");
    _qm->setWindowModality(Qt::ApplicationModal);
    _qm->addButton("Interrompi", QMessageBox::AcceptRole);
    _qm->exec();

    //_qm.information(0, tr("GEOIN PLUGIN"), tr("tool running "), QMessageBox::Ok);
    _qp.kill();
}
/***************************************************/
Check_photo::Check_photo(QgisInterface* mi, int type): dbox(mi)
{
    _args << QString("/d="); // + _prj->text() + QString("");
    _args << "/p";
    _args << "/s=1000";

    if ( type == 0 ) {
        setWindowTitle("Controllo progetto di volo");
    } else {
        setWindowTitle("Controllo del volo effettuato");
        _args[1] = "/f";
    }
    _executable = "\"C:/OSGeo4W/apps/qgis/plugins/check_photo.exe\"";

    QVBoxLayout* qvb = new QVBoxLayout;

    QLabel* l2 = new QLabel("Scala di lavoro:");
    QComboBox* cmb = new QComboBox;
    cmb->addItem("1:1000");
    cmb->addItem("1:2000");
    cmb->addItem("1:5000");
    cmb->addItem("1:10000");
    cmb->setCurrentIndex(0);
    connect(cmb, SIGNAL(currentIndexChanged(int)), this, SLOT(_optype(int)));
    QHBoxLayout* hl2 = new QHBoxLayout;
    hl2->addWidget(l2);
    hl2->addWidget(cmb);
    qvb->addLayout(hl2);

    _init(qvb);
}
void Check_photo::_optype(int index)
{
    switch ( index ) {
    case 0:
        _args[2] = "/s=1000";
        break;
    case 1:
        _args[2] = "/s=2000";
        break;
    case 2:
        _args[2] = "/s=5000";
        break;
    case 3:
        _args[2] = "/s=10000";
        break;
    }
}
/*******************************************/
Check_gps::Check_gps(QgisInterface* mi): dbox(mi)
{
    _args << QString("/d=");
    _args << "/p";
    _args << "/s=1000";

    setWindowTitle("Controllo dati gps");
    _executable = "\"C:/OSGeo4W/apps/qgis/plugins/check_gps.exe\"";

    QVBoxLayout* qvb = new QVBoxLayout;
    _init(qvb);
}

/*******************************************/
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

}
void QgsRTtoolsPlugin::ver_gps()
{
    Check_gps* db = new Check_gps(mIface);
    db->open();
}
void QgsRTtoolsPlugin::ver_proj_volo()
{
    Check_photo* db = new Check_photo(mIface, 0);
    db->open();
}
void QgsRTtoolsPlugin::ver_volo()
{
    Check_photo* db = new Check_photo(mIface, 1);
    db->open();
}
void QgsRTtoolsPlugin::ver_tria()
{
    QgsDataSourceURI uri;
    uri.setDatabase("C:/Google_drive/Regione Toscana Tools/Dati_test/scarlino/geo.sqlite");
    uri.setDataSource("", "AVOLOP", "geom"); // schema, nome layer, nome colonna geografica
    mIface->addVectorLayer(uri.uri(), "AVOLOP", "spatialite"); // uri, nome lnella legenda, nome provider
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

