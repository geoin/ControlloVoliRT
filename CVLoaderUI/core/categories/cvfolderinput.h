#ifndef CV_FOLDER_INPUT_H
#define CV_FOLDER_INPUT_H

#include "core/categories/cvcontrol.h"

#include <QStringList>

namespace CV {
namespace Core {

class CVFolderInput : public CVObject {
	Q_OBJECT

public:
	CVFolderInput(QObject *parent);
	~CVFolderInput();

	virtual bool isValid() const;
	virtual bool persist();
	virtual bool load();
	
	virtual bool remove();

	inline void origin(const QString& file) { _origin = file; }
	inline const QString& origin() const { return _origin; }

private:
	QString _origin;
};

} // namespace Core
} // namespace CV

#endif // CV_FOLDER_INPUT_H
