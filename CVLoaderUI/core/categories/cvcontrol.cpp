#include "cvcontrol.h"

namespace CV {
namespace Core {

int CVControl::count() const { 
	return _objects.length(); 
}

CVObject* CVControl::at(int i) const { 
	return _objects.at(i); 
}

void CVControl::remove(int i) { 
	_objects.removeAt(i); 
}

bool CVControl::isComplete() const { 
	foreach(CVObject* obj, _objects) {
		if (!obj->isValid()) {
			return false;
		}
	}
	return true;
};

void CVControl::load() { 
	foreach(CVObject* obj, _objects) {
		obj->load();
	}
};

void CVControl::insert(CVObject* obj, bool setPath) { 
	if (setPath) {
		obj->uri(uri());
		obj->init();
	}
	obj->controlType(type());
	_objects.append(obj); 
}

void CVObject::log(QString uri, QString note) const {
	Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
	e->control = _controlType;  
	e->object = _type;
	e->uri = uri;
	e->note = note;
	Core::CVJournal::add(e);
}

void CVObject::log(QString note) const {
	log(_uri, note);
}

}
}