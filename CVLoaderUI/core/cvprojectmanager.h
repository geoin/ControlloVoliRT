#ifndef CV_CORE_CVPROJECTMANAGER_H
#define CV_CORE_CVPROJECTMANAGER_H

#include "cvproject.h"

#include <QObject>
#include <QList>

namespace CV {
namespace Core {

class CVProjectManager : public QObject {
    Q_OBJECT
public:
    explicit CVProjectManager(QObject *parent = 0);

signals:
    void addProject(Core::CVProject*);

public slots:
    void onNewProject();
    void onLoadProject();
    void onCloseProject();
    void onDeleteProject();

private:
    QList<CVProject*> _projects;
	static const QString _db;
};

} // namespace Core
} // namespace CV

#endif // CV_CORE_CVPROJECTMANAGER_H
