#ifndef QUERYBUILDER_H
#define QUERYBUILDER_H

#include "CVUtil/cvspatialite.h"

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QSharedPointer>
#include <QVariant>

namespace CV {
namespace Core {
namespace SQL {

static const QString database = "geo.sqlite";

class Query {
public:
	typedef QSharedPointer<Query> Ptr;
	typedef QVariant::Type Type; 

	Query(CV::Util::Spatialite::Connection& cnn) : _connection(cnn), _stm(_connection) {}

	bool remove(const QString& tab, const QStringList& where, const QVariantList& binds);
	bool update(const QString& tab, const QStringList& values, const QStringList& where, const QVariantList& binds);
	bool insert(const QString& tab, const QStringList& fields, const QStringList& values, const QVariantList& binds);
	CV::Util::Spatialite::Recordset select(const QStringList& what, const QStringList& from, const QStringList& where, const QVariantList& binds, const QStringList& order = QStringList(), int = 0);
	
	void bindValue(const int& index, const QVariant& val);

private:
	CV::Util::Spatialite::Connection& _connection;
	CV::Util::Spatialite::Statement _stm;
};

class QueryBuilder {
public:
	static Query::Ptr build(CV::Util::Spatialite::Connection& cnn);

private:
	
};

} // namespace SQL
} // namespace Core
} // namespace CV

#endif // QUERYBUILDER_H
