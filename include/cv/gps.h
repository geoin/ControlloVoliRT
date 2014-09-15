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
	inline const std::string mission() { return _mission; }

	inline void dateTime(const std::string& date, const std::string& time) {
		_date = date;
		_time = time;

		_timestamp();
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

	inline void point(CV::Util::Geometry::OGRGeomPtr blob) {
		_point = blob;
	}

	inline CV::Util::Geometry::OGRGeomPtr geom() { return _point; }

	inline const OGRPoint* toPoint() const {
		const OGRGeometry* ptr = _point;
		return reinterpret_cast<const OGRPoint*>(ptr);
	}

	inline OGRPoint* toPoint() {
		OGRGeometry* ptr = _point;
		return reinterpret_cast<OGRPoint*>(ptr);
	}

	Poco::Timestamp timestamp() const {
		return _ts;
	}

	//TODO: as in photo, but as to be changed
	std::string time2;

private:
	void _timestamp() {
		std::stringstream stream;
		std::string timeStart = time();
		std::string day = date();
		stream << day << " " << timeStart;  
		int d;
		Poco::DateTime date;
		if (Poco::DateTimeParser::tryParse("%Y/%m/%d %H:%M:%s", stream.str(), date, d)) {
			_ts = date.timestamp();
		}
	}

	std::string _strip;
	std::string _mission;

	std::string _time;
	std::string _date;
	Poco::Timestamp _ts;
	
	int _nSat;
	int _nBase;
	double _pdop;
	
	CV::Util::Geometry::OGRGeomPtr _point;
};

}
}

#endif