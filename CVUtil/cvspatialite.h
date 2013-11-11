/*
    File: cvspatialite.h
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

#ifndef SPATIALITE_H
#define SPATIALITE_H

#include <string>
#include <vector>
#include <stdexcept>
#include <Poco/SharedPtr.h>

#include <sqlite3.h>
#include "spatialite.h"

namespace CV {
    namespace Util {
        namespace Spatialite {

        typedef std::runtime_error spatialite_error ;

        class _connection_core {
        friend class Connection;
        public:
            _connection_core( sqlite3 *handle, void *cache);
            ~_connection_core();
            sqlite3     *_db();

        private:
            void        *_cache;
            sqlite3     *_handle;
        };

        class Connection {
            friend class Statement;

        public:
            typedef enum { MAX_MSG_LEN = 250 } MaxMessageLen;
            typedef enum { NO_SPATIAL_METADATA = 0,
                           LEGACY_SPATIAL_METADATA = 1,
                           FDO_OGR_SPATIAL_METADATA = 2,
                           CURRENT_SPATIAL_METADATA = 3 } SpatialMetadata;

            Connection();
            ~Connection();
            bool is_valid() const;

            void create(std::string const &dbname);
            void open(std::string const &dbname);
            void initialize_metdata();
            SpatialMetadata check_metadata() const;
            int load_shapefile( std::string const &filename,
                                std::string const &layername,
                                std::string const &iconv_char_set,
                                int srid,
                                std::string const &geom_col_name,
                                bool coerce2d,
                                bool compressed,
                                bool spatial_index );

            void remove_layer(std::string const &layername);

            void begin_transaction();
            void commit_transaction();
            void rollback_transaction();

            void execute_immediate(std::string const &sql);

        private:
           sqlite3 *_db();
           Poco::SharedPtr< _connection_core > _cnn_core;
        };


        class _statement_core {
        public:
            _statement_core( sqlite3_stmt *stmt );
            ~_statement_core();
            sqlite3_stmt *_statement();

        private:
            sqlite3_stmt *_stmt;
        };

        class Recordset;
        class Statement {
            friend class _statement_core;
            friend class Recordset;

        public:
            Statement( Connection const &cnn, std::string const &sql );
            Statement ( Connection const &cnn );
            ~Statement();
            void prepare( std::string const &sql );
            void execute( std::string const &sql );
            void execute();
            Recordset recordset();

        private:
            sqlite3_stmt *_statement();
            sqlite3 *_db();
            Poco::SharedPtr<_statement_core>    _stmt;
            Connection                          _cnn;
        };


        class Recordset {
            friend class Statement;

        public:
            Recordset( Statement const &stmt );
            ~Recordset();

            bool next();

            int  toInt(int fldidx);
            long long  toInt64(int fldidx);
            double  toDouble(int fldidx);
            std::string  toString(int fldidx);
            void  toString(int fldidx, std::string &str);
            void  toBlob( int fldidx, std::vector<char> &v );

        private:
            Statement _stmt;
        };

        } //end Spatialite
    } //end Util
} //end CV


#endif // SPATIALITE_H
