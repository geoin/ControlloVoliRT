#ifndef CV_LIDAR_COMMON_H
#define CV_LIDAR_COMMON_H

#include "Poco/SharedPtr.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeParser.h"

#include "dem_interpolate/geom.h"
#include "dem_interpolate/dsm.h"

#include "CVUtil/ogrgeomptr.h"

#include "common/util.h"

#include "gps.h"

namespace CV {
namespace Lidar {

class Sensor {
public:
	typedef Poco::SharedPtr<Sensor> Ptr;

	Sensor() {
		fov(0.0); ifov(0.0); freq(0.0); scan(0.0);
	}
	~Sensor() {}

	inline void fov(double fov) { _fov = fov; }
	inline double fov() const { return _fov; }

	inline void ifov(double ifov) { _ifov = ifov; }
	inline double ifov() const { return _ifov; }

	inline void freq(double freq) { _freq = freq; }
	inline double freq() const { return _freq; }

	inline void scan(double scan) { _scan = scan; }
	inline double scan() const { return _scan; }

	inline double halfGroundWidth() const {
		return tan(DEG_RAD(fov() / 2.0));
	}

private:
	double _fov;
	double _ifov;
	double _freq;
	double _scan;
};

class Axis {
public:
	typedef Poco::SharedPtr<Axis> Ptr;

	Axis() : _qt(0.0), _line(NULL) {}

	Axis(CV::Util::Geometry::OGRGeomPtr g, double qt) : _geom(g), _qt(qt), _line(NULL) {
		OGRLineString* ls = toLineString();
		
		_first = DPOINT(ls->getX(0), ls->getY(0), qt);
		_last = DPOINT(ls->getX(1), ls->getY(1), qt);
	} 

	double quota() const { return _qt; }

	const DPOINT& first() const { return _first; }
	const DPOINT& last() const { return _last; }

	inline OGRLineString* toLineString() {
		if (!_line) {
			OGRGeometry* og = _geom;
			_line = reinterpret_cast<OGRLineString*>(og);
		}
		return _line;
	}

	inline const OGRLineString* toLineString() const {
		const OGRGeometry* og = _geom;
		return reinterpret_cast<const OGRLineString*>(og);
	}

	bool isValid() const {
		return _line != NULL && _line->getNumPoints() == 2;
	}

	double length() const { return _line ? _line->get_Length() : 0.0; }

	double angle() const { return -(_last.angdir(_first)); }

	void stripName(const std::string& s) { _stripName = s; }
	const std::string& stripName() const { return _stripName; }

	void missionName(const std::string& m) { _missionName = m; }
	const std::string& missionName() const { return _missionName; }

	void addSample(GPS::Sample::Ptr s) { _samples.push_back(s); }
	
	void addFirstSample(GPS::Sample::Ptr s) { _firstSample = s; }
	void addLastSample(GPS::Sample::Ptr s) { _lastSample = s; }

	double averageSpeed() const;

private:

	DPOINT _first;
	DPOINT _last;
	
	double _qt;

	CV::Util::Geometry::OGRGeomPtr _geom;
	OGRLineString* _line;

	std::string _stripName;
	std::string _missionName;

	std::vector<GPS::Sample::Ptr> _samples;
	GPS::Sample::Ptr _firstSample;
	GPS::Sample::Ptr _lastSample; 
};

class Strip {
public:
	typedef Poco::SharedPtr<Strip> Ptr;

	Strip() : _yaw(0.0), _length(0.0), _polygon(NULL) {}

	Strip(CV::Util::Geometry::OGRGeomPtr g) : _yaw(0.0), _length(0.0), _polygon(NULL) {
		geom(g);
	}

	~Strip() {}
	
	void fromAxis(Axis::Ptr axis, DSM* dsm, double hWidth);

	inline OGRPolygon* toPolygon() {
		if (!_polygon) {
			OGRGeometry* og = _geom;
			_polygon = reinterpret_cast<OGRPolygon*>(og);
		}
		return _polygon;
	}

	inline const OGRPolygon* toPolygon() const {
		const OGRGeometry* og = _geom;
		return reinterpret_cast<const OGRPolygon*>(og);
	}

	void yaw(double yaw) { _yaw = yaw; } 
	double yaw() const { return _yaw; }

	void name(const std::string& s) { _name = s; }
	const std::string& name() const { return _name; }

	void missionName(const std::string& m) { _missionName = m; }
	const std::string& missionName() const { return _missionName; }

	CV::Util::Geometry::OGRGeomPtr geom() const { return _geom; }
	void geom(CV::Util::Geometry::OGRGeomPtr g) { _geom = g; }

	bool isParallel(Strip::Ptr other, int p = 10) const;
	bool intersect(Strip::Ptr other) const;
	int intersectionPercentage(Strip::Ptr other) const;

	Axis::Ptr axis() const { return _axis; }
	
private:
	std::string _missionName, _name;
	double _length;
	double _yaw;

	CV::Util::Geometry::OGRGeomPtr _geom;
	OGRPolygon* _polygon;

	Axis::Ptr _axis;
};

class Block {
public:
	typedef Poco::SharedPtr<Block> Ptr;
	
	void add(Strip::Ptr);
	
	CV::Util::Geometry::OGRGeomPtr geom() const { return _geom; }

private:
	CV::Util::Geometry::OGRGeomPtr _geom;
};

class Attitude {
public:
	typedef Poco::SharedPtr<Attitude> Ptr;

	Attitude() {
		bearing(0.0); height(0.0);
	}
	~Attitude() {}

	inline void bearing(double bearing) { _bearing = bearing; }
	inline double bearing() const { return _bearing; }

	inline void height(double height) { _height = height; }
	inline double height() const { return _height; }

private:
	double _bearing;
	double _height;
};
	
class Mission {
public:
	typedef Poco::SharedPtr<Mission> Ptr;

	Mission() {}
	~Mission() {}

private:

};

}
}

#endif //CV_LIDAR_H