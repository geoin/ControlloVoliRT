#ifndef RT_QGSPLUGIN_H
#define RT_QGSPLUGIN_H

#include "qgisplugin.h"
#include <QObject>
#include <QDialog>
#include <QVector>

class QAction;
class QgsGeometry;
class QTextStream;
class QToolBar;
class QMenu;

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
	dbox(QWidget* parent = NULL);
private slots:
	void esegui(bool);
};

#endif
