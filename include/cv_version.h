#ifndef CV_VERSION_H
#define CV_VERSION_H

#define CHECK_VERSION "1.4.0a"
#define CHECK_DATE "01/01/2016"

#include <iostream> 
#include <sstream>

namespace CV {
namespace Version {
	
	static void print() {
		std::cout << std::endl << "Versione: " << CHECK_VERSION << " - " << CHECK_DATE << std::endl << std::endl;
	}

}
}

#endif //CV_VERSION_H
