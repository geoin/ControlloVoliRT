#ifndef CV_GUI_CVPROJECTOVERVIEWDETAIL_H
#define CV_GUI_CVPROJECTOVERVIEWDETAIL_H

#include <QWidget>

#include "core/cvproject.h"

class QPlainTextEdit;
class QVBoxLayout;
class QFormLayout;
class QComboBox;
class QLabel;

namespace CV {
namespace GUI {
namespace Details {

class CVProjectOverviewDetail : public QWidget {
	Q_OBJECT
public:
	CVProjectOverviewDetail(QWidget* p, Core::CVProject*);

	void addProjectInformations(QVBoxLayout*);
	void addNotesEditor(QVBoxLayout*);

signals:
	void updateNotes(const QString&);
	void updateScale(int);

public slots:
	void onSaveProject();
	void onEditProject(bool);

protected:
	virtual void showEvent(QShowEvent* event);

private:
	void _addScaleCombo(QFormLayout*);
	QLabel* _addDescr(QFormLayout*, QString label, QString descr);

	QPlainTextEdit* _notes;
	QComboBox* _scale;
	Core::CVProject* _controller;

	QLabel* _lastModified;
};

}
} // namespace GUI
} // namespace CV

#endif // CV_GUI_CVPROJECTOVERVIEWDETAIL_H
