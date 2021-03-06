/**
 * @file
 * OpenM++ data library: public interface for db common structures
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_DB_COMMON_H
#define OM_DB_COMMON_H

#include <cmath>

using namespace std;

#include "libopenm/omError.h"

namespace openm
{
    /** SQLite db-provider name */
    extern const char * SQLITE_DB_PROVIDER;

    /** MySQL and MariaDB db-provider name */
    extern const char * MYSQL_DB_PROVIDER;

    /** PostgreSQL db-provider name */
    extern const char * PGSQL_DB_PROVIDER;

    /** MS SQL db-provider name */
    extern const char * MSSQL_DB_PROVIDER;

    /** Oracle db-provider name */
    extern const char * ORACLE_DB_PROVIDER;

    /** DB2 db-provider name */
    extern const char * DB2_DB_PROVIDER;

    /** db-exception default error message: "unknown db error" */
    extern const char dbUnknownErrorMessage[];   

    /** db-exception */
    typedef OpenmException<4000, dbUnknownErrorMessage> DbException;

    /** row factory and setter interface to select row from database */
    class IRowAdapter
    {
    public:
        virtual ~IRowAdapter(void) noexcept { }

        /** create new row (tuple, struct or array) initialized with default field values */
        virtual IRowBase * createRow(void) const = 0;

        /** return row size: number of columns */
        virtual int size(void) const = 0;

        /** array[rowSize] of type_info for each column, used to convert from db-type to target type */
        virtual const type_info * const * columnTypes(void) const = 0;

        /**
         * @brief   field value setter: i_row[i_column] = *i_value 
         * 
         * @param[in] i_row     new row created by createRow()
         * @param[in] i_column  zero-based column index                            
         * @param[in] i_value   db-field value, casted to the target column type
         *
         * this method called for each row by data library select() methods to set field values. \n 
         * this method called only if db-field value NOT IS NULL.
         */
        virtual void set(IRowBase * i_row, int i_column, const void * i_value) const = 0;
    };

    /** union to pass value to database methods */
    union DbValue
    {
    public:

        /** value of integer type */
        long long llVal;

        /** value of unsigned integer type */
        unsigned long long ullVal;

        /** value of boolean type */
        bool isVal;

        /** value of float type */
        float fVal;

        /** value of double type */
        double dVal;

        /** value of long double type */
        long double dlVal;

        /** c-style string */
        const char * szVal;

        /** zero initialized value */
        DbValue(void) { clear(); }

        /** value initialized by supplied integer */
        DbValue(int i_value) : DbValue(static_cast<long long>(i_value)) { }

        /** value initialized by supplied unsigned integer */
        DbValue(unsigned int i_value) : DbValue(static_cast<unsigned long long>(i_value)) { }

        /** value initialized by supplied long */
        DbValue(long i_value) : DbValue(static_cast<long long>(i_value)) { }

        /** value initialized by supplied unsigned long */
        DbValue(unsigned long i_value) : DbValue(static_cast<unsigned long long>(i_value)) { }

        /** value initialized by supplied long */
        DbValue(long long i_value) {
            clear();
            llVal = i_value;
        }

        /** value initialized by supplied unsigned long */
        DbValue(unsigned long long i_value) {
            clear();
            ullVal = i_value;
        }

        /** value initialized by supplied float */
        DbValue(float i_value) { 
            clear(); 
            fVal = i_value; 
        }

        /** value initialized by supplied double */
        DbValue(double i_value) { 
            clear(); 
            dVal = i_value; 
        }

        /** value initialized by supplied long double */
        DbValue(long double i_value) { 
            clear(); 
            dlVal = i_value; 
        }

        /** value initialized by supplied boolean */
        DbValue(bool i_value) { 
            clear(); 
            isVal = i_value; 
        }

        /** value initialized by supplied c-style string */
        DbValue(const char * i_value) {
            clear();
            szVal = i_value;
        }

        /** clear value with zeros */
        void clear(void);

        /** cast between float, double and long double */
        template<typename TSrc, typename TDst> static TDst castDouble(const DbValue & i_value);
    };

    /** converter for value column (parameter, accumulator or expression value) to string */
    class IValueFormatter
    {
    public:
        virtual ~IValueFormatter(void) noexcept { }

        /**
         * convert value to string using snprintf.
         *
         * @param[in] i_value   db-field value, casted to the target column type
         * @param[in] i_isNull  if true then value is NULL and return is "null"
         *
         * @return value converted to string, result must be copied before next call.
         */
        virtual const char * formatValue(const void * i_value, bool i_isNull = false) = 0;
    };


    /** public interafce for row processing during select, ie: select and append to row list */
    class IRowProcessor
    {
    public:
        virtual ~IRowProcessor(void) noexcept { }

        /** process row, ie: append to row list or aggregate. */
        virtual void processRow(IRowBaseUptr & i_row) = 0;
    };
}

#endif  // OM_DB_COMMON_H
