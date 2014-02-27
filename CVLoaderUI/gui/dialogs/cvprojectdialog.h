#ifndef CV_GUI_DIALOGS_CVPROJECTDIALOG_H
#define CV_GUI_DIALOGS_CVPROJECTDIALOG_H

#include "core/cvproject.h"

#include <QDialog>

class QLineEdit;
class QPlainTextEdit;
class QComboBox;

namespace CV {
namespace GUI {
namespace Dialogs {

class CVProjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit CVProjectDialog(QWidget *parent = 0);

    bool getInput(Core::CVProject& proj);

public slots:
    void selectConfiguration();
    void selectProjectFolder();
	void changeProjectType(int);

private:
    QLineEdit* _name, *_path, *_conf;
    QPlainTextEdit* _note;
    QComboBox* _type, * _scale;
};

} // namespace Dialogs
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DIALOGS_CVPROJECTDIALOG_H
