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
#include "core/categories/cvlidarfinalinput.h"
#include "core/categories/cvcsvinput.h"
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

			ctrl = _finalLidar(proj, false);
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
		Core::CVSettings::get(CV_PATH_PROJECT).toString()
	);
	if (proj.isEmpty()) {
		return;
	}
	Core::CVSettings::set(CV_PATH_PROJECT, proj);

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
			obj->type(CVObject::MISSION);
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

	ctrl->insert(plan->at(2));
	ctrl->insert(plan->at(3), false);
	
	if (proj->type == CVProject::PHOTOGRAMMETRY) {
		CVFlyAttitude* fa = new CVFlyAttitude(proj);
		fa->type(CVObject::ATTITUDE);
		fa->uri(proj->path);
		ctrl->insert(fa);
	} else {
		CVFolderInput* folder = new CVFolderInput(ctrl);
		folder->controlType(fly_t);
		folder->type(CVObject::RAW_STRIP_DATA);
		folder->table("RAW_STRIP_DATA");
		ctrl->insert(folder);

		CVFolderInput* folder1 = new CVFolderInput( ctrl );
		folder1->controlType( fly_t );
		folder1->type( CVObject::TEST_CLOUD );
		folder1->table( "CLOUD_SAMPLE" );
		ctrl->insert( folder1 );

		//CVFileInput* file = new CVCloudSampleInput(ctrl);
		//file->type(CVObject::TEST_CLOUD);
		//file->uri(proj->path); 
		//file->control(plan_t);
		//ctrl->insert(file, false);

		CVCsvInput* controlPoint = new CVCsvInput(proj);
		controlPoint->setTable("CONTROL_POINTS");
		controlPoint->uri(proj->path);
		controlPoint->type(CVObject::CLOUD_CONTROL_POINTS);
		controlPoint->controlType(CVControl::LIDAR_FLY);
		//controlPoint->table("CONTROL_CLOUD");
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
		//ctrl->uri(proj->db());
		CVCamera* cam = new CVCamera(ctrl);
		cam->isPlanning(true);
		cam->type(CVObject::CAMERA);
		ctrl->insert(cam);
	} else {
		plan_t = CVControl::LIDAR_PLAN;
		ctrl = new CVControl(plan_t, proj);
		//ctrl->uri(proj->db());
		CVSensor* sensor = new CVSensor(ctrl);
		sensor->type(CVObject::SENSOR);
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
	file->type(CVObject::DEM);
	file->uri(proj->path); 
	file->control(plan_t);
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

	CVCsvInput* layer = new CVCsvInput(ctrl);
	layer->setTable("RAW_CONTROL_POINTS");
	layer->controlType(CVControl::LIDAR_RAW);
	layer->type(CVObject::RAW_CONTROL_POINTS);
	ctrl->insert(layer);

	CVFolderInput* folder = new CVFolderInput(ctrl);
	folder->controlType(CVControl::LIDAR_RAW);
	folder->type(CVObject::RAW_STRIP_DATA);
	folder->table("RAW_STRIP_DATA");
	ctrl->insert(folder);

	if (load) {
		ctrl->load();
	}
	return ctrl;
}

CVControl* CVProjectManager::_finalLidar(CVProject* proj, bool load) {
	CVControl* ctrl = new CVControl(CVControl::LIDAR_FINAL, proj);
	ctrl->uri(proj->path);

	CVLidarFinalInput* fin = new CVLidarFinalInput(ctrl);
	fin->controlType(CVControl::LIDAR_FINAL);
	fin->type(CVObject::RAW_STRIP_DATA);
	ctrl->insert(fin);

	
	CVControl* fly = proj->get(CVControl::LIDAR_FLY);
	ctrl->insert(fly->at(0));
	ctrl->insert(fly->at(2));

	/*CVFolderInput* folder = new CVFolderInput(ctrl);
	folder->controlType(CVControl::LIDAR_FINAL);
	folder->type(CVObject::RAW_STRIP_DATA);
	folder->table("FINAL_INTENSITY"); 
	ctrl->insert(folder);

	folder = new CVFolderInput(ctrl);
	folder->controlType(CVControl::LIDAR_FINAL);
	folder->type(CVObject::RAW_STRIP_DATA);
	folder->table("FINAL_GROUND_ELL"); 
	ctrl->insert(folder);

	folder = new CVFolderInput(ctrl);
	folder->controlType(CVControl::LIDAR_FINAL);
	folder->type(CVObject::RAW_STRIP_DATA);
	folder->table("FINAL_GROUND_ORTO");
	ctrl->insert(folder);

	folder = new CVFolderInput(ctrl);
	folder->controlType(CVControl::LIDAR_FINAL);
	folder->type(CVObject::RAW_STRIP_DATA);
	folder->table("FINAL_OVERGROUND_ELL"); 
	ctrl->insert(folder);

	folder = new CVFolderInput(ctrl);
	folder->controlType(CVControl::LIDAR_FINAL);
	folder->type(CVObject::RAW_STRIP_DATA);
	folder->table("FINAL_OVERGROUND_ORTO"); 
	ctrl->insert(folder);

	folder = new CVFolderInput(ctrl);
	folder->controlType(CVControl::LIDAR_FINAL);
	folder->type(CVObject::RAW_STRIP_DATA);
	folder->table("FINAL_MDS"); 
	ctrl->insert(folder);

	folder = new CVFolderInput(ctrl);
	folder->controlType(CVControl::LIDAR_FINAL);
	folder->type(CVObject::RAW_STRIP_DATA);
	folder->table("FINAL_MDT"); 
	ctrl->insert(folder);*/

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
