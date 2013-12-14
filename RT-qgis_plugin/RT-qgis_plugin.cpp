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
#include <qgsmessagebar.h>
#include <QFileDialog>
#include <QProcessEnvironment>
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
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

dbox::dbox(QgisInterface* mi): _mi(mi)
{
    // get plugin folder
    QByteArray p = qgetenv( "QGIS_PREFIX_PATH" );
    _plugin_dir = QDir::cleanPath(QString(p.data()) + QDir::separator() + "plugins");

    // get settings folder
    p = qgetenv( "ALLUSERSPROFILE" );
    _set_dir = QDir::cleanPath(QString(p.data()) + QDir::separator() + "RT_tools");
    QDir qd(_set_dir);
    if ( !qd.exists() )
        qd.mkdir(_set_dir);
}
QString dbox::get_last_prj()
{
    QFileInfo qfs(_set_dir, "rt_tools.cfg");
    QSettings qs(qfs.filePath(), QSettings::IniFormat);
    QString last_prj = qs.value("PROJ_DIR", "").toString();
    return last_prj;
}

void dbox::_init(QVBoxLayout* qv)
{
    QString last_prj = get_last_prj();

    QVBoxLayout* qvb = new QVBoxLayout;

    // sezione iniziale comune a tutti
    QLabel* l1 = new QLabel("Cartella progetto:");
    _prj = new QLineEdit;
    _prj->setText(last_prj);
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
     _out->setMinimumHeight(200);
     _out->setMinimumWidth(400);
     qvb->addWidget(_out);

    QPushButton* bok = new QPushButton("Esegui");
    connect(bok, SIGNAL(clicked(bool)), this, SLOT(_exec(bool)));
    QPushButton* brep = new QPushButton("Report");
    connect(brep, SIGNAL(clicked(bool)), this, SLOT(_report(bool)));
    QPushButton* bcanc = new QPushButton("Esci");
    connect(bcanc, SIGNAL(clicked(bool)), this, SLOT(_esci(bool)));
    QHBoxLayout* hl2 = new QHBoxLayout;
    hl2->addWidget(bok);
    hl2->addWidget(brep);
    hl2->addWidget(bcanc);

    qvb->addLayout(hl2);
    setLayout(qvb);

    connect(this, SIGNAL(finished(int)), this, SLOT(_chiudi(int)));
    connect(&_qp, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(_terminated(int, QProcess::ExitStatus)));
    connect(&_qp, SIGNAL(readyReadStandardOutput()), this, SLOT(_received()));

    QFileInfo qf(_executable);
    if ( !qf.exists() ) {
        // if executable does not exists disable the excute button
        _out->append(_executable + " non trovato");
        bok->setEnabled(false);
    }

    _args[0] = QString("/d=") + _prj->text();
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
    QString last_prj = _prj->text();
    if ( !last_prj.isEmpty() ) {
        QFileInfo qfs(_set_dir, "rt_tools.cfg");
        QSettings qs(qfs.filePath(), QSettings::IniFormat);
        qs.setValue("PROJ_DIR", last_prj) ;
    }
    deleteLater();
}
void dbox::_esci(bool b)
{
    done(0);
}
void dbox::_received()
{
    QByteArray qba = _qp.readAllStandardOutput();
    QString qs(qba);

    if ( !qs.isEmpty() ) {
        if ( qs.contains("Layer:", Qt::CaseInsensitive) ) {
            QString q1 = qs;
            q1.replace('\n', ':');
            q1.replace('\r', ':');
            QStringList q = q1.split(':');
            if ( q.size() > 1 ) {
                for ( int i = 0; i < q.size(); i++) {
                    if ( q[i].contains("Layer", Qt::CaseInsensitive) ) {
                        _layers.push_front(q[i + 1]);
                    }
                }
            }
        }
        qs = qs.remove('\r');
        _out->append(qs);
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
void dbox::_terminated1(int exitCode, QProcess::ExitStatus a)
{
    _qm->done(0);
    delete _qm;
    _qm = NULL;
    QString dir = _prj->text();
    QString name = _check_name + ".pdf";
    QFileInfo qf(dir, name);
    QString qs = "file:" + qf.filePath();
    QDesktopServices::openUrl(QUrl(qs));
}
void dbox::_add_layers_to_legend()
{
    QgsDataSourceURI uri;
    QString dir = _prj->text();
    QFileInfo qf(dir, "geo.sqlite");

    uri.setDatabase(qf.filePath());

    if ( _layers.size() ) {
        for (int i = 0; i < _layers.size(); i++) {
            uri.setDataSource("", _layers[i], "geom"); // schema, nome layer, nome colonna geografica
            _mi->addVectorLayer(uri.uri(), _layers[i], "spatialite"); // uri, nome lnella legenda, nome provider
        }
    }
}
void dbox::_report(bool b)
{
    _qp.disconnect( SIGNAL(finished(int, QProcess::ExitStatus)), this );
    connect(&_qp, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(_terminated1(int, QProcess::ExitStatus)));

    QStringList args;
    QString exe = "cmd.exe";
    args << "/c";
    QFileInfo qf(_plugin_dir, "run.bat");
    args << qf.filePath();
    args << _prj->text();
    args << _check_name;

    _esegui(exe, args);
}
void dbox::_exec(bool b)
{
    _qp.disconnect( SIGNAL(finished(int, QProcess::ExitStatus)), this );
    connect(&_qp, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(_terminated(int, QProcess::ExitStatus)));
    _esegui(_executable, _args);
}

void dbox::_esegui(const QString& exe, const QStringList& args)
{
    _out->clear();
    _layers.clear();
    QString qs(exe);
    for (int i = 0; i < args.size(); i++)
        qs += QString(" ") + args[i];
    _out->append(qs);

    _qp.start(exe, args);

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

    _qp.kill();
}
/***************************************************/
Check_photo::Check_photo(QgisInterface* mi, int type): dbox(mi)
{
    setWindowTitle(type == 0 ? "Controllo progetto di volo" : "Controllo del volo effettuato");
    _check_name = "check_photo";

    // prepare the parameters
    _args << QString("/d="); // project dir
    _args << "/p";  // type of check (p = project /f = flight
    _args << "/s=1000";

    if ( type == 1 )
        _args[1] = "/f";

    QString name = _check_name + ".exe";

    QFileInfo qf(_plugin_dir, name);
    _executable = qf.filePath();

    _check_name.append( type == 0 ? "P" : "V");

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
    setWindowTitle("Controllo dati gps");
    _check_name = "check_gps";

    _args << QString("/d="); // project folder
    _args << "/p"; // check type /p = photogrammetry /l = lidar

    QString name = _check_name + ".exe";
    QFileInfo qf(_plugin_dir, name);
    _executable = qf.filePath();

    QVBoxLayout* qvb = new QVBoxLayout;

    QLabel* l2 = new QLabel("Tipo di progetto:");
    QComboBox* cmb = new QComboBox;
    cmb->addItem("Fotogrammetria");
    cmb->addItem("Lidar");
    cmb->setCurrentIndex(0);
    connect(cmb, SIGNAL(currentIndexChanged(int)), this, SLOT(_optype(int)));
    QHBoxLayout* hl2 = new QHBoxLayout;
    hl2->addWidget(l2);
    hl2->addWidget(cmb);
    qvb->addLayout(hl2);

    _init(qvb);
}
void Check_gps::_optype(int index)
{
    switch ( index ) {
    case 0:
        _args[1] = "/p";
        break;
    case 1:
        _args[1] = "/l";
        break;
    }
}
/**********************************************************************************/
Check_ta::Check_ta(QgisInterface* mi): dbox(mi)
{
    setWindowTitle("Controllo triangolazione aerea");
    _check_name = "check_ta";

    _args << QString("/d="); // project dir
    _args << "/r="; // first results file
    _args << "/c="; // second results file
    _args << "/o="; // observation file
    _args << "/s=1000";

    QString name = _check_name + ".exe";
    QFileInfo qf(_plugin_dir, name);
    _executable = qf.filePath();

    QFileInfo qfs(_set_dir, "rt_tools.cfg");
    QSettings qs(qfs.filePath(), QSettings::IniFormat);


    QVBoxLayout* qvb = new QVBoxLayout;

    QLabel* l1 = new QLabel("File di riferimento:");
    _f1 = new QLineEdit;
    QString q1 = qs.value("TA_REF", "").toString();
    _f1->setText(q1);
    if ( !q1.isEmpty() )
         _args[1] = QString("/r=") + q1;

    QPushButton* b1 = new QPushButton("...");
    b1->setFixedWidth(20);
    connect(b1, SIGNAL(clicked(bool)), this, SLOT(_dirlist1(bool)));
    QHBoxLayout* hl1 = new QHBoxLayout;
    hl1->addWidget(l1);
    hl1->addWidget(_f1);
    hl1->addWidget(b1);
    qvb->addLayout(hl1);

    QLabel* l2 = new QLabel("File da confrontare:");
    _f2 = new QLineEdit;
    QString q2 = qs.value("TA_CFG", "").toString();
    _f2->setText(q2);
    if ( !q2.isEmpty() )
        _args[2] = QString("/c=") + q2;

    QPushButton* b2 = new QPushButton("...");
    b2->setFixedWidth(20);
    connect(b2, SIGNAL(clicked(bool)), this, SLOT(_dirlist2(bool)));
    QHBoxLayout* hl2 = new QHBoxLayout;
    hl2->addWidget(l2);
    hl2->addWidget(_f2);
    hl2->addWidget(b2);
    qvb->addLayout(hl2);

    QLabel* l3 = new QLabel("File Osservazioni:");
    _f3 = new QLineEdit;
    QString q3 = qs.value("TA_OBS", "").toString();
    _f3->setText(q3);
    if ( !q3.isEmpty() )
        _args[3] = QString("/o=") + q3;


    QPushButton* b3 = new QPushButton("...");
    b3->setFixedWidth(20);
    connect(b3, SIGNAL(clicked(bool)), this, SLOT(_dirlist3(bool)));
    QHBoxLayout* hl3 = new QHBoxLayout;
    hl2->addWidget(l3);
    hl2->addWidget(_f3);
    hl2->addWidget(b3);
    qvb->addLayout(hl3);

    QLabel* l4 = new QLabel("Scala di lavoro:");
    QComboBox* cmb = new QComboBox;
    cmb->addItem("1:1000");
    cmb->addItem("1:2000");
    cmb->addItem("1:5000");
    cmb->addItem("1:10000");
    cmb->setCurrentIndex(0);
    connect(cmb, SIGNAL(currentIndexChanged(int)), this, SLOT(_optype(int)));
    QHBoxLayout* hl4 = new QHBoxLayout;
     hl4->addWidget(l4);
     hl4->addWidget(cmb);
     qvb->addLayout(hl4);

     _init(qvb);
}

bool Check_ta::_dirlist1(bool)
{
    QFileInfo qfs(_set_dir, "rt_tools.cfg");
    QSettings qs(qfs.filePath(), QSettings::IniFormat);

    QFileDialog qf;
    QString fileName = _f1->text();
    if ( fileName.isEmpty() ) {
            fileName = _prj->text();
    }
    fileName = qf.getOpenFileName(this, "Selezionare il file con gli assetti di riferimento:", fileName, "*.txt");
    if ( !fileName.isEmpty() ) {
        _f1->setText(fileName);
        _args[1] = QString("/r=") + fileName;
        qs.setValue("TA_REF", fileName) ;
    }
    return true;
}
bool Check_ta::_dirlist2(bool)
{
    QFileInfo qfs(_set_dir, "rt_tools.cfg");
    QSettings qs(qfs.filePath(), QSettings::IniFormat);

    QFileDialog qf;
    QString fileName = _f2->text();
    if ( fileName.isEmpty() ) {
            fileName = _prj->text();
    }
    fileName = qf.getOpenFileName(this, "Selezionare il file con gli assetti da confrontare:", fileName);
    if ( !fileName.isEmpty() ) {
        _f2->setText(fileName);
        _args[2] = QString("/c=") + fileName;
        qs.setValue("TA_CFG", fileName) ;
    }
    return true;
}
bool Check_ta::_dirlist3(bool)
{
    QFileInfo qfs(_set_dir, "rt_tools.cfg");
    QSettings qs(qfs.filePath(), QSettings::IniFormat);

    QFileDialog qf;
    QString fileName = _f3->text();
    if ( fileName.isEmpty() ) {
           fileName = _prj->text();
    }
    fileName = qf.getOpenFileName(this, "Selezionare il file con le osservazioni:", fileName);
    if ( !fileName.isEmpty() ) {
        _f3->setText(fileName);
        _args[3] = QString("/o=") + fileName;
        qs.setValue("TA_OBS", fileName) ;
    }
    return true;
}
void Check_ta::_optype(int index)
{
    switch ( index ) {
    case 0:
        _args[4] = "/s=1000";
        break;
    case 1:
        _args[4] = "/s=2000";
        break;
    case 2:
        _args[4] = "/s=5000";
        break;
    case 3:
        _args[4] = "/s=10000";
        break;
    }
}
Check_ortho::Check_ortho(QgisInterface* mi): dbox(mi)
{
    setWindowTitle("Controllo orto immagini");
    _check_name = "check_ortho";

    _args << QString("/d="); // project dir
    _args << "/i";  // image folder
    _args << "/s=1000"; // reference scale

    QString name = _check_name + ".exe";
    QFileInfo qf(_plugin_dir, name);
    _executable = qf.filePath();

    QVBoxLayout* qvb = new QVBoxLayout;

    QFileInfo qfs(_set_dir, "rt_tools.cfg");
    QSettings qs(qfs.filePath(), QSettings::IniFormat);

    // cartella delle immagini
    QLabel* l1 = new QLabel("Cartella Ortho immagini:");
    _idir = new QLineEdit;
    QString q1 = qs.value("ORTHO_IDIR", "").toString();
    _idir->setText(q1);
    if ( !q1.isEmpty() )
         _args[1] = QString("/i=") + q1;
    QPushButton* b1 = new QPushButton("...");
    b1->setFixedWidth(20);
    connect(b1, SIGNAL(clicked(bool)), this, SLOT(_img_dir(bool)));
    QHBoxLayout* hl1 = new QHBoxLayout;
    hl1->addWidget(l1);
    hl1->addWidget(_idir);
    hl1->addWidget(b1);

    qvb->addLayout(hl1);

    _init(qvb);
}
bool Check_ortho::_img_dir(bool)
{
    QFileInfo qfs(_set_dir, "rt_tools.cfg");
    QSettings qs(qfs.filePath(), QSettings::IniFormat);

    QFileDialog qf;
    QString dirName = _idir->text();
    if ( dirName.isEmpty() ) {
         dirName = _prj->text();
    }
    dirName = qf.getExistingDirectory(this, tr("Directory"), dirName);
    if ( !dirName.isEmpty() ) {
        _idir->setText(dirName);
        _args[1] = QString("/i=") + dirName;
        qs.setValue("ORTHO_IDIR", dirName) ;
    }
    return true;
}
/*******************************************/
const QString icon_path("C:/Google Drive/Regione Toscana Tools/icons");

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
    // get the main menu of QGIS
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
    QString name = "CVloader_ui.exe";
    QByteArray p = qgetenv( "QGIS_PREFIX_PATH" );
    QString plugin_dir = QDir::cleanPath(QString(p.data()) + QDir::separator() + "plugins");

    QFileInfo qf(plugin_dir, name);

    //QMessageBox::information(NULL, "starting...", qf.filePath(), QMessageBox::Ok);
    QProcess::startDetached(qf.filePath());
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
    Check_ta* db = new Check_ta(mIface);
    db->open();
}
void QgsRTtoolsPlugin::ver_ortho()
{
    Check_ortho* db = new Check_ortho(mIface);
    db->open();
}
void QgsRTtoolsPlugin::ver_proj_lidar()
{
   // QgsDataSourceURI uri;
    //uri.setDatabase("C:/Google_drive/Regione Toscana Tools/Dati_test/scarlino/geo.sqlite");
    //uri.setDataSource("", "AVOLOP", "geom"); // schema, nome layer, nome colonna geografica
    //mIface->addVectorLayer(uri.uri(), "AVOLOP", "spatialite"); // uri, nome lnella legenda, nome provider
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

