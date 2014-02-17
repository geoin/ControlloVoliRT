/* 
	File: CVloader_ui.cpp
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

#include <QApplication>
#include "dropwindow.h"
#include <QFileInfo>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DropWindow window;
    window.show();

    QByteArray p = qgetenv( "DOCKBOOKRT" );
	QDir dir = QFileInfo(p).path();
    QFileInfo qf(dir, "pdf_convert.bat");

    //QFileInfo qf(_plugin_dir, "run.bat");
    //QFileInfo qf("pdf_convert.bat");
	QString qs = qf.filePath();


    return app.exec();
}
