/*
    File: docbook.h
    Author:  F.Flamigni
    Date: 2013 November 07
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

#ifndef DOCKBOOK_H
#define DOCKBOOK_H

#include "Poco/Xml/XMLWriter.h"
#include "Poco/SAX/AttributesImpl.h"
#include "Poco/SharedPtr.h"
#include <fstream>
#include <sstream>
#include <list>

//using namespace Poco::XML;
//using namespace Poco;

class docbook_item;
typedef Poco::SharedPtr<docbook_item> Doc_Item;

class docbook_item {
public:
	docbook_item() {}
	docbook_item(const std::string& tag): _tag(tag) {}
	docbook_item(const std::string& tag, const Poco::XML::AttributesImpl& attr): _tag(tag), _attrs(attr) {}
	Doc_Item add_item(const std::string& tag) {
		Doc_Item dci(new docbook_item(tag));
		_lst.push_back(dci);
		return dci;
	}
	Doc_Item add_item(const std::string& tag, const Poco::XML::AttributesImpl& attr) {
		Doc_Item dci(new docbook_item(tag, attr));
		_lst.push_back(dci);
		return dci;
	}
	void set_tag(const std::string& tag) {
		_tag = tag;
	}

	void append(const std::string& v, const std::string& tag = "");
	void append(int v, const std::string& tag = "");
	void append(double v, const std::string& tag = "", int precision = 3);

	virtual void write(Poco::XML::XMLWriter& writer) {
		if ( _attrs.getLength() != 0 )
			writer.startElement("", _tag, "", _attrs);
		else
			writer.startElement("", _tag, "");

		std::list< Doc_Item >::iterator it;
		for (it = _lst.begin(); it != _lst.end(); it++) {
			docbook_item* dbi = *it;
			dbi->write(writer);
		}
		writer.endElement("", _tag, "");
	}
protected:
	std::list< Doc_Item > _lst;
	std::string _tag;
	std::string _text;
	Poco::XML::AttributesImpl _attrs;
};
class docbook_txt: public docbook_item {
public:
	docbook_txt(const std::string& str): _text(str) {}
	virtual void write(Poco::XML::XMLWriter& writer) {
		writer.characters(Poco::XML::toXMLString(_text));
	}
private:
	std::string _text;
};
inline void docbook_item::append(const std::string& v, const std::string& tag) {
	if ( !tag.empty() ) {
		Doc_Item dci(new docbook_item(tag));
		_lst.push_back(dci);
		dci->append(v);
	} else {
		Doc_Item dci(new docbook_txt(v));
		_lst.push_back(dci);
	}
}
inline void docbook_item::append(int v, const std::string& tag) {
	std::stringstream ss;
	ss << v;
	append(ss.str(), tag);
}
inline void docbook_item::append(double v, const std::string& tag, int precision) {
	std::stringstream ss;
	ss.precision(precision);
	ss << v;
	append(ss.str(), tag);
}	
class docbook: public docbook_item {
public:
	docbook() {}
	docbook(const std::string& name): _name(name) {}
	/// set the name of the output xml file
	void set_name(const std::string& name) {
		_name = name;
	}
	/// write the document
	void write(void) {
		if ( _name.empty() )
			throw(std::runtime_error("no file name"));
		if ( _lst.empty() )
			throw(std::runtime_error("the document is empty"));
		std::ofstream ofs(_name.c_str());
		if ( !ofs.is_open() )
			throw(std::runtime_error("error opening output file"));
		Poco::XML::XMLWriter writer(ofs, Poco::XML::XMLWriter::WRITE_XML_DECLARATION | Poco::XML::XMLWriter::PRETTY_PRINT);
		writer.setNewLine("\n");
		writer.startDocument();
		writer.startDTD("article", "-//OASIS//DTD DocBook XML V4.5//IT\" \"http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd", "");
		writer.endDTD();
		std::list< Doc_Item >::iterator it;
		for (it = _lst.begin(); it != _lst.end(); it++) {
			docbook_item* dbi = *it;
			dbi->write(writer);
		}
		writer.endDocument();
	}
private:
	std::string _name;
};
#endif
