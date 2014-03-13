/**
 * @file
 * OpenM++ data library: public interface for db common structures
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef DB_COMMON_H
#define DB_COMMON_H

#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
using namespace std;

#include "libopenm/common/omHelper.h"

namespace openm
{
    /** db-exception default error message: "unknown db error" */
    extern const char dbUnknownErrorMessage[];   

    /** db-exception */
    typedef OpenmException<4000, dbUnknownErrorMessage> DbException;

    /** db-row abstract base */
    struct IRowBase
    {
        virtual ~IRowBase(void) throw() = 0;
    };

    /** unique pointer to db row */
    typedef unique_ptr<IRowBase> IRowBaseUptr;

    /** db rows: vector of unique pointers to db row */
    typedef vector<IRowBaseUptr> IRowBaseVec;

    /** row factory and setter interface to select row from database */
    class IRowAdapter
    {
    public:
        virtual ~IRowAdapter(void) throw() { }

        /** create new row (tuple, struct or array) initialized with default field values */
        virtual IRowBase * createRow(void) const = 0;

        /** return row size: number of columns */
        virtual int size(void) const = 0;

        /** array[rowSize] of type_info for each column, used to convert from db-type to target type */
        virtual const type_info ** columnTypes(void) const = 0;

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
        unsigned long long llVal;

        /** value of unsigned integer type */
        unsigned long long ullVal;

        /** value of boolean type */
        bool isVal;

        /** value of double type */
        double dVal;

        /** value of long double type */
        long double dlVal;

        /** c-style string value */
        char * szVal;

        /** zero initialized value */
        DbValue(void) { clear(); }

        /** value initialized by supplied integer */
        DbValue(int i_value) { 
            clear(); 
            llVal = i_value; 
        }

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

        /** clear value with zeros */
        void clear(void);
    };
}

#endif  // DB_COMMON_H
