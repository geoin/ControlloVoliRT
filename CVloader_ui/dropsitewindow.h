
#ifndef DROPSITEWINDOW_H
#define DROPSITEWINDOW_H

#include <QWidget>
#include <QMetaType>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QMimeData;
class QPushButton;
class QTreeWidget;
QT_END_NAMESPACE
class DropArea;

class item_obj {
public:
	enum ITEM_TYPE {
		ty_project = 0,
		ty_fotogra,
		ty_lidar,
		ty_assi_p,
		ty_assi,
		ty_carto,
		ty_assetti_p,
		ty_assetti,
		ty_camera,
		ty_dem,
		ty_quadro,
		ty_gps,
		ty_missione,
		ty_base
	};
	item_obj() {}
	item_obj(const QString& name, const QString& ext, const QString& msg, ITEM_TYPE type, bool acceptkey = true, bool acceptdrop = true): 
		_name(name), _data_type(ext), _msg(msg), _type(type), _accepkey(acceptkey), _acceptdrop(acceptdrop) {}
	void name(const QString& name) { _name = name; }
	const QString& name(void) const { return _name; }
	void data_type(const QString& type) { _data_type = type; }
	const QString& data_type(void) const { return _data_type; }
	void msg(const QString& msg) { _msg = msg; }
	const QString& msg(void) const { return _msg; }
	void dropped(const QString& dropped) { _dropped = dropped; }
	const QString& dropped(void) const { return _dropped; }
	bool accepkey(void) const { return _accepkey; }
	bool acceptdrop(void) const { return _acceptdrop; }
	void type(ITEM_TYPE type) { _type = type; }
	ITEM_TYPE type(void) const { return _type; }
private:
	QString _name;
	QString _data_type;
	QString _msg;
	QString _dropped;
	ITEM_TYPE _type;
	bool _accepkey;
	bool _acceptdrop;
};
Q_DECLARE_METATYPE(item_obj)

class DropSiteWindow : public QWidget {
    Q_OBJECT
public:
    DropSiteWindow();
	void keyPressEvent( QKeyEvent* event );
private slots:
    void _assign_dropped(const QString&);

private:
    DropArea*	_drop_area;
    QTreeWidget* _prj_tree;

    QPushButton *clearButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
};


#endif
