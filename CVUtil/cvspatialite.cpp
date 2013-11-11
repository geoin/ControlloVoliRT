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

        sqlite3     *_connection_core::_db(){
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
            if ( rs.next() ) {
                ret = (SpatialMetadata) rs.toInt(0);
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

        sqlite3 *Connection::_db() {
            return _cnn_core->_db();
        }


        _statement_core::_statement_core( sqlite3_stmt *stmt ): _stmt(stmt) {
        }

        _statement_core::~_statement_core(){
            sqlite3_finalize( _stmt );
        }


        sqlite3_stmt *_statement_core::_statement(){
            return _stmt;
        }

        Statement::Statement(const Connection &cnn, std::string const &sql ) :
            _cnn(cnn) {
            prepare( sql );
        }

        Statement::Statement( const Connection &cnn ) :
            _cnn(cnn) {
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
        }

        void Statement::execute( std::string const &sql ){
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
            return Recordset(*this);
        }

        sqlite3_stmt *Statement::_statement(){
            return _stmt->_statement();
        }

        sqlite3 *Statement::_db(){
            return _cnn._db();
        }


        Recordset::Recordset(const Statement &stmt ): _stmt(stmt) {
        }

        Recordset::~Recordset(){
        }


       bool Recordset::next() {
            int ret = sqlite3_step( _stmt._statement() );
            if ( ret == SQLITE_DONE )
                return false;
            else if (ret == SQLITE_ROW )
                return true;
            else  {
              std::stringstream err;
              err << "Next error: " <<  std::string( sqlite3_errmsg(_stmt._db()) );
              throw spatialite_error(err.str());
             }
        }

        int Recordset::toInt(int fldidx){
            return sqlite3_column_int( _stmt._statement() , fldidx );
        }

        long long  Recordset::toInt64(int fldidx){
            return (long long)sqlite3_column_int64( _stmt._statement() , fldidx );
        }

        double  Recordset::toDouble(int fldidx){
            return (double)sqlite3_column_double( _stmt._statement() , fldidx );
        }

        std::string  Recordset::toString(int fldidx) {
            int nbytes = sqlite3_column_bytes( _stmt._statement() , fldidx );
             unsigned char const *chr = sqlite3_column_text( _stmt._statement() , fldidx );
            std::string ret( (const char *) chr, nbytes);
            return ret;
        }

        void  Recordset::toString(int fldidx, std::string &str) {
            int nbytes = sqlite3_column_bytes( _stmt._statement() , fldidx );
            unsigned char const *chr = sqlite3_column_text( _stmt._statement() , fldidx );
            str.assign((const char *)chr, nbytes);
        }

        void   Recordset::toBlob( int fldidx, std::vector<char> &v ) {
            int nbytes = sqlite3_column_bytes( _stmt._statement() , fldidx );
            const void *chr = sqlite3_column_blob( _stmt._statement() , fldidx );
            v.resize(nbytes);
            memcpy( &v[0], chr, nbytes);
        }



        } //end Spatialite
    } //end Util
} //end CV

