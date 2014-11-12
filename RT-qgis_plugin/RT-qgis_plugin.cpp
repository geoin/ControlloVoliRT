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
//#include "qgisiface.h"
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

#define RT_PLUGIN_VERSION "1.2.5"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )

#define PARAM_PREFIX(d) QString("/") + QString(d) + QString("=")
#define FLAG_PREFIX(d) QString("/") + QString(d)

#else
#define QGISEXTERN extern "C"

#define PARAM_PREFIX(d) QString("-") + QString(d)
#define FLAG_PREFIX(d) QString("-") + QString(d)

#endif

dbox::dbox(QgisInterface* mi): _mi(mi)
{
    // get plugin folder
    QByteArray p = qgetenv( "CV_QGIS_PREFIX_PATH" );
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

void dbox::_init(QVBoxLayout* qv, bool is_report)
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

    QHBoxLayout* hl2 = new QHBoxLayout;

    QPushButton* bok = new QPushButton("Esegui");
    connect(bok, SIGNAL(clicked(bool)), this, SLOT(_exec(bool)));
    hl2->addWidget(bok);

    if ( is_report ) {
        QPushButton* brep = new QPushButton("Report");
        connect(brep, SIGNAL(clicked(bool)), this, SLOT(_report(bool)));
        hl2->addWidget(brep);
    }

    QPushButton* bcanc = new QPushButton("Esci");
    connect(bcanc, SIGNAL(clicked(bool)), this, SLOT(_esci(bool)));
    hl2->addWidget(bcanc);

    qvb->addLayout(hl2);
    setLayout(qvb);

    connect(this, SIGNAL(finished(int)), this, SLOT(_chiudi(int)));
    connect(&_qp, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(_terminated(int, QProcess::ExitStatus)));
    connect(&_qp, SIGNAL(readyReadStandardOutput()), this, SLOT(_received()));

 #ifdef WIN32
       _executable += ".exe";
 #endif
    QFileInfo qf(_executable);
    if ( !qf.exists() ) {
        // if executable does not exists disable the excute button
        _out->append(_executable + " non trovato");
        bok->setEnabled(false);
    }

    _args[0] = QString(PARAM_PREFIX("d")) + _prj->text();
}
bool dbox::_dirlist(bool)
{
    QFileDialog qf;
    QString dirName = _prj->text();
    dirName = qf.getExistingDirectory(this, tr("Directory"), dirName);
    if ( !dirName.isEmpty() ) {
        _prj->setText(dirName);
        _args[0] = QString(PARAM_PREFIX("d")) + dirName;
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

    _add_layers_to_legend();
    QMessageBox::information(0, tr("GEOIN PLUGIN"), tr("tool terminated "), QMessageBox::Ok);
    //_mi->getLayerRegistry();

    //_add_layers_to_legend();
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
    QFileInfo qf1(_prj->text(), _check_name);
    QByteArray p = qgetenv( "DOCBOOKRT" );
    QString path(p);
    QDir dir(path);

#ifdef WIN32
    QString exe = "cmd.exe";
    QString launcher = "pdf_convert.bat";
    args << FLAG_PREFIX("c");
	dir.cdUp();
    QFileInfo qf(dir, launcher);
#else
    QString exe = "python";
    QString launcher = "report.py";
    QFileInfo qf(dir, launcher);
#endif

    args << qf.filePath();
    args << qf1.filePath();

    _esegui(exe, args);
}
void dbox::_exec(bool b)
{
    _qp.disconnect( SIGNAL(finished(int, QProcess::ExitStatus)), this );
    connect(&_qp, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(_terminated(int, QProcess::ExitStatus)));

    QStringList vArgs = getArgs();
    if (vArgs.length()) {
        _esegui(_executable, vArgs);
    } else {
        _esegui(_executable, _args);
    }
}

void dbox::_esegui(const QString& exec, const QStringList& args)
{
    _out->clear();
    _layers.clear();

    QString exe(exec);

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
    _args << QString(PARAM_PREFIX("d")); // project dir
    _args << FLAG_PREFIX("p");  // type of check (p = project /f = flight)

    if ( type == 1 )
        _args[1] = FLAG_PREFIX("f");

    QString name = _check_name;// + ".exe";

    QFileInfo qf(_plugin_dir, name);
    _executable = qf.filePath();

    _check_name.append( type == 0 ? "P" : "V");

    QVBoxLayout* qvb = new QVBoxLayout;

    _init(qvb);
}

/*******************************************/
Check_gps::Check_gps(QgisInterface* mi): dbox(mi)
{
    setWindowTitle("Controllo dati gps");
    _check_name = "check_gps";

    _args << QString(PARAM_PREFIX("d")); // project folder
    _args << FLAG_PREFIX("p"); // check type /p = photogrammetry /l = lidar

    QString name = _check_name;
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

    _init(qvb, false);
}
void Check_gps::_optype(int index)
{
    switch ( index ) {
    case 0:
        _args[1] = FLAG_PREFIX("p");
        break;
    case 1:
        _args[1] = FLAG_PREFIX("l");
        break;
    }
}
/**********************************************************************************/

QStringList Check_ta::getArgs() {
    QStringList args;
    args << QString(PARAM_PREFIX("d")) + _prj->text();

    QString par;

    par = _f1->text();
    if (par.length()) {
        args << QString(PARAM_PREFIX("r")) + par;
    } else {
        args << "";
    }

    par = _f2->text();
    if (par.length()) {
        args << QString(PARAM_PREFIX("c")) + par;
    } else {
        args << "";
    }

    par = _f3->text();
    if (par.length()) {
        args << QString(PARAM_PREFIX("o")) + par;
    } else {
        args << "";
    }
    return args;
}

Check_ta::Check_ta(QgisInterface* mi): dbox(mi)
{
    setWindowTitle("Controllo triangolazione aerea");
    _check_name = "check_ta";

    _args << PARAM_PREFIX("d"); // project dir
    _args << PARAM_PREFIX("r"); // first results file
    _args << PARAM_PREFIX("c"); // second results file
    _args << ""; // observation file

    QString name = _check_name;
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
         _args[1] = QString(PARAM_PREFIX("r")) + q1;

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
        _args[2] = QString(PARAM_PREFIX("c")) + q2;

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
        _args[3] = QString(PARAM_PREFIX("o")) + q3;


    QPushButton* b3 = new QPushButton("...");
    b3->setFixedWidth(20);
    connect(b3, SIGNAL(clicked(bool)), this, SLOT(_dirlist3(bool)));
    QHBoxLayout* hl3 = new QHBoxLayout;
    hl2->addWidget(l3);
    hl2->addWidget(_f3);
    hl2->addWidget(b3);
    qvb->addLayout(hl3);

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
        _args[1] = QString(PARAM_PREFIX("r")) + fileName;
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
        _args[2] = QString(PARAM_PREFIX("c")) + fileName;
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
        _args[3] = QString(PARAM_PREFIX("o")) + fileName;
        qs.setValue("TA_OBS", fileName) ;
    }
    return true;
}

Check_ortho::Check_ortho(QgisInterface* mi): dbox(mi)
{
    setWindowTitle("Controllo orto immagini");
    _check_name = "check_ortho";

    _args << QString(PARAM_PREFIX("d")); // project dir
    _args << FLAG_PREFIX("i");  // image folder

    QString name = _check_name;
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
         _args[1] = QString(PARAM_PREFIX("i")) + q1;
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
        _args[1] = QString(PARAM_PREFIX("i")) + dirName;
        qs.setValue("ORTHO_IDIR", dirName) ;
    }
    return true;
}
/************************************************************/
Check_lidar::Check_lidar(QgisInterface* mi, int type): dbox(mi)
{
    setWindowTitle(type == 0 ? "Controllo progetto di ripresa" : "Controllo della ripresa effettuata");
    _check_name = "check_lidar";

    // prepare the parameters
    _args << QString(PARAM_PREFIX("d")); // project dir
    _args << FLAG_PREFIX("p");  // type of check (p = project /f = flight)

    if ( type == 1 )
        _args[1] = FLAG_PREFIX("f");

    QString name = _check_name;// + ".exe";

    QFileInfo qf(_plugin_dir, name);
    _executable = qf.filePath();

    _check_name.append( type == 0 ? "P" : "V");

    QVBoxLayout* qvb = new QVBoxLayout;

    _init(qvb);
}
/**********************************************************/

Check_lidar_raw::Check_lidar_raw(QgisInterface* mi): dbox(mi)
{
    setWindowTitle("Controllo lidar dati grezzi");
    _check_name = "check_lidar_raw";

    // prepare the parameters
    _args << QString(PARAM_PREFIX("d")); // project dir

    QString name = _check_name;// + ".exe";

    QFileInfo qf(_plugin_dir, name);
    _executable = qf.filePath();

    QVBoxLayout* qvb = new QVBoxLayout;

    _init(qvb);
}

/*******************************************/
QString icon_path; //("C:/Google Drive/Regione Toscana Tools/icons");

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
    QByteArray p = qgetenv( "CV_QGIS_PREFIX_PATH" );
    icon_path = QDir::cleanPath(QString(p.data()) + QDir::separator() + "icons");

    // get the main menu of QGIS
    QMenu* qm = (QMenu*) mIface->layerMenu()->parent();
    // appende il menu di collaudo
    qmb = (QMenu*) qm->addMenu("Collaudo");
    // appende la toolbar di collaudo
    qtb = mIface->addToolBar( "Collaudo" );

    long k = -1;

    QString icon = icon_path + "/projects.png";
    mAction[++k] = qmb->addAction(QIcon(icon), "Gestione progetto");
    connect(mAction[k], SIGNAL(activated()), this, SLOT(set_prj()));
    qtb->addAction(mAction[k]);

    icon = icon_path + "/voloP.png";
    mAction[++k] = qmb->addAction(QIcon(icon), "Verifica Progetto volo aerofotogrammetrico");
    connect(mAction[k], SIGNAL(activated()), this, SLOT(ver_proj_volo()));
    qtb->addAction(mAction[k]);

    icon = icon_path + "/gps.png";
    mAction[++k] = qmb->addAction(QIcon(icon), "Verifica dati GPS");
    connect(mAction[k], SIGNAL(activated()), this, SLOT(ver_gps()));
    qtb->addAction(mAction[k]);

    icon = icon_path + "/volo.png";
    mAction[++k] = qmb->addAction(QIcon(icon), "Verifica volo aerofotogrammetrico");
    connect(mAction[k], SIGNAL(activated()), this, SLOT(ver_volo()));
    qtb->addAction(mAction[k]);

    icon = icon_path + "/tria.png";
    mAction[++k] = qmb->addAction(QIcon(icon), "Verifica Triangolazione aerea");
    connect(mAction[k], SIGNAL(activated()), this, SLOT(ver_tria()));
    qtb->addAction(mAction[k]);

    icon = icon_path + "/Ortho.png";
    mAction[++k] = qmb->addAction(QIcon(icon), "Verifica orto immagini");
    connect(mAction[k], SIGNAL(activated()), this, SLOT(ver_ortho()));
    qtb->addAction(mAction[k]);

    icon = icon_path + "/voloPL.png";
    mAction[++k] = qmb->addAction(QIcon(icon), "Verifica Progetto volo LIDAR");
    connect(mAction[k], SIGNAL(activated()), this, SLOT(ver_proj_lidar()));
    qtb->addAction(mAction[k]);

    icon = icon_path + "/voloL.png";
    mAction[++k] = qmb->addAction(QIcon(icon), "Verifica volo LIDAR");
    connect(mAction[k], SIGNAL(activated()), this, SLOT(ver_lidar()));
    qtb->addAction(mAction[k]);

    icon = icon_path + "/lidar.png";
    mAction[++k] = qmb->addAction(QIcon(icon), "Verifica dati grezzi LIDAR");
    connect(mAction[k], SIGNAL(activated()), this, SLOT(ver_raw_lidar()));
    qtb->addAction(mAction[k]);

    icon = icon_path + "/lidar1.png";
    mAction[++k] = qmb->addAction(QIcon(icon), "Verifica elaborati LIDAR");
    connect(mAction[k], SIGNAL(activated()), this, SLOT(ver_prod_lidar()));
    qtb->addAction(mAction[k]);
}
void QgsRTtoolsPlugin::unload()
{
    for (int i = 0; i < mAction.size(); i++)
        delete mAction[i];
    qtb->deleteLater();
    qmb->deleteLater();

}
/*********************** SLOTS attivazione comandi ************/
void QgsRTtoolsPlugin::set_prj()
{
    QString name = "CVloader"; //"CVloader_ui.exe";
    QByteArray p = qgetenv( "CV_QGIS_PREFIX_PATH" );

    QString plugin_dir = QDir::cleanPath(QString(p.data()) + QDir::separator() + "plugins");

    QFileInfo qf(plugin_dir, name);

    //QMessageBox::information(NULL, "starting...", qf.filePath(), QMessageBox::Ok);
    QProcess::startDetached(QString("\"") + qf.filePath() + QString("\""));
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
    Check_lidar* db = new Check_lidar(mIface, 0);
    db->open();
}
void QgsRTtoolsPlugin::ver_lidar()
{
    Check_lidar* db = new Check_lidar(mIface, 1);
    db->open();
}
void QgsRTtoolsPlugin::ver_raw_lidar()
{
    Check_lidar_raw* db = new Check_lidar_raw(mIface);
    db->open();
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
    QByteArray p = qgetenv( "CV_QGIS_PREFIX_PATH" );
    icon_path = QDir::cleanPath(QString(p.data()) + QDir::separator() + "icons");


    QString icon = icon_path + "/Regione.png";
    return icon.toStdString().c_str(); 
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
    return RT_PLUGIN_VERSION;
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

