#include "cv/lidar.h"

#include <sstream>

using namespace CV::Lidar;
using namespace CV::Util::Geometry;

//TODO: creare la strisciata a blocchi
void Strip::fromAxis(Axis::Ptr axis, DSM* dsm, double thf) {
	MatOri m(0, 0, axis->angle());

	OGRGeomPtr gp_ = OGRGeometryFactory::createGeometry(wkbLinearRing);
	OGRGeometry* g = gp_;
	OGRLinearRing* gp = reinterpret_cast<OGRLinearRing*>(g);
	gp->setCoordinateDimension(2);

	for (int i = 0; i < 4; i++) {
		DPOINT pa = ( i < 2 ) ? axis->first() : axis->last();
		double x = ( i == 0 || i == 3 ) ? -thf : thf;
		DPOINT pd(x, 0, -1); //h = 1, DPOINT (h*thf, 0, -h)
		pd = m * pd;

		DPOINT pt;
		if (!dsm->RayIntersect(pa, pd, pt)) {
			if (!dsm->IsInside(pt.z)) {
				std::stringstream ss;
				ss << "la strisciata " << axis->stripName() << " della missione " << axis->missionName() << " cade al di fuori del dem";
				throw std::runtime_error(ss.str());
			}
		}
		gp->addPoint(pt.x, pt.y);
	}
	gp->closeRings();

	OGRGeometry* rg = OGRGeometryFactory::createGeometry(wkbPolygon);
	OGRPolygon* p = reinterpret_cast<OGRPolygon*>(rg);
	p->setCoordinateDimension(2);
	p->addRing(gp);

	geom(rg);
	missionName(axis->missionName());
	name(axis->stripName());
	yaw(axis->angle());

	_axis = axis;
}

bool Strip::isParallel(Strip::Ptr other, int p) const {
	double diff = yaw() - other->yaw();
	diff = fabs(diff > 180 ? 360 - diff : diff);
	return diff < p || diff > 180 - p;
}

bool Strip::intersect(Strip::Ptr other) const {
	OGRGeomPtr sourceGeom = geom();
	OGRGeomPtr targetGeom = other->geom();
	OGRBoolean b = sourceGeom->Intersect(targetGeom);
	return b ? true : false;
}

int Strip::intersectionPercentage(Strip::Ptr other) const {
	int p = 0;
	Util::Geometry::OGRGeomPtr sourceGeom = geom();
	OGRGeomPtr intersection = sourceGeom->Intersection(other->geom());
	if (intersection->getGeometryType() == wkbPolygon) {
		double srcMajorAxis, srcMinorAxis;
		get_elong(sourceGeom, yaw(), &srcMajorAxis, &srcMinorAxis);
		
		double targetMajorAxis, targetMinorAxis;
		get_elong(intersection, other->yaw(), &targetMajorAxis, &targetMinorAxis);

		p = static_cast<int>(100 * (targetMajorAxis / srcMajorAxis));
	}
	return p;
}

bool Strip::Intersection::contains(double x, double y) {
	OGRGeomPtr rg = OGRGeometryFactory::createGeometry(wkbPoint);
	OGRGeometry* g = rg;
	OGRPoint* p = reinterpret_cast<OGRPoint*>(g);
	p->setX(x);
	p->setY(y);
	return _geom->Contains(rg) ? true : false;
}

bool Strip::Intersection::contains(DPOINT& pt) {
	double x = pt.x;
	double y = pt.y;
	return contains(x, y);
}

Strip::Intersection::Ptr Strip::intersection(Strip::Ptr other) const {
	Util::Geometry::OGRGeomPtr sourceGeom = geom();
	Util::Geometry::OGRGeomPtr i = sourceGeom->Intersection(other->geom());
	Strip::Intersection::Ptr iPtr;
	iPtr.assign(new Strip::Intersection(i));
	return iPtr;
}

double Axis::averageSpeed() const { 
	Poco::Timestamp start = _firstSample->timestamp();
	Poco::Timestamp end = _lastSample->timestamp();

	Poco::Timestamp stamp = end - start;
	Poco::Timestamp::TimeVal elapsed = stamp.epochMicroseconds();
	
	double speed_MS = 0;
	if(elapsed != 0) {
		speed_MS = length() / abs(elapsed / (1000.0 * 1000.0));
	}
	return speed_MS;
}

void Block::add(Strip::Ptr strip) {
	if (!_geom) {
		_geom = strip->geom();
	} else {
		_geom = _geom->Union(strip->geom());
	}
}

ControlPoint::Status ControlPoint::zDiffFrom(DSM* dsm) {
	const OGRPoint* p = toPoint();
	double q = dsm->GetQuota(p->getX(), p->getY());
	if (q == Z_NOVAL) {
		_status = NO_VAL;
	} else if (q == Z_OUT) {
		_status = OUT_VAL;
	} else {
		_status = VALID;
		_diff = q  - _quota;
	}

	return _status;
}

double CloudStrip::computeDensity() {
	//TODO: use RAII handler 
	if (!_factory->Open(_cloudPath, false)) {
		throw std::runtime_error("Cannot open " + _cloudPath);
	}

	OGRPolygon* pol = _strip->toPolygon();
	double area = pol->get_Area();
	if (area == 0.0) {
		_factory->Close();
		throw std::runtime_error("Empty strip area");
	}

	DSM* dsm = _factory->GetDsm();
	unsigned int count = dsm->Npt();
	if (count == 0) {
		_factory->Close();
		throw std::runtime_error("Empty cloud");
	}

	_density = count / area;
	_factory->Close();

	return _density;
}

double CloudStrip::computeDensity(Sensor::Ptr s, double speed) {
	_density = 2*s->tanHalfFov() * speed/s->freq();
	return _density;
}