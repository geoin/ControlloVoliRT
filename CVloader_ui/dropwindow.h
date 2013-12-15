/* 
	File: dropwindow.cpp
	Author:  F.Flamigni
	Date: 2013 December 6
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

#ifndef DROPWINDOW_H
#define DROPWINDOW_H

#include <QWidget>
#include <QMetaType>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QMimeData;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
QT_END_NAMESPACE
class DropArea;
class loader;

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
		ty_contorno,
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

class DropWindow : public QWidget {
    Q_OBJECT
public:
    DropWindow();
	~DropWindow();
	void keyPressEvent( QKeyEvent* event );
private slots:
    void _assign_dropped(const QString&);
    void exec();

private:
	void _child_manager(QTreeWidgetItem * wi);
	void _item_manager(const item_obj& io, QTreeWidgetItem* wi = NULL);


    DropArea*	_drop_area;
    QTreeWidget* _prj_tree;

    QPushButton* execButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
	loader* _ld;

};


#endif
