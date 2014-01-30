#include "cvprojectmanager.h"

#include "gui/dialogs/cvprojectdialog.h"
#include "gui/helper/cvactionslinker.h"
#include "gui/cvgui_utils.h"

#include "core/sql/querybuilder.h"

#include "core/categories/cvcamera.h"
#include "core/categories/cvshapelayer.h"
#include "core/categories/cvfileinput.h"
#include "core/categories/cvmissionobject.h"

#include "CVUtil/cvspatialite.h"

namespace CV {
namespace Core {

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

	bool ret = proj->create(SQL::database);
	if (ret) {
		CVCategory* cat = _plan(proj, false);
		proj->insert(cat);
		
		cat = new CVCategory(CVCategory::GPS_DATA, proj);
		cat->uri(proj->path + QDir::separator() + SQL::database);
		proj->insert(cat);

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
		QString db = proj->loadFrom(dir.absolutePath());

		// Init categories
		CVCategory* cat = _plan(proj, true);
		proj->insert(cat);

		cat = new CVCategory(CVCategory::GPS_DATA, proj);
		cat->uri(db);
		proj->insert(cat);

		QStringList ids;
		proj->missionList(ids);
		foreach(const QString& id, ids) {
			cat->insert(new CVMissionObject(cat, id));
		}
		cat->load();

		proj->insert(new CVCategory(CVCategory::FLY, proj));
		
		emit addProject(proj);
		_projects.append(proj);
	}
}   

CVCategory* CVProjectManager::_plan(CVProject* proj, bool b) {
	CVCategory* cat = new CVCategory(CVCategory::PLAN, proj);
	cat->uri(proj->db());

	// Init camera
	CVCamera* cam = new CVCamera(cat);
	cat->insert(cam);
	
	CVShapeLayer* layer = new CVShapeLayer(cat);
	layer->columns(QStringList() << "A_VOL_ENTE" << "A_VOL_DT" << "A_VOL_RID");
	layer->table("AVOLOP");
	cat->insert(layer);

	layer = new CVShapeLayer(cat);
	layer->columns(QStringList() << "count(*)");
	layer->table("CARTO");
	cat->insert(layer);

	CVFileInput* file = new CVFileInput(cat);
	file->uri(proj->path); 
	cat->insert(file, false);

	if (b) {
		cat->load();
	}

	return cat;
}
   
void CVProjectManager::onCloseProject() {
	//TODO
}


} // namespace Core
} // namespace CV
