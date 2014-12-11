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

	inline void table(const QString& t) { _table = t; }
	inline const QString& table() const { return _table; }

	QStringList& data() { return _data; }

	void setFolder(QString f) {
		_folder = f;
	} 

	const QString& folder() const { return _folder; } 

private:
	QString _table;
	QString _origin;
	QStringList _data;

	QString _folder;
};

} // namespace Core
} // namespace CV

#endif // CV_FOLDER_INPUT_H
