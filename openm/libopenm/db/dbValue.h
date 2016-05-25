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
    private:
        /** key size: dimension count, acc_id, sub_id, expr_id */
        int idCount;

        /** value type, use std::string type for VARCHAR input parameters */
        const type_info & typeOf;

        vector<const type_info *> typeVec;  // column types

        // set value column method
        function<void (const void * i_value, DbValue & o_dbVal)> doSetValue;

        // value column setter casting for numeric and boolean types
        template<typename TValue> static void setValueColumn(const void * i_value, DbValue & o_dbVal)
        {
            TValue val = *static_cast<const TValue *>(i_value);
            o_dbVal = DbValue(val);
        }

    public:
        /** new row adapter for value table row, use std::string type for VARCHAR input parameters */
        ValueRowAdapter(int i_idCount, const type_info & i_type);

        // IRowAdapter interface implementation
        IRowBase * createRow(void) const override { return new ValueRow(idCount, typeOf); }
        int size(void) const override { return idCount + 1; }
        const type_info * const * columnTypes(void) const override { return typeVec.data(); }

        /** IRowAdapter interface implementation: set column value */
        void set(IRowBase * i_row, int i_column, const void * i_value) const override;
    };

    /** row processor to calculate digest of value table row (parameter, accumulator or expression) */
    class ValueRowDigester : public IRowProcessor 
    {
    private:
        /** key size: dimension count, acc_id, sub_id, expr_id */
        int idCount;

        /** value type, use std::string type for VARCHAR input parameters */
        const type_info & typeOf;

        /** digest calculator */
        MD5 * md5;

        /** NULL value constant for digest calculation */
        static const char * nullValueForDigest;

    public:
        /** new row digester for value table row, use std::string type for VARCHAR input parameters */
        ValueRowDigester(int i_idCount, const type_info & i_type, MD5 * io_md5) : 
            idCount(i_idCount), 
            typeOf(i_type),
            md5(io_md5)
        { }

        /** append row to digest */
        void processRow(IRowBaseUptr & i_row) override;
    };
}

#endif  // DB_VALUE_H
