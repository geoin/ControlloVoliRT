#include "cvflyattitude.h"

#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QUuid>
#include <QRegExp>
#include <QTextStream>

#include <assert.h>

//TODO: this must be the base class, move all specialized logic in derived

namespace CV {
namespace Core {

CVFlyAttitude::CVFlyAttitude(QObject* p) : CVObject(p) {
	_isValid = false;
	_count = 0;
}

CVFlyAttitude::~CVFlyAttitude() {

}

bool CVFlyAttitude::isValid() const {
	return _isValid;
}

bool CVFlyAttitude::remove() { //TODO, should use id
	_data.clear();

	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	bool ret = q->remove(
		"ASSETTI", 
		QStringList(),
		QVariantList()
	);

	_count = 0;
	return ret;
}

bool CVFlyAttitude::persist() {
	_count = 0;
	QFile file(_origin);
	bool open = file.open(QIODevice::ReadOnly | QIODevice::Text);
	if (!open) {
		emit persisted();
		return false;
	}
	remove();

	QTextStream str(&file);
	str.setCodec("UTF-8");

	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		emit persisted();
		return false;
	}
	cnn.begin_transaction();
	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);

	QStringList rec;
	rec << "ID" << "STRIP" << "NAME" << "PX" << "PY" << "PZ" << "OMEGA" << "PHI" << "KAPPA";
	
	QStringList ph;
	ph << "?1" << "?2" << "?3" << "?4" << "?5" << "?6" << "?7" << "?8" << "?9";

	QMap<QString, int> attData;
	while (!str.atEnd()) { 
		QString line = str.readLine();
		if (line.isEmpty()) { 
			continue;
		}
		QStringList l = line.split(QRegExp("[\\t*\\s*]"), QString::SkipEmptyParts);
		if (l.size() != 7) {
			continue;
		}

		QString key = l.at(0);
		QStringList id = key.split("_");
		if (id.length() < 2) { //TODO: check, this is probably not true anymore (len must be >= 2)
			continue;
		}

		QVariantList data;
		QString part = id.at(0);
		QString photo = id.at(1);

		attData[part]++; //QT container set default at zero, so i need no initialization
		data << key << part << photo;

		bool ok;
		qreal val;
		for (int i = 1; i < l.size(); ++i) {
			val = l.at(i).toDouble(&ok);
			if (!ok) { break; }
			data << val;
		}
		if (!ok) { continue; }

		assert(rec.size() == ph.size() && rec.size() == data.size());
		if (!q->insert(
				"ASSETTI",
				rec,
				ph,
				data
			)) {
			cnn.rollback_transaction();
			emit persisted();
			return false;
		}
		emit itemInserted(++_count);
	}

	cnn.commit_transaction();

	int num = 0;
	foreach(int i, attData) {
		num += i;
	}
	_data << QString::number(attData.size()) << QString::number(num);

	/*Core::CVJournalEntry::Entry e(new Core::CVJournalEntry);
	e->control = Core::CVControl::FLY;  
	e->object = Core::CVObject::ATTITUDE;
	e->uri = _origin;
	//e->db = uri();
	Core::CVJournal::add(e);*/

	log(_origin, "");

	emit persisted();
	return true;
}

bool CVFlyAttitude::load() {
	_isValid = false;
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(uri().toStdString());
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}
	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);

	QMap<QString, int> attData;
	try {
		CV::Util::Spatialite::Recordset set = q->select(
			QStringList() << "STRIP" << "count(*)",
			QStringList() << "ASSETTI GROUP BY STRIP", 
			QStringList(),
			QVariantList()
		);
	
		while (!set.eof()) {
			attData[QString(set[0].toString().c_str())] = set[1].toInt();
			set.next();
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return false;
	}

	int num = 0;
	foreach(int i, attData) {
		num += i;
	}
	_data << QString::number(attData.size()) << QString::number(num);
	_isValid = _data.size() == 2;
	return _isValid;
}

} // namespace Core
} // namespace CV
