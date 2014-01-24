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
	
    void onNewMission();

private:
	CVCategory* _plan(CVProject*, bool load);

    QList<CVProject*> _projects;
};

} // namespace Core
} // namespace CV

#endif // CV_CORE_CVPROJECTMANAGER_H
