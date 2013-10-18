/*
    File: feature.h
    Author: F.Flamigni
    Date: 2013 October 17
    Comment:

    Disclaimer:
        This file is part of Tabula.

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

#ifndef TOOLS_EXPORTS_H
#define TOOLS_EXPORTS_H

#ifndef WIN32
	#define TOOLS_EXPORTS
#else
#ifdef DLL_EXPORTS
	#define TOOLS_EXPORTS __declspec( dllexport )
#else
	#define TOOLS_EXPORTS __declspec( dllimport )
#endif

#endif // #ifndef WIN32

#endif // #ifndef TOOLS_EXPORTS_H