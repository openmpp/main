/**
 * @file
 * OpenM++ data library: db value classes access value rows: input parameter, accumulator or expression tables
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef DB_VALUE_H
#define DB_VALUE_H

using namespace std;

#include "libopenm/db/dbCommon.h"
#include "helper.h"
#include "md5.h"

namespace openm
{
    /** value table row: parameter, accumulators or expression tables */
    struct ValueRow : public IRowBase
    {
        /** key size: dimension count, acc_id, sub_id, expr_id */
        int idCount;

        /** key columns: dimemnsions id and acc_id, sub_id, expr_id */
        unique_ptr<int32_t[]> idArr;

        /** if true then value is NOT NULL */
        bool isNotNull;

        /** value type, use std::string type for VARCHAR input parameters */
        const type_info & typeOf;

        /** value if type is numeric: integer, double, boolean etc. */
        DbValue dbVal;

        /** value if type is string */
        string strVal;

        /** create empty row of value table */
        ValueRow(int i_idCount, const type_info & i_type);
        ~ValueRow(void) throw() { }
    };

    /** Row adapter to select row from value table (parameter, accumulator or expression) */
    class ValueRowAdapter : public IRowAdapter
    {
    public:
        /** new row adapter for value table row, use std::string type for VARCHAR input parameters */
        ValueRowAdapter(int i_idCount, const type_info & i_type);

        // IRowAdapter interface implementation
        IRowBase * createRow(void) const override { return new ValueRow(idCount, typeOf); }
        int size(void) const override { return idCount + 1; }
        const type_info * const * columnTypes(void) const override { return typeVec.data(); }

        /** IRowAdapter interface implementation: set column value */
        void set(IRowBase * i_row, int i_column, const void * i_value) const override;

    private:
        /** key size: dimension count, acc_id, sub_id, expr_id */
        int idCount;

        /** value type, use std::string type for VARCHAR input parameters */
        const type_info & typeOf;

        vector<const type_info *> typeVec;  // column types

        // set value column method handler
        function<void (const void * i_value, DbValue & o_dbVal)> doSetValue;

        // value column setter casting for numeric and boolean types
        template<typename TValue> static void setValueColumn(const void * i_value, DbValue & o_dbVal)
        {
            TValue val = *static_cast<const TValue *>(i_value);
            o_dbVal = DbValue(val);
        }

    private:
        ValueRowAdapter(const ValueRowAdapter &) = delete;
        ValueRowAdapter & operator=(const ValueRowAdapter &) = delete;
    };

    /** converter for value column (parameter, accumulator or expression value) to string */
    class ValueFormatter : public IValueFormatter 
    {
    public:
        static const char * nullValueString;                    /** NULL value as string */
        static const char * ValueFormatter::trueValueString;    /** true boolean value as string */
        static const char * ValueFormatter::falseValueString;   /** false boolean value as string */

        /** new converter for value column.
         *
         * @param[in] i_type             value type, use std::string type for VARCHAR input parameters
         * @param[in] i_doubleFormat     if not null or empty then printf format for float and double, default float: %g, double: %.15g
         * @param[in] i_longDoubleFormat if not null or empty then printf format for long double, default: %.17g
         */
        ValueFormatter(const type_info & i_type, const char * i_doubleFormat = "", const char * i_longDoubleFormat = "");

        /**
         * IValueFormatter interface implementation: convert value to string using snprintf.
         *
         * @param[in] i_value   db-field value, casted to the target column type
         * @param[in] i_isNull  if true then value is NULL and return is "null"
         *
         * @return value converted to string, must be copied before next call.
         */
        const char * formatValue(const void * i_value, bool i_isNull = false) override;

        /**
         * IValueFormatter interface implementation: convert DbValue to string using snprintf.
         *
         * @param[in] i_value   db-field value, casted to the target column type
         * @param[in] i_isNull  if true then value is NULL and return is "null"
         *
         * @return value converted to string, must be copied before next call.
         */
        const char * formatValue(const DbValue & i_value, bool i_isNull = false) override;

    private:
        /** value type, use std::string type for VARCHAR input parameters */
        const type_info & typeOf;

        /** value buffer to store string of the value */
        char valueStr[OM_STR_DB_MAX + 1];

        // value to string converter handler
        function<int (const void * i_value, size_t i_size, char * io_buffer)> doFormatValue;

        // value to string converter handler
        function<int (const DbValue & i_value, size_t i_size, char * io_buffer)> doFormatDbValue;

    private:
        ValueFormatter(const ValueFormatter &) = delete;
        ValueFormatter & operator=(const ValueFormatter &) = delete;
    };

    /** row processor to calculate digest of value table row (parameter, accumulator or expression) */
    class ValueRowDigester : public IRowProcessor 
    {
    public:
        /** new row digester for value table row, use std::string type for VARCHAR input parameters */
        ValueRowDigester(int i_idCount, const type_info & i_type, MD5 * io_md5, const char * i_doubleFormat = "", const char * i_longDoubleFormat = "");

        /** IRowProcessor implementation: append row to digest */
        void processRow(IRowBaseUptr & i_row) override;

    private:
        /** key size: dimension count, acc_id, sub_id, expr_id */
        int idCount;

        /** value type, use std::string type for VARCHAR input parameters */
        const type_info & typeOf;

        /** digest calculator */
        MD5 * md5;

        /** converter for value column into string */
        unique_ptr<ValueFormatter> fmtValue;

    private:
        ValueRowDigester(const ValueRowDigester &) = delete;
        ValueRowDigester & operator=(const ValueRowDigester &) = delete;
    };
}

#endif  // DB_VALUE_H
