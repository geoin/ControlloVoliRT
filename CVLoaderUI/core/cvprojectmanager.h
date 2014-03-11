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
	CVControl* _plan(CVProject*, bool load);
	CVControl* _fly(CVProject*, bool load);
	CVControl* _orto(CVProject*, bool load);
	CVControl* _rawLidar(CVProject*, bool load);
	CVControl* _finalLidar(CVProject*, bool load);

	QList<CVProject::Ptr> _projects;
};

} // namespace Core
} // namespace CV

#endif // CV_CORE_CVPROJECTMANAGER_H
