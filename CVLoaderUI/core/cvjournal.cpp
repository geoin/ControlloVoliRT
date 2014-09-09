#include "cvjournal.h"
#include "core/sql/querybuilder.h"
#include "core/categories/cvcontrol.h"

#include "CVUtil/cvspatialite.h"

#include <QUuid>
#include <QDateTime>

namespace CV {
namespace Core {

void CVJournal::add(CVJournalEntry::Entry e) {
	CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
	if (e->id.isEmpty()) {
		e->id = QUuid::createUuid().toString();
	}

	if (!e->date.isValid()) {
		e->date = QDateTime::currentDateTimeUtc();
	}

	Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);
	q->insert(
		"JOURNAL", 
		QStringList() << "ID" << "DATE" << "URI" << "NOTE" << "CONTROL" << "OBJECT",
		QStringList() << "?1" << "?2" << "?3" << "?4" << "?5" << "?6",
		QVariantList() << e->id << e->date << e->uri << e->note << e->control << e->object 
	);
}

CVJournalEntry::EntryList CVJournal::last(const QString& db, const QStringList& filters, const QVariantList& binds, int num) {
	CVJournalEntry::EntryList list;
 
	try {
		CV::Util::Spatialite::Connection& cnn = SQL::Database::get();
		Core::SQL::Query::Ptr q = Core::SQL::QueryBuilder::build(cnn);

		Util::Spatialite::Recordset set = q->select(
			QStringList()  << "ID" << "DATE" << "URI" << "NOTE" << "CONTROL" << "OBJECT",
			QStringList()  << "JOURNAL", 
			filters,
			binds,
			QStringList() << "DATE DESC",
			1
		);

		while (!set.eof()) {
			CVJournalEntry::Entry e(new CVJournalEntry);
			e->id = set[0].toString().c_str();
			e->date = QDateTime::fromMSecsSinceEpoch(set[1].toInt64());
			e->uri = set[2].toString().c_str();
			e->note = set[3].toString().c_str();
			
			list.append(e);
			set.next();
		}
	} catch (CV::Util::Spatialite::spatialite_error& err) {
		Q_UNUSED(err)
		return CVJournalEntry::EntryList();
	}
	return list;
}

} // namespace Core
} // namespace CV
