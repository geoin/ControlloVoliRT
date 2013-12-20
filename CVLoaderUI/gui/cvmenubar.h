#ifndef CVMENUBAR_H
#define CVMENUBAR_H

#include <QMenuBar>

namespace CV {
namespace GUI {

class CVMenuBar : public QMenuBar {
    Q_OBJECT
public:
    explicit CVMenuBar(QWidget *parent = 0);

signals:

public slots:

};

}
}

#endif // CVMENUBAR_H
