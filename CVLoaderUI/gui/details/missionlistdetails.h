#ifndef CV_GUI_DETAILS_MISSIONSDETAILS_H
#define CV_GUI_DETAILS_MISSIONSDETAILS_H

#include "gui/cvbasetabwidget.h"
#include "core/categories/cvcategory.h"
#include "core/categories/cvmissionobject.h"

#include <QWidget>

class QPushButton;
class QMenu;
class QLabel;

namespace CV {
namespace GUI {
namespace Details {


// Toolbar contentente titolo e controlli

class CVMissionListToolbar : public QWidget {
    Q_OBJECT
public:
    explicit CVMissionListToolbar(QWidget* p = 0);

    QMenu* menu() const;

    void title(const QString&);
    QString title() const;

    inline QPushButton* previous() const { return _back; }
    inline QPushButton* next() const { return _next; }

signals:
    void viewNext();
    void viewPrevious();

private:
    QPushButton* _menu, * _next, * _back;
    QLabel* _title;
};

// Body, contiene le varie schede

class CVMissionListBody : public QWidget {
    Q_OBJECT
public:
    explicit CVMissionListBody(QWidget* p = 0);

private:
   QList<CVBaseTabWidget*> _missions;
};

// Widget principale

class CVMissionListDetails : public QWidget {
    Q_OBJECT
public:
    explicit CVMissionListDetails(QWidget* p = 0, CV::Core::CVCategory* category = 0);

	void add(Core::CVMissionObject*);

public slots:
    void onAddMission();
    void onRemoveMission();

	void onMissionChange();
    void onPreviousMission();
    void onNextMission();

private:
    CVMissionListToolbar* _bar;
    CVMissionListBody* _body;

	CV::Core::CVCategory* _category;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_MISSIONSDETAILS_H
