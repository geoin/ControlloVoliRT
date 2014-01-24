#include "cvrinexdetail.h"

#include "gui/helper/cvactionslinker.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

#include <QMimeData>

#include <QFile>
#include <QUrl>

#include <QFileDialog>

//#include <QStandardPaths>

namespace CV {
namespace GUI {
namespace Details {

CVRinexDetail::CVRinexDetail(QWidget* p) : CVBaseDetail(p) {
    setAcceptDrops(true);

	title(tr("Rinex aereo"));
	description(tr("File rinex"));
}


} // namespace Details
} // namespace GUI
} // namespace CV
