#include "cvprojectmanager.h"

#include "gui/dialogs/cvprojectdialog.h"
#include "gui/helper/cvactionslinker.h"
#include "gui/cvgui_utils.h"

#include "core/sql/querybuilder.h"

#include "core/categories/cvcamera.h"

#include "CVUtil/cvspatialite.h"

namespace CV {
namespace Core {

const QString CVProjectManager::_db = "geo.db";

CVProjectManager::CVProjectManager(QObject* p) : QObject(p) {
    
}

void CVProjectManager::onNewProject() {
    GUI::Dialogs::CVProjectDialog dialog;
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

	GUI::CVScopedCursor c;

    Core::CVProject* proj = new Core::CVProject(this);
    dialog.getInput(*proj);

	bool ret = proj->create(_db);
	if (ret) {
		CVCategory* cat = new CVCategory(CVCategory::PLAN, proj);

		CVCamera* cam = new CVCamera(cat);
		cam->uri(proj->path + QDir::separator() + proj->name + QDir::separator() + _db); //TODO: change path in project
		
		cat->insert(cam);

		proj->insert(cat);

		proj->insert(new CVCategory(CVCategory::GPS_DATA, proj));
		proj->insert(new CVCategory(CVCategory::FLY, proj));

		emit addProject(proj);
		_projects.append(proj);
	}
}

void CVProjectManager::onLoadProject() {
	QString proj = QFileDialog::getExistingDirectory(NULL, tr("Seleziona cartella progetto"));

	GUI::CVScopedCursor c;
	QDir dir(proj);
	if (dir.exists(_db)) {
		CVProject* proj = new CVProject(this);
		proj->loadFrom(dir.absolutePath(), _db);
		// Init category
		CVCategory* cat = new CVCategory(CVCategory::PLAN, proj);

		// Init camera
		CVCamera* cam = new CVCamera(cat);
		cam->uri(proj->path + QDir::separator() + _db);
		cam->load();
		
		cat->insert(cam);

		proj->insert(cat);

		proj->insert(new CVCategory(CVCategory::GPS_DATA, proj));
		proj->insert(new CVCategory(CVCategory::FLY, proj));
		
		emit addProject(proj);
		_projects.append(proj);
	}
}   
    
void CVProjectManager::onDeleteProject() {
	//TODO
	QString proj = QFileDialog::getExistingDirectory(NULL, tr("Seleziona cartella progetto"));
	QDir dir(proj);
	if (dir.exists(_db)) {
		dir.cdUp();
		dir.remove(proj);
	}
}
   
void CVProjectManager::onCloseProject() {
	//TODO
}


} // namespace Core
} // namespace CV
