#ifndef OGRGEOMPTR_H
#define OGRGEOMPTR_H
#include <Poco/SharedPtr.h>
#include <ogr_geometry.h>
#include <vector>

namespace CV {
    namespace Util {
        namespace Geometry {

            template <class C>
            class OGRGeometryReleasePolicy {
            public:
                static void release(C* pObj) {
                    OGRGeometryFactory::destroyGeometry( pObj );
                }
            };

            class OGRGeomPtr
            {
            public:
                ~OGRGeomPtr();
                OGRGeomPtr();
                OGRGeomPtr( OGRGeomPtr const &gptr );
                OGRGeomPtr( std::vector<unsigned char> &v );
                OGRGeomPtr( OGRGeometry const *g );
                OGRGeomPtr &operator=( OGRGeomPtr const &gptr );
                OGRGeomPtr &operator=( std::vector<unsigned char>  &v );
                OGRGeomPtr &operator=( OGRGeometry const *g );

                OGRGeometry *operator->();
                OGRGeometry const *operator->() const;

                operator OGRGeometry const *() const;
                operator OGRGeometry *();
                operator std::vector<unsigned char> const &() const;

            private:
                void _assign(std::vector<unsigned char> &v);
                void _assign(OGRGeometry const *g);

                std::vector<unsigned char> mutable _v;
                Poco::SharedPtr< OGRGeometry, Poco::ReferenceCounter , OGRGeometryReleasePolicy<OGRGeometry> > _geom;
            };

        }
    }
}

#endif // OGRGEOMPTR_H
