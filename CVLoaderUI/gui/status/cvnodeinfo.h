#ifndef CV_CVNODEINFO_H
#define CV_CVNODEINFO_H

#include <QObject>

namespace CV {
namespace GUI {
namespace Status {

class CVNodeInfo {
public:
    enum Type {
        UNKNOWN_TYPE, PHOTOGRAMMETRY, FLY_PLAN,
        GPS_DATA, FLY
    };

    enum Status {
        UNKNOWN_STATUS, INCOMPLETE, COMPLETE
    };

    CVNodeInfo();

    inline bool isProjectRoot() const { return _isRoot; }
    inline void isProjectRoot(bool b) { _isRoot = b; }

    inline Type type() const { return _type; }
    inline void type(Type t) { _type = t; }

    inline Status status() const { return _status; }
    inline void status(Status s) { _status = s; }

private:
    bool _isRoot;
    Type _type;
    Status _status;

    Q_DISABLE_COPY(CVNodeInfo)
};

}
} // namespace GUI
} // namespace CV

#endif // CV_CVNODEINFO_H
