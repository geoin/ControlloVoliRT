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

#include "Poco/XML/XMLWriter.h"
#include "Poco/SAX/AttributesImpl.h"
#include "Poco/SharedPtr.h"
#include <fstream>
#include <sstream>
#include <list>

class docbook_item;
typedef Poco::SharedPtr<docbook_item> Doc_Item;

// generic docbook item
class docbook_item {
public:
	/// default constructor
	docbook_item() {}
	/// constructor without attributes
	docbook_item(const std::string& tag): _tag(tag) {}
	/// constructor with attributes
	docbook_item(const std::string& tag, const Poco::XML::AttributesImpl& attr): _tag(tag), _attrs(attr) {}
	/// add an item without attributes
	Doc_Item add_item(const std::string& tag) {
		Doc_Item dci(new docbook_item(tag));
		_lst.push_back(dci);
		return dci;
	}
	/// add an item with attributes
	Doc_Item add_item(const std::string& tag, const Poco::XML::AttributesImpl& attr) {
		Doc_Item dci(new docbook_item(tag, attr));
		_lst.push_back(dci);
		return dci;
	}
	void set_tag(const std::string& tag) {
		_tag = tag;
	}
	std::string get_tag(void) const {
		return _tag;
	}

	Doc_Item get_item(const std::string& tag) {
		std::list< Doc_Item >::reverse_iterator it;
		for (it = _lst.rbegin(); it != _lst.rend(); it++) {
			Doc_Item dbi = *it;
			if ( dbi->get_tag() == tag )
				return dbi;
		}
		return Doc_Item(NULL);
	}

	/// append a 
	void append(const std::string& v, const std::string& tag = "");
	void append(int v, const std::string& tag = "");
	void append(double v, const std::string& tag = "", int precision = 3);
	void add_instr(const std::string& target, const std::string& data);

	/// write a generic item
	virtual void write(Poco::XML::XMLWriter& writer) {
		if ( _attrs.getLength() != 0 )
			writer.startElement("", _tag, "", _attrs);
		else
			writer.startElement("", _tag, "");

		_write(writer);
		writer.endElement("", _tag, "");
	}
protected:
	void _write(Poco::XML::XMLWriter& writer) {
		std::list< Doc_Item >::iterator it;
		for (it = _lst.begin(); it != _lst.end(); it++) {
			Doc_Item dbi = *it;
			dbi->write(writer);
		}
	}
	std::list< Doc_Item > _lst; /// list of child items
	std::string _tag; // tag of the item
	std::string _text; // test of the item
	Poco::XML::AttributesImpl _attrs;// attributes of the item
};
// text item
class docbook_txt: public docbook_item {
public:
	docbook_txt(const std::string& str): _text(str) {}
	virtual void write(Poco::XML::XMLWriter& writer) {
		writer.characters(Poco::XML::toXMLString(_text));
	}
private:
	std::string _text; /// text of the item
};
/// instruction item
class docbook_inst: public docbook_item {
public:
	docbook_inst(const std::string& target, const std::string& data):
	  _target(target), _data(data) {}
	virtual void write(Poco::XML::XMLWriter& writer) {
		writer.processingInstruction(_target, _data);
	}
private:
	std::string _target;
	std::string _data;
};
// append text tag is optional to indicate a specific renderer
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
/// append an integer item
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
// append a floating point item
inline void docbook_item::add_instr(const std::string& target, const std::string& data) {
	Doc_Item dci(new docbook_inst(target, data));
	_lst.push_back(dci);
}
class docbook: public docbook_item {
public:
	docbook(): _dtd_url("http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd") {}
	docbook(const std::string& name): _name(name), _dtd_url("http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd") {}
	/// set the name of the output xml file
	void set_name(const std::string& name) {
		_name = name;
	}
	void set_dtd(const std::string& dtd) {
		_dtd_url = dtd;
	}
	std::string name(void) const { return _name; }
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
		std::stringstream ss;
		ss << "-//OASIS//DTD DocBook XML V4.5//EN\" ";
		ss << "\"" << _dtd_url;
		writer.startDTD("article", ss.str(), ""); //"-//OASIS//DTD DocBook XML V4.5//EN\" \"http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd", "");
		writer.endDTD();
		/// write the item list
		_write(writer);
		writer.endDocument();
	}
private:
	std::string _name;
	std::string _dtd_url;
};
#endif
