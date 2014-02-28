#include "cvprojectmanager.h"

#include "gui/dialogs/cvprojectdialog.h"
#include "gui/helper/cvactionslinker.h"
#include "gui/cvgui_utils.h"

#include "core/sql/querybuilder.h"

#include "core/categories/cvcamera.h"
#include "core/categories/cvsensor.h"
#include "core/categories/cvflyattitude.h"
#include "core/categories/cvshapelayer.h"
#include "core/categories/cvfileinput.h"
#include "core/categories/cvmissionobject.h"
#include "core/cvcore_utils.h"

#include "CVUtil/cvspatialite.h"

namespace CV {
namespace Core {

CVProjectManager::CVProjectManager(QObject* p) : QObject(p) {
    
}

void CVProjectManager::onNewProject() {
    GUI::Dialogs::CVProjectDialog dialog(static_cast<QWidget*>(parent()));
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

	GUI::CVScopedCursor c;

    Core::CVProject* proj = new Core::CVProject(this);
	if (!dialog.getInput(*proj)) {
		return;
	}
	bool ret = proj->create(SQL::database);
	if (ret) {
		if (!proj->refPath.isEmpty()) {
			QString target = proj->path + QDir::separator() + "refval.xml";
			if (QFile::exists(target)) {
				QFile::remove(target);
			}
			QFile::copy(proj->refPath, target);
		}

		QString db = proj->path + QDir::separator() + SQL::database;

		CVControl* ctrl = _plan(proj, false);
		proj->insert(ctrl);
		
		if (proj->type == CVProject::PHOTOGRAMMETRY) {
			ctrl = new CVControl(CVControl::GPS_DATA, proj);
			ctrl->uri(db);
			proj->insert(ctrl);

			ctrl = _fly(proj, false);
			proj->insert(ctrl);

			ctrl = _orto(proj, false);
			proj->insert(ctrl);
		}

		emit addProject(proj);
		_projects.append(CVProject::Ptr(proj));
	}
}

void CVProjectManager::onNewMission() {

}

void CVProjectManager::onLoadProject() {
	QString proj = QFileDialog::getExistingDirectory(
		NULL, 
		tr("Seleziona cartella progetto"),
		Core::CVSettings::get("/paths/project").toString()
	);
	if (proj.isEmpty()) {
		return;
	}
	Core::CVSettings::set("/paths/project", proj);

	GUI::CVScopedCursor c;
	QDir dir(proj);
	if (dir.exists(SQL::database)) {
		CVProject* proj = new CVProject(this);
		QString db = proj->loadFrom(dir.absolutePath());
		if (db.isEmpty()) {
			return;
		}

		// Init controls
		CVControl* ctrl = NULL;

		ctrl = _plan(proj, true);
		proj->insert(ctrl);

		if (proj->type == CVProject::PHOTOGRAMMETRY) {
			ctrl = new CVControl(CVControl::GPS_DATA, proj);
			ctrl->uri(db);
			proj->insert(ctrl);

			QStringList ids;
			proj->missionList(ids);
			foreach (const QString& id, ids) {
				ctrl->insert(new CVMissionObject(ctrl, id));
			}
			ctrl->load();
			
			ctrl = _fly(proj, true);
			proj->insert(ctrl);

			ctrl = _orto(proj, true);
			proj->insert(ctrl);
		}
		
		emit addProject(proj);
		_projects.append(CVProject::Ptr(proj));
	}
}   

//Take most data from other controls
CVControl* CVProjectManager::_fly(CVProject* proj, bool b) {
	CVControl* plan = proj->get(CVControl::PLAN);
	CVControl* gps = proj->get(CVControl::GPS_DATA);

	CVControl* ctrl = new CVControl(CVControl::FLY, proj);
	ctrl->uri(proj->db());

	CVShapeLayer* axis = new CVShapeLayer(proj);
	axis->uri(proj->path);
	axis->type(CVObject::AVOLOV);
	axis->controlType(CVControl::FLY);
	axis->table("AVOLOV");
	axis->columns(QStringList() << "A_VOL_ENTE" << "A_VOL_DT" << "A_VOL_RID");
	ctrl->insert(axis);
	if (b) {
		axis->load();
	}

	ctrl->insert(plan->at(2));
	ctrl->insert(plan->at(3), false);

	CVFlyAttitude* fa = new CVFlyAttitude(proj);
	fa->uri(proj->path);
	ctrl->insert(fa);
	if (b) {
		fa->load();
	}
	return ctrl;
}

CVControl* CVProjectManager::_plan(CVProject* proj, bool b) {
	CVControl* ctrl = NULL;

	// Init camera
	if (proj->type == CVProject::PHOTOGRAMMETRY) {
		ctrl = new CVControl(CVControl::PLAN, proj);
		ctrl->uri(proj->db());
		CVCamera* cam = new CVCamera(ctrl);
		ctrl->insert(cam);
	} else {
		ctrl = new CVControl(CVControl::LIDAR_PLAN, proj);
		ctrl->uri(proj->db());
		CVSensor* sensor = new CVSensor(ctrl);
		ctrl->insert(sensor);
	}
	
	CVShapeLayer* layer = new CVShapeLayer(ctrl);
	layer->columns(QStringList() << "A_VOL_ENTE" << "A_VOL_DT" << "A_VOL_RID");
	layer->table("AVOLOP");
	layer->type(CVObject::AVOLOP);
	layer->controlType(CVControl::PLAN);
	ctrl->insert(layer);

	layer = new CVShapeLayer(ctrl);
	layer->columns(QStringList());
	layer->table("CARTO");
	layer->type(CVObject::CARTO);
	layer->controlType(CVControl::PLAN);
	ctrl->insert(layer);

	CVFileInput* file = new CVFileInput(ctrl);
	file->uri(proj->path); 
	ctrl->insert(file, false);

	if (b) {
		ctrl->load();
	}
	return ctrl;
}

CVControl* CVProjectManager::_orto(CVProject* proj, bool b) {
	CVControl* ctrl = new CVControl(CVControl::ORTO, proj);
	ctrl->uri(proj->db());

	CVShapeLayer* layer = new CVShapeLayer(ctrl);
	layer->columns(QStringList());
	layer->table("QUADRO_RT");
	layer->type(CVObject::QUADRO);
	layer->controlType(CVControl::ORTO);
	ctrl->insert(layer);

	layer = new CVShapeLayer(ctrl);
	layer->columns(QStringList());
	layer->table("CONTORNO_RT");
	layer->controlType(CVControl::ORTO);
	layer->type(CVObject::CONTOUR);
	ctrl->insert(layer);

	if (b) {
		ctrl->load();
	}
	return ctrl;
}
   
void CVProjectManager::onCloseProject() {
	//TODO
	_projects.clear();
}


} // namespace Core
} // namespace CV
