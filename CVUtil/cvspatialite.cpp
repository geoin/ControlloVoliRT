/*
    File: cvspatialite.cpp
    Author:  A.Comparini
    Date: 2013 November 03
    Comment:
        C++ Spatialite helper classes.

    Disclaimer:
        This file is part of CV a framework for verifying aerial missions for cartography.

        CV is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        Tabula is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with Tabula.  If not, see <http://www.gnu.org/licenses/>.


        Copyright (C) 2013 Geoin s.r.l.

*/
#include "cvspatialite.h"
#include <sstream>
#include <string.h>

namespace CV {
    namespace Util {
        namespace Spatialite {


        _connection_core::_connection_core( sqlite3 *handle, void *cache): _cache(cache), _handle(handle) {
        }

        _connection_core::~_connection_core(){
            if (_cache) spatialite_cleanup_ex(_cache);
            if (_handle) sqlite3_close (_handle);
        }

        sqlite3     *_connection_core::_db() const{
            return _handle;
        }

        Connection::Connection() {
        }

        Connection::~Connection() {
        }

        bool Connection::is_valid() const{
            return !_cnn_core.isNull();
        }

        void Connection::create(std::string const &dbname){
            sqlite3 *hndl = NULL;
            int ret = sqlite3_open_v2 (dbname.c_str(), &hndl, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
            if (ret != SQLITE_OK) {
                std::stringstream err;
                err << "error: cannot open " << dbname << " - " <<  std::string(sqlite3_errstr (ret));
                throw spatialite_error(err.str());
            }

            // Initialize spatialite
            void *cache = spatialite_alloc_connection ();
            spatialite_init_ex (hndl , cache, 0);
            _cnn_core.assign(new _connection_core(hndl, cache));
        }

        void Connection::open(std::string const &dbname) {
            create( dbname );
        }

        void Connection::initialize_metdata() {
            execute_immediate( "SELECT InitSpatialMetadata(1)" );
        }

        Connection::SpatialMetadata Connection::check_metadata() const {
            Statement stmt(*this, "SELECT CheckSpatialMetadata()");
            Recordset rs = stmt.recordset();
            SpatialMetadata ret = NO_SPATIAL_METADATA;
            if ( !rs.eof() ) {
                ret = (SpatialMetadata) rs[0].toInt();
            }
            return ret;
         }

        int  Connection::load_shapefile(std::string const &filename,
                            std::string const &layername,
                            std::string const &iconv_char_set,
                            int srid,
                            std::string const &geom_col_name,
                            bool coerce2d,
                            bool compressed,
                            bool spatial_index ) {

            // try to remove the layer if exists. Ignore errors
            remove_layer(layername);

            // must copy const in non const char * due to unsafe spatialite load_shapefile()
            // spatialite should use const char * instead  !!!!!
            char * c_fname = new char[filename.size()+1];
            strcpy(c_fname, filename.c_str());

            char * c_layername = new char[layername.size()+1];
            strcpy(c_layername, layername.c_str());

            char * c_iconv_char_set = new char[iconv_char_set.size()+1];
            strcpy(c_iconv_char_set, iconv_char_set.c_str());

            char * c_geo_col_name = new char[geom_col_name.size()+1];
            strcpy( c_geo_col_name, geom_col_name.c_str() );

            char err_msg[MAX_MSG_LEN]; // we don't know how much large should the buffer be !!!!!!!
            int nrows = 0;
            int ret = ::load_shapefile(_db(),
                             c_fname,
                             c_layername,
                             c_iconv_char_set,
                             srid,
                             c_geo_col_name,
                             coerce2d ? 1 : 0,
                             compressed ? 1 : 0,
                             0,
                             spatial_index ? 1 : 0,
                             &nrows,
                             err_msg);
            std::stringstream err;
            if ( ret == 0 ){ //failure
                err << "error: Load from shape " << filename << " failed! - " << std::string(err_msg);
                throw spatialite_error(err.str());
            }
            return nrows;
        }

        void Connection::remove_layer(std::string const &layername) {
            if (gaiaDropTable(_db(), layername.c_str()) == 0) {
                std::stringstream err;
                err << "error: Generic error in removing layer " << layername;
                throw spatialite_error( err.str() );
            }
        }

        void Connection::begin_transaction() {
            execute_immediate("BEGIN TRANSACTION");
        }

        void Connection::commit_transaction() {
            execute_immediate("COMMIT TRANSACTION");
        }

        void Connection::rollback_transaction() {
            execute_immediate("ROLLBACK TRANSACTION");
        }

        void Connection::execute_immediate(std::string const &sql){
            Statement stmt(*this, sql);
            stmt.execute();
        }

        sqlite3 *Connection::_db() const {
            return _cnn_core->_db();
        }


        _statement_core::_statement_core( sqlite3_stmt *stmt ): _stmt(stmt) {
        }

        _statement_core::~_statement_core(){
            sqlite3_finalize( _stmt );
        }


        sqlite3_stmt *_statement_core::_statement() const {
            return _stmt;
        }

        Statement::Statement(): _bndfldcnt(-1) {
        }

        Statement::Statement(const Connection &cnn, std::string const &sql ) :
            _cnn(cnn),
            _bndfldcnt(-1) {
            prepare( sql );
        }

        Statement::Statement( const Connection &cnn ) :
            _cnn(cnn),
            _bndfldcnt(-1) {
        }

        Statement::~Statement() {
        }

        void Statement::prepare( std::string const &sql ) {
            // check preconditions
            if ( !_cnn.is_valid() )
                throw spatialite_error("Invalid Connection");

            sqlite3_stmt *stmt = NULL;
            char const *tail=NULL;
            int ret = sqlite3_prepare_v2( _db(), sql.c_str(), -1, &stmt, &tail );
            if ( ret != SQLITE_OK ) {
              std::stringstream err;
              err << "Prepare error: " <<  std::string( sqlite3_errmsg(_db()) );
              throw spatialite_error(err.str());
             }
            _stmt.assign(new _statement_core(stmt));

            _bndflds.clear();
            _bndfldcnt = sqlite3_bind_parameter_count(stmt);
            for (int i=1; i <= _bndfldcnt; i++ ){
                BindField bfld(*this, i);
                _bndflds[i] = bfld;
            }
        }

        void Statement::reset() const {
            // check preconditions
            if ( !_cnn.is_valid() )
                throw spatialite_error("Invalid Connection");
            if ( _stmt.isNull() )
                throw spatialite_error("Invalid Statement");
            int ret = sqlite3_reset(_stmt->_statement());
            if ( ret != SQLITE_OK ) {
                std::stringstream err;
                err << "Reset error: " <<  std::string( sqlite3_errmsg(_db()) );
                throw spatialite_error(err.str());
            }
        }

        void Statement::execute( std::string const &sql ) {
            prepare(sql);
            execute();
        }

        void Statement::execute( ){
            if ( _stmt.isNull() )
                throw spatialite_error("Invalid Statement");

            int ret = sqlite3_step( _stmt->_statement() );
            if ( ret != SQLITE_ROW && ret != SQLITE_DONE ) {
              std::stringstream err;
              err << "Execute error: " <<  std::string( sqlite3_errmsg( _db() ) );
              throw spatialite_error(err.str());
             }
        }

        Recordset Statement::recordset() {
            if ( _stmt.isNull() )
                throw spatialite_error("Invalid Statement");
            return Recordset(*this);
        }

        bool Statement::is_query(){
            if ( _stmt.isNull() )
                throw spatialite_error("Invalid Statement");
            int ret = sqlite3_data_count( _statement() );
            if ( ret == 0 )
                return false;
            else
                return true;
        }

        BindField &Statement::operator[]( int i){
            if ( i < 1 || i > _bndfldcnt )
                throw spatialite_error("Invalid bind field index");
            return _bndflds[i];
        }

        BindField &Statement::operator[]( std::string const &nm){
            int idx = sqlite3_bind_parameter_index(_statement(), nm.c_str());
            if (idx == 0 )
                throw spatialite_error("Invalid bind filed name");
            return _bndflds[idx];
        }


        sqlite3_stmt *Statement::_statement() const {
            return _stmt->_statement();
        }

        sqlite3 *Statement::_db() const {
            return _cnn._db();
        }

        Field::Field(){
        }

        Field::Field( Field const &fld):
                        _stmt(fld._stmt),
                        _name(fld._name),
                        _index(fld._index) {
        }

        Field &Field::operator=(Field const &fld) {
            _stmt = fld._stmt;
            _name = fld._name;
            _index  = fld._index;
            return *this;
        }

        Field::Field(Statement &stmt, int idx): _stmt(stmt), _index(idx) {
        }

        std::string const &Field::name() const {
            return _name;
        }

        int Field::index() const{
            return _index;
        }

        QueryField::QueryField(): Field() {
        }

        QueryField::QueryField( QueryField const &fld):
                        Field(fld),
                        _type(fld._type) {
        }

        QueryField &QueryField::operator=(QueryField const &fld) {
            Field::operator=(fld);
            _type = fld._type;
            return *this;
        }

        int  QueryField::toInt() {
            return sqlite3_column_int( _stmt._statement() , _index );
        }

        long long  QueryField::toInt64(){
            return (long long)sqlite3_column_int64( _stmt._statement() , _index );
        }

        double  QueryField::toDouble(){
            return (double)sqlite3_column_double( _stmt._statement() , _index );
        }

        std::string  QueryField::toString(){
            int nbytes = sqlite3_column_bytes( _stmt._statement() , _index );
             unsigned char const *chr = sqlite3_column_text( _stmt._statement() , _index );
            std::string ret( (const char *) chr, nbytes);
            return ret;
        }

        void  QueryField::toString( std::string &str){
            int nbytes = sqlite3_column_bytes( _stmt._statement() , _index );
            unsigned char const *chr = sqlite3_column_text( _stmt._statement() , _index );
            str.assign((const char *)chr, nbytes);
        }

        void  QueryField::toBlob(std::vector<unsigned char> &v ){
            int nbytes = sqlite3_column_bytes( _stmt._statement() , _index );
            const void *chr = sqlite3_column_blob( _stmt._statement() , _index );
            v.resize(nbytes);
            memcpy( &v[0], chr, nbytes);
        }

        QueryField::FieldType QueryField::type() const {
            return _type;
        }

        QueryField::QueryField( Statement &stmt, int idx): Field(stmt, idx) {
            const char *nm = sqlite3_column_name( _stmt._statement(), idx);
            _name.clear();
            if ( nm )
                _name.assign( nm );
            _type = (QueryField::FieldType)sqlite3_column_type(_stmt._statement(), idx );
        }

        BindField::BindField(): Field() {
        }

        BindField::BindField( BindField const &fld):
                        Field(fld) {
        }

        BindField &BindField::operator=(BindField const &fld) {
            Field::operator=(fld);
            return *this;
        }

        int  BindField::toInt(){
            return Poco::AnyCast<int>(_value);
        }

        long long  BindField::toInt64(){
            return Poco::AnyCast<long long>(_value);
        }

        double  BindField::toDouble(){
            return Poco::AnyCast<double>(_value);
        }

        std::string  BindField::toString(){
            return Poco::AnyCast<std::string>(_value);
        }

        void  BindField::toString(std::string &str) {
            str = Poco::AnyCast<std::string>(_value);
        }

        void  BindField::toBlob(std::vector<unsigned char> &v ){
            v = Poco::RefAnyCast< std::vector<unsigned char> >(_value);
        }

        void BindField::fromInt(int v){
            _value = v;
            int ret = sqlite3_bind_int(_stmt._statement(), _index, v );
            if (ret != SQLITE_OK) {
                std::stringstream err;
                err << "Bind integer error: " <<  std::string( sqlite3_errmsg(_stmt._db()) );
                throw spatialite_error(err.str());
            }
        }

        void BindField::fromInt64(long long v){
            _value = v;
            int ret = sqlite3_bind_int64(_stmt._statement(), _index, v );
            if (ret != SQLITE_OK) {
                std::stringstream err;
                err << "Bind int64 error: " <<  std::string( sqlite3_errmsg(_stmt._db()) );
                throw spatialite_error(err.str());
            }
       }

        void BindField::fromDouble(double v){
            _value = v;
            int ret = sqlite3_bind_double(_stmt._statement(), _index, v );
            if (ret != SQLITE_OK) {
                std::stringstream err;
                err << "Bind double error: " <<  std::string( sqlite3_errmsg(_stmt._db()) );
                throw spatialite_error(err.str());
            }
        }

        void BindField::fromString(std::string const &v){
            _value = v;
            const char *c = Poco::RefAnyCast<std::string>(_value).c_str();
            int ret = sqlite3_bind_text(_stmt._statement(), _index, c, v.size(), SQLITE_STATIC );
            if (ret != SQLITE_OK) {
                std::stringstream err;
                err << "Bind string error: " <<  std::string( sqlite3_errmsg(_stmt._db()) );
                throw spatialite_error(err.str());
            }
        }

        void BindField::fromBlob( std::vector<unsigned char> const &v ){
            _value = v;
            unsigned char const *c = &(Poco::RefAnyCast< std::vector<unsigned char> >(_value))[0];
            int ret = sqlite3_bind_blob(_stmt._statement(), _index, (const char *)&v[0], v.size(), SQLITE_STATIC );
            if (ret != SQLITE_OK) {
                std::stringstream err;
                err << "Bind blob error: " <<  std::string( sqlite3_errmsg(_stmt._db()) );
                throw spatialite_error(err.str());
            }
        }

        void BindField::fromBlob( unsigned char const *v, int l ){
            _value = v;
            int ret = sqlite3_bind_blob(_stmt._statement(), _index, (const char *)v, l, SQLITE_TRANSIENT );
            if (ret != SQLITE_OK) {
                std::stringstream err;
                err << "Bind blob error: " <<  std::string( sqlite3_errmsg(_stmt._db()) );
                throw spatialite_error(err.str());
            }
        }

        BindField &BindField::operator=(int v) {
            fromInt(v);
            return *this;
        }

        BindField &BindField::operator=(long long v) {
            fromInt64(v);
            return *this;
        }

        BindField &BindField::operator=(double v) {
            fromDouble(v);
            return *this;
        }

        BindField &BindField::operator=(std::string const &v) {
            fromString(v);
            return *this;
        }

        BindField &BindField::operator=( std::vector<unsigned char> const &v) {
            fromBlob(&v[0], v.size());
            return *this;
        }

        BindField::BindField( Statement &stmt, int idx): Field( stmt, idx ) {
            const char *nm = sqlite3_bind_parameter_name( _stmt._statement(), idx);
            _name.assign( nm );
        }

        Recordset::Recordset(const Statement &stmt ):
            _stmt(stmt), _fldcnt(-1),_eof(true) {
            _eof = next();
            int i=0;
            for (i=0; i < fields_count(); i++) {
                QueryField fld(_stmt, i);
                _flds[fld.name()] = fld;
            }
        }

        Recordset::~Recordset(){
        }


       bool Recordset::next() {
            int ret = sqlite3_step( _stmt._statement() );
            if ( ret == SQLITE_DONE )
                _eof = true;
            else if (ret == SQLITE_ROW )
                _eof = false;
            else  {
              std::stringstream err;
              err << "Next error: " <<  std::string( sqlite3_errmsg(_stmt._db()) );
              throw spatialite_error(err.str());
             }
            return _eof;
        }

        bool Recordset::eof() const {
            return _eof;
        }

        int Recordset::fields_count() const {
            if ( _fldcnt < 0 )
                _fldcnt = sqlite3_data_count( _stmt._statement() );
            return _fldcnt;
        }

        std::string Recordset::column_name(int fldidx) const {
            if (fldidx < 0 || fldidx >= fields_count() )
                throw spatialite_error("Field index out of range");
            //const char *nm = sqlite3_column_origin_name( _stmt._statement(), fldidx);
            const char *nm = sqlite3_column_name( _stmt._statement(), fldidx);
            std::string ret( nm );
            return ret;
        }

        int Recordset::column_index(std::string const &name) const {
           if ( _flds.find(name) == _flds.end())
               throw spatialite_error("Field name not exists");
           return _flds[name].index();
        }

        QueryField &Recordset::operator[](std::string const &name) const {
            if ( _flds.find(name) == _flds.end())
                throw spatialite_error("Field name not exists");
            return _flds[name];
        }

        QueryField &Recordset::operator[](int fldx) const {
            return (*this)[column_name(fldx)];
        }
        } //end Spatialite
    } //end Util
} //end CV

