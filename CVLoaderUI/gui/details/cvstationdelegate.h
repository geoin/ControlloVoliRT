#ifndef CV_GUI_CVSTATIONDELEGATE_H
#define CV_GUI_CVSTATIONDELEGATE_H

#include <QWidget>
#include <QContextMenuEvent>
#include <QLabel>

namespace CV {
namespace GUI {
namespace Details {

class CVStationDelegate : public QWidget {
    Q_OBJECT
public:
    explicit CVStationDelegate(QWidget* = 0);

	void title(const QString& title) { _title->setText(title); }

signals:
	void remove();

protected:
	virtual void contextMenuEvent(QContextMenuEvent*);

private:
	QLabel* _title;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVSTATIONDELEGATE_H
