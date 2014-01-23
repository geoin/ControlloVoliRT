#include "cvjournal.h"
#include "core/sql/querybuilder.h"

#include "CVUtil/cvspatialite.h"

#include <QUuid>
#include <QDateTime>

namespace CV {
namespace Core {

void CVJournal::add(CVJournalEntry::Entry e) {
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(SQL::database.toStdString()); 
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return;
	}

	if (e->id.isEmpty()) {
		e->id = QUuid::createUuid().toString();
	}

	if (e->date.isEmpty()) {
		e->date = QDateTime::currentMSecsSinceEpoch();
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	q->insert(
		"JOURNAL", 
		QStringList() << "ID" << "DATE" << "URI" << "NOTE" << "CONTROL" << "OBJECT",
		QStringList() << "?1" << "?2" << "?3" << "?4" << "?5" << "?6",
		QVariantList() << e->id << e->date << e->uri << e->note << e->control << e->object 
	);
}

CVJournalEntry::Entry CVJournal::last(const QStringList& filters, const QVariantList& binds, const QStringList& order) {
	CV::Util::Spatialite::Connection cnn;
	try {
		cnn.open(SQL::database.toStdString()); 
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return CVJournalEntry::Entry();
	}

	CVJournalEntry::Entry e(new CVJournalEntry); 
	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	q->select(
		QStringList()  << "ID" << "DATE" << "URI" << "NOTE" << "CONTROL" << "OBJECT",
		QStringList()  << "JOURNAL", 
		filters,
		binds,
		order
	);

	return e;
}

} // namespace Core
} // namespace CV
