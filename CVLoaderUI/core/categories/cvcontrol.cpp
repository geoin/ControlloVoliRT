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
	_objects.append(obj); 
}

}
}