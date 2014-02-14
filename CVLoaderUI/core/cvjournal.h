#ifndef CV_JOURNAL_H
#define CV_JOURNAL_H

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
	QString db;
};

class CVJournal {
public:
	static void add(CVJournalEntry::Entry);
	static CVJournalEntry::EntryList last(const QString& db, const QStringList& filters, const QVariantList& binds, int num = 1);
};

} // namespace Core
} // namespace CV

#endif // CV_JOURNAL_H
