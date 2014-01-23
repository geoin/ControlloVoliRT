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

    void getInput(Core::CVProject& proj);

public slots:
    void selectProjectFolder();

private:
    QLineEdit* _name, *_path;
    QPlainTextEdit* _note;
    QComboBox* _type;
};

} // namespace Dialogs
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DIALOGS_CVPROJECTDIALOG_H