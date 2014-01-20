#ifndef CV_GUI_DETAILS_CVCAMERADETAIL_H
#define CV_GUI_DETAILS_CVCAMERADETAIL_H

#include <QWidget>
#include <QMap>
#include <QFileInfo>
#include <QScopedPointer>

class QPlainTextEdit;
class QLineEdit;

namespace CV {
namespace GUI {
namespace Details {

class CVCameraDetail : public QWidget {
    Q_OBJECT
public:
    explicit CVCameraDetail(QWidget *parent = 0);

    QLineEdit* lineEdit(QWidget* p, const QPalette&);

signals:
    void cameraInput(const QString& uri);

public slots:
    void onCameraInput(const QString& uri);

    void onLoadCamParameters();
    void onEditCamParameters();
    void onClearCamParameters();

protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);

private:
    QScopedPointer<QFileInfo> _file;

    QMap<QString, QLineEdit*> _params;
    QString _uri;

    QPlainTextEdit* _note;
};

} // namespace Details
} // namespace GUI
} // namespace CV

#endif // CV_GUI_DETAILS_CVCAMERADETAIL_H
