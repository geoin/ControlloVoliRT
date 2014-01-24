#include "cvprojectmanager.h"

#include "gui/dialogs/cvprojectdialog.h"
#include "gui/helper/cvactionslinker.h"
#include "gui/cvgui_utils.h"

#include "core/sql/querybuilder.h"

#include "core/categories/cvcamera.h"
#include "core/categories/cvshapelayer.h"
#include "core/categories/cvfileinput.h"

#include "CVUtil/cvspatialite.h"

namespace CV {
namespace Core {

CVProjectManager::CVProjectManager(QObject* p) : QObject(p) {
    
}

//TODO: load and creation must share code!

void CVProjectManager::onNewProject() {
    GUI::Dialogs::CVProjectDialog dialog;
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

	GUI::CVScopedCursor c;

    Core::CVProject* proj = new Core::CVProject(this);
    dialog.getInput(*proj);

	bool ret = proj->create(SQL::database);
	if (ret) {
		CVCategory* cat = _plan(proj, false);
		proj->insert(cat);

		proj->insert(new CVCategory(CVCategory::GPS_DATA, proj));
		proj->insert(new CVCategory(CVCategory::FLY, proj));

		emit addProject(proj);
		_projects.append(proj);
	}
}

void CVProjectManager::onNewMission() {

}

void CVProjectManager::onLoadProject() {
	QString proj = QFileDialog::getExistingDirectory(NULL, tr("Seleziona cartella progetto"));
	if (proj.isEmpty()) {
		return;
	}

	GUI::CVScopedCursor c;
	QDir dir(proj);
	if (dir.exists(SQL::database)) {
		CVProject* proj = new CVProject(this);
		proj->loadFrom(dir.absolutePath());

		// Init categories
		CVCategory* cat = _plan(proj, true);
		proj->insert(cat);

		proj->insert(new CVCategory(CVCategory::GPS_DATA, proj));
		proj->insert(new CVCategory(CVCategory::FLY, proj));
		
		emit addProject(proj);
		_projects.append(proj);
	}
}   

CVCategory* CVProjectManager::_plan(CVProject* proj, bool b) {
	CVCategory* cat = new CVCategory(CVCategory::PLAN, proj);

	// Init camera
	CVCamera* cam = new CVCamera(cat);
	cam->uri(proj->path + QDir::separator() + SQL::database);
	if (b) {
		cam->load();
	}
	cat->insert(cam);
	
	CVShapeLayer* layer = new CVShapeLayer(cat);
	layer->uri(proj->path + QDir::separator() + SQL::database); 
	layer->columns(QStringList() << "A_VOL_ENTE" << "A_VOL_DT" << "A_VOL_RID");
	layer->table("AVOLOP");
	if (b) {
		layer->load();
	}
	cat->insert(layer);

	layer = new CVShapeLayer(cat);
	layer->uri(proj->path + QDir::separator() + SQL::database); 
	layer->columns(QStringList() << "count(*)");
	layer->table("CARTO");
	if (b) {
		layer->load();
	}
	cat->insert(layer);

	CVFileInput* file = new CVFileInput(cat);
	file->uri(proj->path); 
	if (b) {
		file->load();
	}
	cat->insert(file);

	return cat;
}
   
void CVProjectManager::onCloseProject() {
	//TODO
}


} // namespace Core
} // namespace CV
