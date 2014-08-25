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

#include <limits>

#define CV_DISABLE_COPY(T)  \
	T (const T&);			\
	T& operator= (const T&)

#define CV_DISABLE_DEFAULT_CTOR(T) \
	T()

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
		_line = toLineString();
		
		_first = DPOINT(_line->getX(0), _line->getY(0), qt);
		_last = DPOINT(_line->getX(1), _line->getY(1), qt);
	} 

	double quota() const { return _qt; }

	const DPOINT& first() const { return _first; }
	const DPOINT& last() const { return _last; }

	inline OGRLineString* toLineString() {
		OGRGeometry* og = _geom;
		return reinterpret_cast<OGRLineString*>(og);
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

	Strip() : _yaw(0.0), _length(0.0) {}

	Strip(CV::Util::Geometry::OGRGeomPtr g) : _yaw(0.0), _length(0.0) {
		geom(g);
	}

	~Strip() {}

	class Intersection {
		public:
			typedef Poco::SharedPtr<Intersection> Ptr;
			Intersection(CV::Util::Geometry::OGRGeomPtr g) : _geom(g) {}

			bool contains(double x, double y);
			bool contains(DPOINT&);

		private:
			CV::Util::Geometry::OGRGeomPtr _geom;
	};
	
	void fromAxis(Axis::Ptr axis, DSM* dsm, double hWidth);

	inline OGRPolygon* toPolygon() {
		OGRGeometry* og = _geom;
		return reinterpret_cast<OGRPolygon*>(og);
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
	Strip::Intersection::Ptr intersection(Strip::Ptr other) const;

	Axis::Ptr axis() const { return _axis; }
	bool hasAxis() const { return !_axis.isNull(); }
	
private:
	std::string _missionName, _name;
	double _length;
	double _yaw;

	CV::Util::Geometry::OGRGeomPtr _geom;

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

class ControlPoint {
public:
	enum Status { UNKNOWN = 0, VALID = 1, NO_VAL, OUT_VAL };

	typedef Poco::SharedPtr<ControlPoint> Ptr;

	ControlPoint(CV::Util::Geometry::OGRGeomPtr g) : _geom(g), _quota(0.0), _diff(std::numeric_limits<double>::max()), _status(UNKNOWN) {}
	ControlPoint(CV::Util::Geometry::OGRGeomPtr g, double q) : _geom(g), _quota(q), _diff(std::numeric_limits<double>::max()), _status(UNKNOWN) {}

	inline OGRPoint* toPoint() {
		OGRGeometry* og = _geom;
		return reinterpret_cast<OGRPoint*>(og);
	}

	inline const OGRPoint* toPoint() const {
		const OGRGeometry* og = _geom;
		return reinterpret_cast<const OGRPoint*>(og);
	}

	void quota(double q) { _quota = q; }
	double quota() const { return _quota; }

	void name(const std::string& n) { _name = n; }
	const std::string& name() const { return _name; }

	void zDiffFrom(DSM*);
	double zDiff() const { return _diff; } 

	inline bool isValid() const { return _status == VALID; }

	inline Status status() const { return _status; }

private:
	CV::Util::Geometry::OGRGeomPtr _geom;
	std::string _name;
	double _quota;

	double _diff;
	Status _status;
};

class DSMHandler {
public:
	DSMHandler(DSM_Factory* dsm) : _dsm(dsm) {
		_d = dsm ? dsm->GetDsm() : NULL;
	}

	~DSMHandler() { 
		if (_dsm != NULL) {
			_dsm->Close();
		}
	}

	bool isNull() const { return _dsm == NULL; }

	DSM* operator -> () {
		return _d;
	}

	const DSM* operator -> () const {
		return _d;
	}

private:
	DSM_Factory* _dsm;
	DSM* _d;

	CV_DISABLE_DEFAULT_CTOR(DSMHandler);
	CV_DISABLE_COPY(DSMHandler);
};

class CloudStrip {
public:
	typedef Poco::SharedPtr<CloudStrip> Ptr;
	typedef Poco::SharedPtr<DSM_Factory> DSMPtr;

	CloudStrip(Strip::Ptr p) : _strip(p), _density(0.0) {
		_factory.assign(new DSM_Factory);
		_factory->SetEcho(MyLas::first_pulse);
	}

	~CloudStrip() { 
		if (_factory->GetDsm() != NULL) {
			_factory->Close();
		}
	}

	const std::string& name() const { return _strip->name(); } 

	double density() const { return _density; }
	void density(double d) { _density = d; }

	Strip::Ptr strip() { return _strip; }

	void cloudPath(const std::string& path) { _cloudPath = path; }
	
	void computeDensity();

	DSM_Factory* dsm(bool open = true) { 
		bool ret = true;
		if (open) {
			ret = _factory->Open(_cloudPath, false);
		}
		return ret ? _factory.get() : NULL; 
	}

private:
	double _density;
	Strip::Ptr _strip;
	DSMPtr _factory;

	std::string _cloudPath;

	CV_DISABLE_DEFAULT_CTOR(CloudStrip);
	CV_DISABLE_COPY(CloudStrip);
};

}
}

#endif //CV_LIDAR_H