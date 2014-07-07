#include "ogrgeomptr.h"

namespace CV {
    namespace Util {
        namespace Geometry {

            OGRGeomPtr::~OGRGeomPtr(){
            }

            OGRGeomPtr::OGRGeomPtr(){
            }

            OGRGeomPtr::OGRGeomPtr( OGRGeomPtr const &gptr ){
                _geom = gptr._geom;
            }

            OGRGeomPtr::OGRGeomPtr(const std::vector<unsigned char> &v ){
                _assign(v);
            }

            OGRGeomPtr::OGRGeomPtr( OGRGeometry *g ){
                _assign(g);
            }

            OGRGeomPtr &OGRGeomPtr::operator=( OGRGeomPtr const &gptr ){
                _geom = gptr._geom;
                return *this;
            }

            OGRGeomPtr &OGRGeomPtr::operator=( std::vector<unsigned char>  const &v ){
                _assign(v);
                return (*this);
            }

            OGRGeomPtr &OGRGeomPtr::operator=( OGRGeometry *g ){
                _assign(g);
                return (*this);
            }

            OGRGeometry *OGRGeomPtr::operator->(){
                return _geom.get();
            }

            OGRGeometry const *OGRGeomPtr::operator->() const{
                return _geom.get();
            }

            void OGRGeomPtr::_assign(std::vector<unsigned char> const &v){
                OGRGeometryFactory gf;
                OGRGeometry *geom;
                unsigned char* c = (unsigned char*) &v[0];
                if ( gf.createFromWkb(c, NULL, &geom) != OGRERR_NONE )
                    throw std::runtime_error("Invalid import from WKB");
                _geom.assign(geom);
            }

            void OGRGeomPtr:: _assign( OGRGeometry *g) {
                _geom.assign(g);
            }

            OGRGeomPtr::operator OGRGeometry const *() const {
                return _geom.get();
            }

            OGRGeomPtr::operator OGRGeometry *() {
                return _geom.get();
            }

            OGRGeomPtr::operator std::vector<unsigned char> const &() const {
                _v.resize(_geom->WkbSize());
                _geom->exportToWkb( wkbNDR, &_v[0] );
                return _v;
            }


        }
    }
}
