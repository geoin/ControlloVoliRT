#ifndef CV_VERSION_H
#define CV_VERSION_H

#define CHECK_VERSION "1.4.0"
#define CHECK_DATE "03/02/2016"

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
