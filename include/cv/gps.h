#ifndef CV_GPS_H
#define CV_GPS_H

#include "Poco/SharedPtr.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeParser.h"

#include "CVUtil/ogrgeomptr.h"

namespace CV {
namespace GPS {

class Sample {
public:
	typedef Poco::SharedPtr<Sample> Ptr;

	Sample() : _nSat(0), _nBase(0), _pdop(0.0) {}

	inline void strip(const std::string& strip) { _strip = strip; }
	inline const std::string& strip() const { return _strip; }

	inline void mission(const std::string& mission) { _mission = mission; }

	inline void dateTime(const std::string& date, const std::string& time) {
		_date = date;
		_time = time;
	}

	inline const std::string& date() const { return _date; }

	inline const std::string& time() const { return _time; }

	inline void gpsData(int sat, int base, double pdop) {
		_nSat = sat;
		_nBase = base;
		_pdop = pdop;
	}

	inline void point(std::vector<unsigned char>& blob) {
		_point = blob;
	}

private:
	std::string _strip;
	std::string _mission;
	std::string _time;
	std::string _date;
	int _nSat;
	int _nBase;
	double _pdop;
	CV::Util::Geometry::OGRGeomPtr _point;
};

}
}

#endif