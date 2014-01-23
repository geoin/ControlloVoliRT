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

	void uri(const QString& uri) { _uri = uri; }

	void loadFrom(const QDir&); 
	bool create(const QString& db);

	void insert(CVCategory*);
	CVCategory* get(CVCategory::Type);

    QString id, name, path, notes; //TODO, to be private
	long long timestamp;
    Type type;

private:
	QString _uri;
	QMap<CVCategory::Type, CVCategory*> _categories;
};

} // namespace Core
} // namespace CV

#endif // CV_CORE_CVPROJECT_H
