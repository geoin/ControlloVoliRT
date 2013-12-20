#include "gui/cvtreenodedelegate.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QLabel>
#include <QHBoxLayout>

namespace CV {
namespace GUI {

CVTreeNodeDelegate::CVTreeNodeDelegate(QWidget* p, const QStringList& s) : QWidget(p) {
    auto box = new QHBoxLayout;
    box->setContentsMargins(0, 0, 0, 0);

    QString labelTxt;
    if (s.length()) {
        labelTxt = s.at(0);
    }
    auto label = new QLabel(labelTxt, this);
    label->setMinimumHeight(22);
    label->setIndent(4);
    box->addWidget(label);
    setLayout(box);

    setAcceptDrops(true);
}

void CVTreeNodeDelegate::dragEnterEvent(QDragEnterEvent* ev) {
    ev->accept();
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

} // namespace GUI
} // namespace CV
