#ifndef CVFLYAXIS_P_H
#define CVFLYAXIS_P_H

#include "core/categories/cvshapelayer.h"
#include "gui/cvshapeviewer.h"

#include "cvbasedetail.h"

#include <QWidget>
#include <QScopedPointer>
#include <QFileInfo>
#include <QList>

class QLabel;
class QTableWidget;

namespace CV {
namespace GUI {
namespace Details {

class CVFlyAxis_p : public CVBaseDetail {
	Q_OBJECT

public:
	CVFlyAxis_p(QWidget *parent, Core::CVObject*);
	~CVFlyAxis_p();
	
	virtual void clearAll();
	virtual void searchFile();
	virtual void importAll(QStringList&);

	inline Core::CVShapeLayerWithMeta* layer() const { return static_cast<Core::CVShapeLayerWithMeta*>(controller()); }

signals:
	void refreshGrid();

public slots:
	void onComboSelected(const QString&);
	void populateTable();

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
	void _clearRefCols();

    QScopedPointer<QFileInfo> _file;
	QString _uri;
	QList<QComboBox*> _editors;

	QTableWidget* table;
	ShapeViewer* _shape;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CVFLYAXIS_P_H
