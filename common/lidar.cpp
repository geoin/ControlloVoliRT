#include "cv/lidar.h"

#include <iostream>
#include <sstream>

using namespace CV::Lidar;
using namespace CV::Util::Geometry;

//TODO: creare la strisciata a blocchi
void Strip::fromAxis(Axis::Ptr axis, DSM* dsm, double thf) {
	MatOri m(0, 0, -axis->angle());

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
			if (!dsm->IsValid(pt.z)) {
				this->isValid(false);
			}
		}

		gp->addPoint(pt.x, pt.y);
	}
	gp->closeRings();

	OGRGeometry* rg = OGRGeometryFactory::createGeometry(wkbPolygon);
	OGRPolygon* p = reinterpret_cast<OGRPolygon*>(rg);
	p->setCoordinateDimension(2);
	p->addRing(gp);

	this->geom(rg);
	this->missionName(axis->missionName());

	std::stringstream str;
	str << axis->stripName();// << " (" << axis->id() << ")";

	this->name(str.str());
	this->yaw(axis->angle());

	_axis = axis;
}

void Strip::fromLineRing(Axis::Ptr axis, OGRLinearRing* gp) {
	OGRGeometry* rg = OGRGeometryFactory::createGeometry(wkbPolygon);
	OGRPolygon* p = reinterpret_cast<OGRPolygon*>(rg);
	p->setCoordinateDimension(2);
	p->addRing(gp);

	this->geom(rg);
	this->missionName(axis->missionName());

	std::stringstream str;
	str << axis->stripName();// << " (" << axis->id() << ")";

	this->name(str.str());
	this->yaw(axis->angle());

	_axis = axis;
}

bool Strip::isParallel(Strip::Ptr other, int p) const {
	double diff = yaw() - other->yaw();
	diff = fabs(diff > 180 ? 360 - diff : diff);
	return diff < p || diff > 180 - p;
}

bool Strip::intersect(Strip::Ptr other) const {
	OGRGeomPtr sourceGeom = this->geom();
	OGRGeomPtr targetGeom = other->geom();
	OGRBoolean b = sourceGeom->Intersect(targetGeom);
	return b ? true : false;
}

int Strip::intersectionPercentage(Strip::Ptr other) const {
	int p = 0;
	Util::Geometry::OGRGeomPtr sourceGeom = this->geom();
	OGRGeomPtr intersection = sourceGeom->Intersection(other->geom());
	if (intersection->getGeometryType() == wkbPolygon) {
		double srcAxis1, srcAxis2;
		get_ellipse_elong(sourceGeom, srcAxis1, srcAxis2);
		
		double targetAxis1, targetAxis2;
		get_ellipse_elong(intersection, targetAxis1, targetAxis2);

		p = static_cast<int>(100 * (targetAxis2 / srcAxis2));
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
	if (elapsed != 0) {
		speed_MS = length() / abs(elapsed / (1000.0 * 1000.0));
	}
	return speed_MS;
}

bool Axis::fromCloud(const std::string& las) {
    DSM_Factory f;
	if (!f.Open(las, false, false)) {
		return false;
	}

	f.GetDsm()->getMajorAxis(_first, _last);

	_geom = OGRGeometryFactory::createGeometry(wkbLineString);
	
	OGRGeometry* og = _geom;
	reinterpret_cast<OGRLineString*>(og)->addPoint(_first.x, _first.y);
	reinterpret_cast<OGRLineString*>(og)->addPoint(_last.x, _last.y);
	
	_line = toLineString();
	
	f.Close();
	return true;
}

void Block::add(Strip::Ptr strip) {
	if (!_geom) {
		_geom = strip->geom();
	} else {
		_geom = _geom->Union(strip->geom());
	}
}

ControlPoint::Status ControlPoint::zDiffFrom(DSM* dsm) {
	double q = dsm->GetQuota(_geom.x, _geom.y);
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
	if (!_factory->Open(_cloudPath, false)) {
		throw std::runtime_error("Cannot open " + _cloudPath);
	}

	OGRPolygon* pol = _strip->toPolygon();
	double area = pol->get_Area();
	if (area == 0.0) {
		_factory->Close();
		_factory.assign(new DSM_Factory);
		throw std::runtime_error("Empty strip area");
	}

	DSM* dsm = _factory->GetDsm();
	unsigned int count = dsm->Npt();
	if (count == 0) {
		_factory->Close();
		_factory.assign(new DSM_Factory);
		throw std::runtime_error("Empty cloud");
	}

	_density = count / area;
	_factory->Close();
	_factory.assign(new DSM_Factory);

	return _density;
}

double Strip::computeDensity(Sensor::Ptr s, DSM* dsm) {
	DPOINT p = axis()->first();
	double q = dsm->GetQuota(p.x, p.y);
	if (q == Z_NOVAL) {
		_density = Z_NOVAL;
	} else if (q == Z_OUT) {
		_density = Z_OUT;
	} else {
		double H = axis()->quota() - q;
		double L = 2 * H * s->tanHalfFov();
		_density = L * s->speed()/s->freq();
	}
	return _density;
}

double Strip::computeDensity(DSM* dsm) {
	double area = toPolygon()->get_Area();
	if (area == 0.0) {
		throw std::runtime_error("Empty strip area");
	}

	unsigned int count = dsm->Npt();
	if (count == 0) {
		throw std::runtime_error("Empty cloud");
	}

	_density = count / area;
	return _density;
}
