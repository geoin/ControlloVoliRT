#ifndef CV_LIDAR_COMMON_H
#define CV_LIDAR_COMMON_H

#include "Poco/SharedPtr.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeParser.h"

#include "common/geom.h"
#include "dem_interpolate/dsm.h"

#include "CVUtil/ogrgeomptr.h"

#include "common/util.h"

#include "gps.h"

#include <limits>
#include <iostream>


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
	
	inline void speed(double speed) { _speed = speed; }
	inline double speed() const { return _speed; }

	inline double tanHalfFov() const {
		return tan(Conv::ToRad(fov() / 2.0));
	}

private:
	double _fov;
	double _ifov;
	double _freq;
	double _scan;
	double _speed;
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

	void id(unsigned int id) { _id = id; }
	unsigned int id() { return _id; }

	double quota() const { return _qt; }
	CV::Util::Geometry::OGRGeomPtr geom() const { return _geom; }

	const DPOINT& first() const { return _first; }
	const DPOINT& last() const { return _last; }
    const std::vector<DPOINT>& BB() const  { return _bb; }
    long npoints() const {return _npoints;}
    long firstPoints() const {return _first_points;}
    long lastPoints() const {return _last_points;}
    long singlePoints() const {return _single_points;}
    long interPoints() const {return _inter_points;}

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

	double angle() const { return _last.angdir(_first); }

	void stripName(const std::string& s) { _stripName = s; }
	const std::string& stripName() const { return _stripName; }

	void missionName(const std::string& m) { _missionName = m; }
	const std::string& missionName() const { return _missionName; }

	void addSample(GPS::Sample::Ptr s) { _samples.push_back(s); }
	
	void addFirstSample(GPS::Sample::Ptr s) { _firstSample = s; }
	void addLastSample(GPS::Sample::Ptr s) { _lastSample = s; }

	double averageSpeed() const;

    bool fromCloud(const std::string& las, double angle = 0., int echo = 0);

private:
	DPOINT _first;
	DPOINT _last;
    std::vector<DPOINT> _bb;
    long _npoints;
    long _first_points, _last_points, _single_points, _inter_points;

	
	double _qt;
	unsigned int _id;

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

	Strip() : _yaw(0.0), _length(0.0), _density(0.0), _isValid(true) {}

	Strip(CV::Util::Geometry::OGRGeomPtr g) : _yaw(0.0), _length(0.0), _density(0.0), _isValid(true) {
		geom(g);
	}

	~Strip() {}

	class Intersection {
		public:
			typedef Poco::SharedPtr<Intersection> Ptr;
			Intersection(CV::Util::Geometry::OGRGeomPtr g) : _geom(g) {}

			bool contains(double x, double y);
			bool contains(DPOINT&);

			void getAxisFromGeom(double& a, double& b, double& beta);
            void toBuffer(double = -150);

			OGRPolygon* toPolygon() { 
				OGRGeometry* g_ = _geom;
				return reinterpret_cast<OGRPolygon*>(g_);
			}

		private:
			CV::Util::Geometry::OGRGeomPtr _geom;
	};
	
	void fromAxis(Axis::Ptr axis, DSM* dsm, double tanHalfFov);
	void fromLineRing(Axis::Ptr axis, OGRLinearRing* gp);

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

	double computeDensity(Sensor::Ptr, DSM* dsm);
    double computeDensity(DSM* dsm);
    double computeDensity(long np);
	double density() const { return _density; }
	void density(double d) { _density = d; }

	bool isValid() const { return _isValid; }
	void isValid(bool b) { _isValid = b;}
	
private:
	bool _isValid;

	std::string _missionName, _name;
	double _length;
	double _yaw;
	double _density;

	CV::Util::Geometry::OGRGeomPtr _geom;

	Axis::Ptr _axis;
};

class Block {
public:
	typedef Poco::SharedPtr<Block> Ptr;
	Block() {}
	Block(CV::Util::Geometry::OGRGeomPtr g) : _geom(g) {}
	
	void add(Strip::Ptr);

	void split(std::vector<CV::Util::Geometry::OGRGeomPtr>&);
	
	CV::Util::Geometry::OGRGeomPtr geom() const { return _geom; }

	inline const OGRMultiPolygon* toMultiPolygon() const {
		const OGRGeometry* og = _geom;
		return reinterpret_cast<const OGRMultiPolygon*>(og);
	}

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
    enum Status { UNKNOWN = 0, VALID = 1, NO_VAL, OUT_VAL, WEAK_VAL };

	typedef Poco::SharedPtr<ControlPoint> Ptr;

	ControlPoint(const double& x, const double& y) : _geom(x, y), _quota(0.0), _diff(std::numeric_limits<double>::max()), _status(UNKNOWN) {}
	ControlPoint(const double& x, const double& y, const double& q) : _geom(x, y), _quota(q), _diff(std::numeric_limits<double>::max()), _status(UNKNOWN) {}

	void quota(double q) { _quota = q; }
	double quota() const { return _quota; }

	void name(const std::string& n) { _name = n; }
	const std::string& name() const { return _name; }

    void cloud(const std::string& n) { _cloud = n; }
    const std::string& cloud() const { return _cloud; }

    Status zDiffFrom(DSM* dsm);
	double zDiff() const { 
		if (_status == UNKNOWN) { throw std::runtime_error("Uninitialized control point"); }
		return _diff; 
	} 

	inline bool isValid() const { return _status == VALID; }

	inline Status status() const { return _status; }

	const DPOINT& point() {
		return _geom;
	}

private:
	DPOINT _geom;
	std::string _name;
    std::string _cloud;
	double _quota;

	double _diff;
	Status _status;
};

class CloudStrip {
public:
	typedef Poco::SharedPtr<CloudStrip> Ptr;
	typedef Poco::SharedPtr<DSM_Factory> DSMPtr;

	CloudStrip(Strip::Ptr p) : _factory(NULL), _strip(p), _density(0.0) {
        init(0., 2);
	}

	~CloudStrip() { 
		release();
	}

    void init(double ang, int echo) {
		if (!_factory.get()) {
			_factory.assign(new DSM_Factory);

		}
        _factory->SetEcho(echo); //single_pulse);
        _factory->SetAngle(ang);
        //std::cout << "STRIP INIT " << ang << " " << std::endl;
	}

	void release() {
		if (_factory.get()) {
			_factory->Close();
			_factory.assign(NULL);
		}
	}

	const std::string& name() const { return _strip->name(); } 

	double density() const { return _density; }
	void density(double d) { _density = d; }

	Strip::Ptr strip() { return _strip; }

	void cloudPath(const std::string& path) { _cloudPath = path; }
	
	double computeDensity();

	DSM_Factory* open(bool tria) { 
		bool ret = _factory->Open(_cloudPath, false, tria);
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

class DSMHandler {
public:
    DSMHandler(CloudStrip::Ptr strip, double ang, int echo, bool tria = true) {
		_strip = strip;

        strip->init(ang, echo);
//        std::cout << "...opening " << std::endl;
		_dsm = strip->open(tria);
		_d = _dsm->GetDsm();
//        std::cout << "...read " << _d->Npt() << std::endl;
	}

	~DSMHandler() { 
		release();
	}

	bool isNull() const { return _dsm == NULL; }

	DSM* operator -> () {
		return _d;
	}

	const DSM* operator -> () const {
		return _d;
	}

	void release() {
		_strip->release();
	}

	std::string name() const {
		return _strip->name();
	}

private:
	DSM_Factory* _dsm;
	DSM* _d;

	CloudStrip::Ptr _strip;

	CV_DISABLE_DEFAULT_CTOR(DSMHandler);
	CV_DISABLE_COPY(DSMHandler);
};

}
}

#endif //CV_LIDAR_H
