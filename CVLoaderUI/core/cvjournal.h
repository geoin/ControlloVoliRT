#ifndef CV_JOURNAL_H
#define CV_JOURNAL_H

#include "core/categories/cvcontrol.h"

#include <QSharedPointer>
#include <QObject>
#include <QList>
#include <QStringList>
#include <QVariantList>
#include <QDateTime>

namespace CV {
namespace Core {

struct CVJournalEntry {
	typedef QSharedPointer<CVJournalEntry> Entry;
	typedef QList<Entry> EntryList;

	CVJournalEntry() {}

	QString id, uri, note;
	short control, object;
	QDateTime date;
};

class CVJournal {
public:
	static void add(CVJournalEntry::Entry);
	static CVJournalEntry::EntryList lastN(const QStringList& filters, const QVariantList& binds, int num = 1);
	static CVJournalEntry::Entry last(CVControl::Type t, CVObject::Type o);
};

} // namespace Core
} // namespace CV

#endif // CV_JOURNAL_H
