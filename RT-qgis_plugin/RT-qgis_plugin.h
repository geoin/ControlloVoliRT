/*
    File: RT-qgis_plugin.h
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

#ifndef RT_QGSPLUGIN_H
#define RT_QGSPLUGIN_H

#include <qgisplugin.h>
#include <QObject>
#include <QDialog>
#include <QProcess>
#include <QMessageBox>
#include <QVector>

class QAction;
class QgsGeometry;
class QTextStream;
class QToolBar;
class QMenu;
class QLineEdit;
class QTextEdit;
class QVBoxLayout;

class QgsRTtoolsPlugin: public QObject, public QgisPlugin {
    Q_OBJECT
public:
    QgsRTtoolsPlugin(QgisInterface* iface);
    ~QgsRTtoolsPlugin();
    void initGui();
    void unload();
private:
    QgisInterface* mIface;
    QVector<QAction*> mAction;
    QToolBar* qtb;
    QMenu* qmb;
private slots:
    void set_prj();
    void ver_gps();
    void ver_proj_volo();
    void ver_volo();
    void ver_tria();
    void ver_ortho();
    void ver_proj_lidar();
    void ver_lidar();
    void ver_raw_lidar();
    void ver_prod_lidar();
};

class dbox: public QDialog {
    Q_OBJECT
public:
    dbox(QgisInterface* mi);
    virtual QStringList getArgs() { return QStringList(); }

protected slots:
    void _report(bool);
    void _exec(bool);
    void _chiudi(int);
    void _terminated(int, QProcess::ExitStatus);
    void _terminated1(int, QProcess::ExitStatus);
    void _received();
    void _esci(bool);
    bool _dirlist(bool);
protected:
    void _esegui(const QString& exe, const QStringList& args);
    void _init(QVBoxLayout* qvb, bool is_report = true);
    void _add_layers_to_legend(void);
    QString get_last_prj(void);
    QString _plugin_dir;
    QString _set_dir;
    QString _executable;
    QString _check_name;

    QTextEdit* _out;
    QMessageBox* _qm;
    QProcess _qp;
    QStringList _args;
    QLineEdit* _prj; // project directory
    QVector<QString> _layers;
    QgisInterface* _mi;
};
/**************************************/
class Check_gps: public dbox {
    Q_OBJECT
public:
    Check_gps(QgisInterface* mi);
protected slots:
    void _optype(int index)    ;
private:
};
class Check_photo: public dbox {
    Q_OBJECT
public:
    Check_photo(QgisInterface* mi, int type);
private:
};
class Check_ta: public dbox {
    Q_OBJECT
public:
    Check_ta(QgisInterface* mi);

    virtual QStringList getArgs();

protected slots:
    bool _dirlist1(bool);
    bool _dirlist2(bool);
    bool _dirlist3(bool);
private:
    QLineEdit* _f1;
    QLineEdit* _f2;
    QLineEdit* _f3;
    QString _file1, _file2, _obs_file;
};
class Check_ortho: public dbox {
    Q_OBJECT
public:
    Check_ortho(QgisInterface* mi);
protected slots:
    bool _img_dir(bool);
private:
     QLineEdit* _idir;
};
class Check_lidar: public dbox {
    Q_OBJECT
public:
    Check_lidar(QgisInterface* mi, int type);
private:
};

class Check_lidar_raw: public dbox {
    Q_OBJECT
public:
    Check_lidar_raw(QgisInterface* mi);
private:
};

class Check_lidar_final: public dbox {
    Q_OBJECT
public:
    Check_lidar_final(QgisInterface* mi);
private:
};

#endif
