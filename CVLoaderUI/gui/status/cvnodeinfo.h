#ifndef CV_CVNODEINFO_H
#define CV_CVNODEINFO_H

#include <QObject>

#include "core/categories/cvcontrol.h"
#include "core/cvproject.h"


namespace CV {
namespace GUI {
namespace Status {

class CVNodeInfo {
public:
    enum Status {
        UNKNOWN_STATUS, INCOMPLETE, COMPLETE
    };

    CVNodeInfo();

    inline bool isProjectRoot() const { return _isRoot; }
    inline void isProjectRoot(bool b) { _isRoot = b; }

    inline Core::CVControl::Type type() const { return _type; }
    inline void type(Core::CVControl::Type t) { _type = t; }
	
    inline Core::CVProject::Type projType() const { return _projType; }
    inline void projType(Core::CVProject::Type t) { _projType = t; }

    inline Status status() const { return _status; }
    inline void status(Status s) { _status = s; }

private:
    bool _isRoot;
    Status _status;
	Core::CVControl::Type _type;
	Core::CVProject::Type _projType;

    Q_DISABLE_COPY(CVNodeInfo)
};

}
} // namespace GUI
} // namespace CV

#endif // CV_CVNODEINFO_H
