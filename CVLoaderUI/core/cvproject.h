#ifndef CV_CORE_CVPROJECT_H
#define CV_CORE_CVPROJECT_H

#include <QObject>

class QDir;

namespace CV {
namespace Core {

class CVProject : public QObject {
    Q_OBJECT
public:
    enum Type { INVALID = 0, PHOTOGRAMMETRY, LIDAR };

    explicit CVProject(QObject* p = 0);

	void loadFrom(const QDir&, const QString&);

    QString id, name, path, notes;
	long long timestamp;
    Type type;
};

} // namespace Core
} // namespace CV

#endif // CV_CORE_CVPROJECT_H
