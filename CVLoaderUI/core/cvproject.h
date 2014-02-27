#ifndef CV_CORE_CVPROJECT_H
#define CV_CORE_CVPROJECT_H

#include "core/categories/cvcontrol.h"

#include "CVUtil/cvspatialite.h"

#include <QMap>
#include <QDir>
#include <QFileDialog>
#include <QResource>
#include <QTextStream>
#include <QDateTime>
#include <QUuid>

namespace CV {
namespace Core {

class CVProject : public QObject {
    Q_OBJECT
public:
	typedef QSharedPointer<CVProject> Ptr;
    enum Type { INVALID = 0, PHOTOGRAMMETRY = 1, LIDAR = 2 };

    explicit CVProject(QObject* p = 0);

	bool persist();

	QString loadFrom(const QDir&); 
	bool create(const QString& db);
	
	void missionList(QStringList&);

	QString db() const { return _db; }

	void insert(CVControl*);
	CVControl* get(CVControl::Type);

	QDateTime creationDate();
	QDateTime lastModificationDate();
	QString projectNotes();

    QString id, name, path, notes, scale; //TODO, to be private
	long long timestamp;
    Type type;

private:
	bool _addResource(const QString& res, CV::Util::Spatialite::Connection&);

	QMap<CVControl::Type, CVControl*> _controls;
	QString _db;
};

} // namespace Core
} // namespace CV

#endif // CV_CORE_CVPROJECT_H
