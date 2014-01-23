#ifndef CV_JOURNAL_H
#define CV_JOURNAL_H

#include <QSharedPointer>
#include <QObject>
#include <QList>
#include <QStringList>
#include <QVariantList>

namespace CV {
namespace Core {

struct CVJournalEntry {
	typedef QSharedPointer<CVJournalEntry> Entry;
	typedef QList<Entry> EntryList;

	CVJournalEntry() {}

	QString id, date, uri, note, control, object;
};

class CVJournal {
public:
	static void add(CVJournalEntry::Entry);
	static CVJournalEntry::Entry last(const QStringList& filters, const QVariantList& binds, const QStringList& order = QStringList());
};

} // namespace Core
} // namespace CV

#endif // CV_JOURNAL_H
