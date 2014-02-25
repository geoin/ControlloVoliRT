#include "cvtreenodedelegate.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QLabel>
#include <QHBoxLayout>

namespace CV {
namespace GUI {

CVTreeNodeDelegate::CVTreeNodeDelegate(QWidget* p, const QStringList& s) : QWidget(p) {
    QString labelTxt;
    if (s.length()) {
        labelTxt = s.at(0);
    }
    QLabel* label = new QLabel(labelTxt, this);
    label->setIndent(4);

   _addWidget(label);
}

CVTreeNodeDelegate::CVTreeNodeDelegate(QWidget* p, QWidget* view) : QWidget(p) {
    _addWidget(view);
}

void CVTreeNodeDelegate::dragEnterEvent(QDragEnterEvent* ev) {
    ev->ignore();
}

void CVTreeNodeDelegate::dragMoveEvent(QDragMoveEvent* ev) {
    ev->accept();
}

void CVTreeNodeDelegate::dragLeaveEvent(QDragLeaveEvent* ev) {
    ev->accept();
}

void CVTreeNodeDelegate::dropEvent(QDropEvent* ev) {
    ev->accept();
}

void CVTreeNodeDelegate::_addWidget(QWidget* v) {
    QHBoxLayout* box = new QHBoxLayout;
    box->setContentsMargins(0, 0, 0, 0);
    box->addWidget(v);
    setLayout(box);
    setAcceptDrops(true);
}

} // namespace GUI
} // namespace CV
