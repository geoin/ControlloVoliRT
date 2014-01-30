#ifndef CV_CORE_CVPROJECT_H
#define CV_CORE_CVPROJECT_H

#include "core/categories/cvcategory.h"

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
    enum Type { INVALID = 0, PHOTOGRAMMETRY, LIDAR };

    explicit CVProject(QObject* p = 0);

	QString loadFrom(const QDir&); 
	bool create(const QString& db);
	
	void missionList(QStringList&);

	QString db() const { return _db; }

	void insert(CVCategory*);
	CVCategory* get(CVCategory::Type);

    QString id, name, path, notes; //TODO, to be private
	long long timestamp;
    Type type;

private:
	QMap<CVCategory::Type, CVCategory*> _categories;
	QString _db;
};

} // namespace Core
} // namespace CV

#endif // CV_CORE_CVPROJECT_H
