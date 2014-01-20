#include "cvprojectmanager.h"

#include "gui/dialogs/cvprojectdialog.h"
#include "gui/helper/cvactionslinker.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QDir>
#include <QFileDialog>
#include <QResource>
#include <QTextStream>
#include <QDateTime>
#include <QUuid>

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

    Core::CVProject* proj = new Core::CVProject(this);
    dialog.getInput(*proj);

	bool ret = create(proj);
	if (ret) {
		emit addProject(proj);
		_projects.append(proj);
	}
}

void CVProjectManager::onLoadProject() {
	QString proj = QFileDialog::getExistingDirectory(NULL, tr("Seleziona cartella progetto"));
	QDir dir(proj);
	if (dir.exists(_db)) {
		CVProject* proj = new CVProject(this);
		proj->loadFrom(dir.absolutePath(), _db);
		
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

bool CVProjectManager::create(Core::CVProject* proj) {
	QDir dir;
	dir.cd(proj->path);
	bool ret = dir.mkdir(proj->name);
	if (!ret) {
		return false;
	}

	dir.cd(proj->name);

	QString db = dir.absolutePath() + dir.separator() + _db;

	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.create(db.toStdString()); 
		if (cnn.check_metadata() == CV::Util::Spatialite::Connection::NO_SPATIAL_METADATA) {
			cnn.initialize_metdata(); 
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	bool isValid = cnn.is_valid();
	if (!isValid) {
		return false;
	}

	QResource sql(":/sql/db.sql");
	QFile res(sql.absoluteFilePath());
	if (!res.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}
	QTextStream str(&res);
	QString query = str.readAll();
	

	QStringList tables = query.split(";", QString::SkipEmptyParts);
	cnn.begin_transaction();
	foreach(QString t, tables) {
		try {
			cnn.execute_immediate(t.simplified().toStdString());
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			cnn.rollback_transaction();
			return false;
		}
	}
	cnn.commit_transaction();

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	q->insert(
		"JOURNAL", 
		QStringList() << "ID" << "DATE" << "URI" << "NOTE" << "CONTROL",
		QStringList() << "?1" << "?2" << "?3" << "?4" << "?5",
		QVariantList() << QUuid::createUuid().toString() << QDateTime::currentMSecsSinceEpoch() << proj->path << proj->notes << proj->type
	);

	return true;
}

} // namespace Core
} // namespace CV
