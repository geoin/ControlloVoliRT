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
#include "core/categories/cvfolderinput.h"
#include "core/cvcore_utils.h"

#include "CVUtil/cvspatialite.h"

namespace CV {
namespace Core {

CVProjectManager::CVProjectManager(QObject* p) : QObject(p) {
}

void CVProjectManager::onNewProject() {
	QWidget* p = static_cast<QWidget*>(parent());
    GUI::Dialogs::CVProjectDialog dialog(p);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Core::CVProject* proj = new Core::CVProject(this);
	if (!dialog.getInput(*proj)) {
		GUI::CVMessageBox::message(p, tr("Errore!"), tr("Dati inseriti incompleti."));
		return;
	}

	GUI::CVScopedCursor c;
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
		} else {
			ctrl = new CVControl(CVControl::LIDAR_GPS_DATA, proj);
		}
        ctrl->uri(db);
        proj->insert(ctrl);

		ctrl = _fly(proj, false);
		proj->insert(ctrl);
		
        if (proj->type == CVProject::PHOTOGRAMMETRY) {
			ctrl = _orto(proj, false);
			proj->insert(ctrl);
		} else {
			ctrl = _rawLidar(proj, false);
			proj->insert(ctrl);

			ctrl = _finalLidar(proj, true);
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
		} else {
			ctrl = new CVControl(CVControl::LIDAR_GPS_DATA, proj);
		}
		ctrl->uri(db);
		proj->insert(ctrl);
		
		QStringList ids;
		proj->missionList(ids);
		foreach (const QString& id, ids) {
			CVMissionObject* obj = new CVMissionObject(ctrl, id);
			obj->missionType(ctrl->type());
			ctrl->insert(obj);
		}
		ctrl->load();
		
		ctrl = _fly(proj, true);
		proj->insert(ctrl);

		if (proj->type == CVProject::PHOTOGRAMMETRY) {
			ctrl = _orto(proj, true);
			proj->insert(ctrl);
		} else {
			ctrl = _rawLidar(proj, true);
			proj->insert(ctrl);
			
			ctrl = _finalLidar(proj, true);
			proj->insert(ctrl);
		}
		
		emit addProject(proj);
		_projects.append(CVProject::Ptr(proj));
	}
}   

//Take most data from other controls
CVControl* CVProjectManager::_fly(CVProject* proj, bool b) {
	CVControl::Type plan_t, fly_t;
	if (proj->type == CVProject::PHOTOGRAMMETRY) {
		plan_t = CVControl::PLAN;
		fly_t = CVControl::FLY;
	} else {
		plan_t = CVControl::LIDAR_PLAN;
		fly_t = CVControl::LIDAR_FLY;
	}

	CVControl* plan = proj->get(plan_t);

	CVControl* ctrl = new CVControl(fly_t, proj);
	ctrl->uri(proj->db());

	//NOTE: now used only in lidar projects. 
	/*CVShapeLayer* axis = new CVShapeLayer(proj);
	axis->uri(proj->path);
	axis->type(CVObject::AVOLOV);
	axis->controlType(fly_t);
	axis->table("AVOLOV");
	//axis->columns(QStringList() << "A_VOL_ENTE" << "A_VOL_DT" << "A_VOL_RID");
	ctrl->insert(axis);*/

	ctrl->insert(plan->at(2));
	ctrl->insert(plan->at(3), false);
	
	if (proj->type == CVProject::PHOTOGRAMMETRY) {
		CVFlyAttitude* fa = new CVFlyAttitude(proj);
		fa->uri(proj->path);
		ctrl->insert(fa);
	} else {
		CVFileInput* file = new CVCloudSampleInput(ctrl);
		file->uri(proj->path); 
		file->control(plan_t);
		file->object(CVObject::TEST_CLOUD);
		ctrl->insert(file, false);

		CVShapeLayer* controlPoint = new CVShapeLayer(proj);
		controlPoint->uri(proj->path);
		controlPoint->type(CVObject::CLOUD_CONTROL_POINTS);
		controlPoint->controlType(CVControl::LIDAR_FLY);
		controlPoint->table("CONTROL_CLOUD");
		//controlPoint->columns(QStringList());
		ctrl->insert(controlPoint);
	}

	if (b) { ctrl->load(); }
	return ctrl;
}

CVControl* CVProjectManager::_plan(CVProject* proj, bool b) {
	CVControl* ctrl = NULL;

	CVControl::Type plan_t;
	// Init camera
	if (proj->type == CVProject::PHOTOGRAMMETRY) {
		plan_t = CVControl::PLAN;
		ctrl = new CVControl(plan_t, proj);
		ctrl->uri(proj->db());
		CVCamera* cam = new CVCamera(ctrl);
		cam->isPlanning(true);
		ctrl->insert(cam);
	} else {
		plan_t = CVControl::LIDAR_PLAN;
		ctrl = new CVControl(plan_t, proj);
		ctrl->uri(proj->db());
		CVSensor* sensor = new CVSensor(ctrl);
		sensor->isPlanning(true);
		ctrl->insert(sensor);
	}
	
	CVShapeLayer* layer = new CVShapeLayerWithMeta(ctrl);
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

	CVFileInput* file = new CVDemInput(ctrl);
	file->uri(proj->path); 
	file->control(plan_t);
	file->object(CVObject::DEM);
	ctrl->insert(file, false);

	if (b) {
		ctrl->load();
	}
	return ctrl;
}

CVControl* CVProjectManager::_orto(CVProject* proj, bool b) {
	CVControl* ctrl = new CVControl(CVControl::ORTO, proj);
	ctrl->uri(proj->db());
	
	CVShapeLayer* layer = new CVShapeLayerWithMeta(ctrl);
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

CVControl* CVProjectManager::_rawLidar(CVProject* proj, bool load) {
	CVControl* ctrl = new CVControl(CVControl::LIDAR_RAW, proj);
	ctrl->uri(proj->db());

	CVShapeLayer* layer = new CVShapeLayer(ctrl);
	layer->columns(QStringList());
	layer->table("RAW_CONTROL_POINTS");
	layer->controlType(CVControl::LIDAR_RAW);
	layer->type(CVObject::RAW_CONTROL_POINTS);
	ctrl->insert(layer);

	CVFolderInput* folder = new CVFolderInput(ctrl);
	folder->controlType(CVControl::LIDAR_RAW);
	folder->type(CVObject::LIDAR_RAW_STRIP_DATA);
	folder->table("STRIP_RAW_DATA");
	ctrl->insert(folder);

	if (load) {
		ctrl->load();
	}
	return ctrl;
}

CVControl* CVProjectManager::_finalLidar(CVProject* proj, bool load) {
	CVControl* ctrl = new CVControl(CVControl::LIDAR_FINAL, proj);
	ctrl->uri(proj->db());

	CVFolderInput* folder = new CVFolderInput(ctrl);
	folder->controlType(CVControl::LIDAR_FINAL);
	folder->type(CVObject::LIDAR_RAW_STRIP_DATA);
	folder->table("STRIP_RAW_DATA"); //TODO change
	ctrl->insert(folder);

	if (load) {
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
