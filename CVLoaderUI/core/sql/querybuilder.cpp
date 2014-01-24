#include "querybuilder.h"

#include <QRegExp>
#include <QDateTime>

#include <assert.h>
#include <algorithm>

namespace CV {
namespace Core {
namespace SQL {

	bool Query::insert(const QString& tab, const QStringList& fields, const QStringList& values, const QVariantList& binds) {
		QString query("INSERT INTO %1 (%2) VALUES (%3)");
		query = query.arg(tab, fields.join(", "), values.join(", "));
		
		try {
			_stm.prepare(query.toStdString()); 
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			return false;
		}

		assert(values.length() >= binds.length());
		QRegExp reg("\\?([0-9]{1,2})");

		try {
			for (int i = 0; i < binds.length(); ++i) {
				int pos = reg.indexIn(values.at(i));
				if (pos < 0) {
					continue;
				}

				int val = reg.cap(1).toInt();
				QVariant b = binds.at(val - 1);
				bindValue(val, b);
			}

			_stm.execute();
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			return false;
		}

		return true;
	}

	bool Query::update(const QString& tab, const QStringList& values, const QStringList& where, const QVariantList& binds) {
		QString query("UPDATE %1 SET %2 WHERE %3");
		query = query.arg(tab, values.join(", "), where.join(" AND "));
		
		try {
			_stm.prepare(query.toStdString()); 
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			return false;
		}

		assert(where.length() + values.size() >= binds.length());
		QRegExp reg("\\?([0-9]{1,2})");

		try {
			int i = 0;
			int len = std::min(binds.size(), values.size());
			for (; i < len; ++i) {
				int pos = reg.indexIn(values.at(i));
				if (pos < 0) {
					continue;
				}

				int val = reg.cap(1).toInt();
				QVariant b = binds.at(val - 1);
				bindValue(val, b);
			}

			len = std::min(where.length(), binds.length());
			for (int i = 0; i < len; ++i) {
				int pos = reg.indexIn(where.at(i));
				if (pos < 0) {
					continue;
				}

				int val = reg.cap(1).toInt();
				QVariant b = binds.at(val - 1);
				bindValue(val, b);
			}

			_stm.execute();
		} catch (CV::Util::Spatialite::spatialite_error& err) {
			Q_UNUSED(err)
			return false;
		}

		return true;
	}
	
	//TODO: too basic - needs order by 
	CV::Util::Spatialite::Recordset Query::select(
		const QStringList& what, 
		const QStringList& from, 
		const QStringList& where, 
		const QVariantList& binds, 
		const QStringList& order, 
		int limit) 
	{
		//catch outside
		QString query("SELECT %1 FROM %2");
		if (where.length()) {
			query.append(" WHERE %3");
		}

		if (order.length()) {
			query.append(" ORDER BY %4");
		}

		if (limit) {
			query.append(" LIMIT " + QString::number(limit));
		}

		query = query.arg(what.join(", "), from.join(", "), where.join(" AND "), order.join(", "));

		_stm.prepare(query.toStdString()); 
		
		assert(where.length() >= binds.length());
		QRegExp reg("\\?([0-9]{1,2})");

		for (int i = 0; i < binds.length(); ++i) {
			int pos = reg.indexIn(where.at(i));
			if (pos < 0) {
				continue;
			}

			int val = reg.cap(1).toInt();
			QVariant b = binds.at(val - 1);
			bindValue(val, b);
		}

		return _stm.recordset();
	}

	//let me bind from Qt types
	void Query::bindValue(const int& index, const QVariant& val) {
		QVariant::Type t = val.type();
		switch (t) {
			case QVariant::String:	
				_stm[index] = val.toString().toStdString();
				break;
			case QVariant::Int:
			case QVariant::LongLong:
				_stm[index] = val.toLongLong();
				break;
			case QVariant::Double:
				_stm[index] = val.toDouble();
				break;
			case QVariant::Date:
			case QVariant::DateTime:
				_stm[index] = val.toDateTime().toMSecsSinceEpoch();
				break;
		}
	}

	Query::Ptr QueryBuilder::build(CV::Util::Spatialite::Connection& cnn) {
		Query::Ptr ptr(new Query(cnn));
		return ptr;
	}

} // namespace SQL
} // namespace Core
} // namespace CV
