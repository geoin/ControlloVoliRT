#include "cvfileinput.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QUuid>
#include <QRegExp>

//TODO: this must be the base class, move all specialized logic in derived

namespace CV {
namespace Core {

CVFileInput::CVFileInput(QObject* p) : CVObject(p) {
	_isValid = false;
}

CVFileInput::~CVFileInput() {

}

bool CVFileInput::isValid() const {
	return _isValid;
}

bool CVFileInput::remove() { //TODO, should use id
	QFile::remove(_target);
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(QString(uri() + QDir::separator() + SQL::database).toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	bool ret = q->remove(
		"DEM", 
		QStringList(),
		QVariantList()
	);

	return ret;
}

bool CVFileInput::persist() {
	remove();

	QFile file(_origin);
	QFileInfo info(file);

	QString name = info.fileName();
	_target = uri() + QDir::separator() + name;
	QString _tmp = _target + "_tmp";
	QFile::rename(_target, _tmp);
	bool ret = file.copy(_target);
	if (!ret) {
		QFile::rename(_tmp, _target);
	} else {
		QFile::remove(_tmp);

		CV::Util::Spatialite::Connection cnn;
		QString db(uri() + QDir::separator() + SQL::database);
		try {
			cnn.open(db.toStdString());
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			return false;
		}

		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		bool ret = q->insert(
			"DEM", 
			QStringList() << "ID" << "URI",
			QStringList() << "?1" << "?2" ,
			QVariantList() << QUuid::createUuid().toString() << name
		);
		if (ret) {
			Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
			e->control = Core::CVControl::PLAN;  //TODO: needs control check, multiple position
			e->object = Core::CVObject::DEM;
			e->uri = _target;
			e->db = db;
			Core::CVJournal::add(e);
		} else {
			return false;
		}
	}

	return load();
}

bool CVFileInput::load() {
	_isValid = false;
	data().clear();
	
	if (_target.isEmpty()) {
		CV::Util::Spatialite::Connection cnn;
		try {
			cnn.open(QString(uri() + QDir::separator() + SQL::database).toStdString());
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			return false;
		}

		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "ID" << "URI",
			QStringList() << "DEM", 
			QStringList(),
			QVariantList(),
			QStringList(),
			1
		);
		if (!set.eof()) {
			QString name(set[1].toString().c_str());
			target(uri() + QDir::separator() + name);
		} else {
			return false;
		}
	}

	QFile file(_target);
    bool open = file.open(QIODevice::ReadOnly | QIODevice::Text);
	if (!open) {
		return false;
	}

	QTextStream str(&file);
	str.setCodec("UTF-8");

	int i = 0;
	int rows = 5; // TODO: handle this the right way
	while (i < rows) { 
		QString line = str.readLine();
		QStringList l = line.split(QRegExp("[\\t*\\s*]"), QString::SkipEmptyParts);
		if (l.size() < 2) {
			data() << "";
		} else {
			data() << l.at(1);
		}
		i++;
	}
	_isValid = data().size() == rows;
	return _isValid;
}

} // namespace Core
} // namespace CV
