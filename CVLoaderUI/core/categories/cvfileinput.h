#ifndef CV_FILE_INPUT_H
#define CV_FILE_INPUT_H

#include "core/categories/cvcontrol.h"

#include <QStringList>

namespace CV {
namespace Core {

class CVFileInput : public CVObject {
	Q_OBJECT

public:
	CVFileInput(QObject *parent);
	virtual ~CVFileInput();

	virtual bool isValid() const;
	virtual bool persist();
	
	virtual bool remove();

	inline void origin(const QString& file) { _origin = file; }
	inline const QString& origin() const { return _origin; }

	inline void target(const QString& file) { _target = file; }
	inline const QString& target() const { return _target; }

	QStringList& data() { return _data; }

	inline void control(CVControl::Type t) { _control = t; }
	inline void object(CVObject::Type t) { _object = t; }

protected: 
	QString _table;
	CVControl::Type _control;
	CVObject::Type _object;

	QString _origin, _target;
	QStringList _data;
};

class CVDemInput : public CVFileInput {
	Q_OBJECT

public:
	CVDemInput(QObject* p) : CVFileInput(p) { 
		_object = CVObject::DEM;
		_control = CVControl::PLAN;
		_table = "DEM";
	}

	virtual bool load();

};

class CVCloudSampleInput : public CVFileInput {
	Q_OBJECT

public:
	CVCloudSampleInput(QObject* p) : CVFileInput(p) { 
		_object = CVObject::TEST_CLOUD;
		_control = CVControl::LIDAR_FLY;
		_table = "CLOUD_SAMPLE";
	}

	virtual bool load();

};

} // namespace Core
} // namespace CV

#endif // CV_FILE_INPUT_H
