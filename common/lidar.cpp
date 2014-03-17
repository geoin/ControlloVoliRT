#include "cv/lidar.h"

#include <sstream>

using namespace CV::Lidar;
using namespace CV::Util::Geometry;

void Strip::fromAxis(Axis::Ptr axis, DSM* dsm, double stripWidth) {
	MatOri m(0, 0, axis->angle());

	OGRGeomPtr gp_ = OGRGeometryFactory::createGeometry(wkbLinearRing);

	OGRLinearRing* gp = (OGRLinearRing*) ((OGRGeometry*) gp_);
	gp->setCoordinateDimension(2);

	for (int i = 0; i < 4; i++) {
		DPOINT pa = ( i < 2 ) ? axis->first() : axis->last();
		double x = ( i == 0 || i == 3 ) ? -stripWidth : stripWidth;
		DPOINT pd(x, 0, -1);
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

	OGRGeomPtr rg = OGRGeometryFactory::createGeometry(wkbPolygon);
	OGRPolygon* p = (OGRPolygon*) ((OGRGeometry*) rg);
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

double Axis::averageSpeed() const { 
	Poco::Timestamp start = _firstSample->timestamp();
	Poco::Timestamp end = _lastSample->timestamp();

	Poco::Timestamp stamp = end - start;
	Poco::Timestamp::TimeVal elapsed = stamp.epochMicroseconds();
	
	double speed_MS = 0;
	if(elapsed != 0) {
		speed_MS = length() / abs(elapsed / 1000.0 * 1000.0);
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

void ControlPoint::zDiffFrom(DSM* dsm) {
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
}