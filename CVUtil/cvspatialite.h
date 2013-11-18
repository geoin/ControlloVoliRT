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
#include <map>
#include <stdexcept>
#include <Poco/SharedPtr.h>
#include <Poco/Any.h>

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
            sqlite3     *_db() const;

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
           sqlite3 *_db() const;
           Poco::SharedPtr< _connection_core > _cnn_core;
        };


        class _statement_core {
        public:
            _statement_core( sqlite3_stmt *stmt );
            ~_statement_core();
            sqlite3_stmt *_statement() const;

        private:
            sqlite3_stmt *_stmt;
        };

        class Recordset;
        class BindField;
        class Statement {
            friend class _statement_core;
            friend class Recordset;
            friend class QueryField;
            friend class BindField;

        public:
            Statement();
            Statement( Connection const &cnn, std::string const &sql );
            Statement ( Connection const &cnn );
            ~Statement();
            void prepare( std::string const &sql );
            void reset() const;
            void execute( std::string const &sql );
            void execute();
            Recordset recordset();
            bool is_query();

            BindField &operator[]( int i);
            BindField &operator[]( std::string const &nm);

        private:
            sqlite3_stmt *_statement() const;
            sqlite3 *_db() const;
            Poco::SharedPtr<_statement_core>    _stmt;
            Connection                          _cnn;

            int mutable _bndfldcnt;
            std::map<int, BindField> mutable _bndflds;
        };

        class Field {
            friend class Recordset;

        public:
            Field();
            Field( Field const &fld);
            Field &operator=(Field const &fld);

            virtual int  toInt() = 0;
            virtual long long  toInt64() = 0;
            virtual double  toDouble() = 0;
            virtual std::string  toString() = 0;
            virtual void  toString(std::string &str) = 0;
            virtual void  toBlob( std::vector<unsigned char> &v ) = 0;
            std::string const &name() const;
            int index() const;

        protected:
            Field( Statement &stmt, int idx);
            Statement _stmt;
            std::string mutable _name;
            int mutable _index;
        };

        class QueryField : public Field {
            friend class Recordset;

        public:
             typedef enum {
                INTEGER_TYPE = 1,
                FLOAT_TYPE = 2,
                BLOB_TYPE = 4,
                NULL_TYPE = 5,
                TEXT_TYPE = 3
            } FieldType;
            QueryField();
            QueryField( QueryField const &fld);
            QueryField &operator=(QueryField const &fld);

            virtual int  toInt();
            virtual long long  toInt64();
            virtual double  toDouble();
            virtual std::string  toString();
            virtual void  toString(std::string &str);
            virtual void  toBlob( std::vector<unsigned char> &v );
            FieldType type() const;

        private:
            QueryField( Statement &stmt, int idx);
            FieldType mutable _type;
        };

        class BindField : public Field {
            friend class Recordset;
            friend class Statement;
        public:
            BindField();
            BindField( BindField const &fld);
            BindField &operator=(BindField const &fld);

            virtual int  toInt();
            virtual long long  toInt64();
            virtual double  toDouble();
            virtual std::string  toString();
            virtual void  toString(std::string &str);
            virtual void  toBlob( std::vector<unsigned char> &v );

            void fromInt(int v);
            void fromInt64(long long v);
            void fromDouble(double v);
            void fromString(std::string const &v);
            void fromBlob(const std::vector<unsigned char> &v );
            void fromBlob( unsigned char const *v, int l );

            BindField &operator=(int v);
            BindField &operator=(long long v);
            BindField &operator=(double v);
            BindField &operator=(std::string const &v);
            BindField &operator=( std::vector<unsigned char> const &v);

        private:
            BindField( Statement &stmt, int idx);
            Poco::Any _value;
         };

        class Recordset {
            friend class Statement;
            friend class Connection;

        public:
            ~Recordset();
            bool next();
            bool eof() const;
            int fields_count() const;
            std::string column_name(int fldix) const;
            int column_index(std::string const &name) const;

            QueryField &operator[](std::string const &name) const;
            QueryField &operator[](int fldx) const;

        private:
            Recordset( Statement const &stmt );

            Statement _stmt;
            int mutable _fldcnt;
            std::map<std::string, QueryField> mutable _flds;
            bool _eof;
        };

        } //end Spatialite
    } //end Util
} //end CV


#endif // SPATIALITE_H
