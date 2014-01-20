#ifndef CVBASEINPUTVERIFIER_H
#define CVBASEINPUTVERIFIER_H

#include <QString>
#include <QObject>

namespace CV {
namespace Core {

class CVBaseInputVerifier : public QObject {
	Q_OBJECT

public:
	CVBaseInputVerifier(QObject* p);
	~CVBaseInputVerifier();

	virtual bool verifyURI(const QString&) = 0;
	virtual bool verifyData(const QString&) = 0;

private:
	
};

} // namespace Core
} // namespace CV

#endif // CVBASEINPUTVERIFIER_H
