#ifndef CV_GUI_DIALOGS_CVMISSIONDIALOG_H
#define CV_GUI_DIALOGS_CVMISSIONDIALOG_H

#include <QDialog>

class QLineEdit;
class QPlainTextEdit;

namespace CV {
namespace GUI {
namespace Dialogs {

class CVMissionDialog : public QDialog {
    Q_OBJECT
public:
    explicit CVMissionDialog(QWidget* p = 0);

    void getInput(QString&, QString&);

private:
    QLineEdit* _name;
    QPlainTextEdit* _note;
};

} // namespace Dialogs
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DIALOGS_CVMISSIONDIALOG_H
