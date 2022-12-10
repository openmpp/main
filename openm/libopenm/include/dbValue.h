/**
 * @file
 * OpenM++ data library: db value classes access value rows: input parameter, accumulator or expression tables
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef DB_VALUE_H
#define DB_VALUE_H

#include "libopenm/db/dbCommon.h"
#include "helper.h"
#include "md5.h"

using namespace std;

namespace openm
{
    /** set DbValue by casting a pointer to the value */
    class DbValueSetter
    {
    public:
        /** create DbValue setter.
         *
         * @param[in] i_type    value type, use std::string type for VARCHAR values
         */
        DbValueSetter(const type_info & i_type);

        /** set DbValue by casting a pointer to the value */
        void set(const void * i_value, DbValue & o_dbVal);

        /** return value type */
        const type_info & valueType(void) const { return typeOf; };

    private:
        /** value type, use std::string type for VARCHAR values */
        const type_info & typeOf;

        /** set DbValue by casting a pointer to the value */
        function<void(const void * i_value, DbValue & o_dbVal)> doSetValue;

        /** set DbValue by casting a pointer to the value */
        template<typename TValue> static void setValue(const void * i_value, DbValue & o_dbVal)
        {
            TValue val = *static_cast<const TValue *>(i_value);
            o_dbVal = DbValue(val);
        }

    private:
        // DbValueSetter(const DbValueSetter &) = delete;
        DbValueSetter & operator=(const DbValueSetter &) = delete;
    };

    /** convert value to string using snprintf: integer and float values. */
    template<typename TValue>
    int FormatHandler(const void * i_value, size_t i_size, char * io_buffer, const char * i_format)
    {
        TValue val = *static_cast<const TValue *>(i_value);
        return snprintf(io_buffer, i_size, i_format, val);
    }

    /** convert value to string using snprintf: integer and float values. */
    template<typename TValue>
    int FloatSqlFormatHandler(const void * i_value, size_t i_size, char * io_buffer, const char * i_format)
    {
        TValue val = *static_cast<const TValue *>(i_value);

        if (isfinite(val)) {
            return snprintf(io_buffer, i_size, i_format, val);
        }
        // else copy "NULL" into output buffer
        strncpy(io_buffer, "NULL", i_size);
        io_buffer[i_size - 1] = '\0';
        return (int)strnlen(io_buffer, i_size);
    }

    /** convert bool value to string: return "true" or "false" */
    extern int BoolFormatHandler(const void * i_value, size_t i_size, char * io_buffer);

    /** convert bool value to SQL constant: return "1" or "0" */
    extern int BoolSqlFormatHandler(const void * i_value, size_t i_size, char * io_buffer);

    /** convert value to string: make a copy of source string */
    extern int StrFormatHandler(const void * i_value, size_t i_size, char * io_buffer);

    /** convert string value into SQL constant: return 'quoted source value' */
    extern int StrSqlFormatHandler(const void * i_value, size_t i_size, char * io_buffer);

    inline const char * nullValueString = "null";      /** NULL value as string */
    inline const char * trueValueString = "true";      /** true boolean value as string */
    inline const char * falseValueString = "false";    /** false boolean value as string */

    /** converter for value column (parameter, accumulator or expression value) to string */
    template<const size_t valueLen>
    class ValueFormatterBase : public IValueFormatter
    {
    public:
        /** converter for value column into string.
         *
         * @param[in] i_type             value type, use std::string type for VARCHAR values
         * @param[in] i_isSqlFormat      if true then convert into SQL constant: 'quoted string values' and 1/0 for boolean values
         * @param[in] i_doubleFormat     if not null or empty then printf format for float and doubles, default: %.15g
         */
        ValueFormatterBase(const type_info & i_type, bool i_isSqlFormat = false, const char * i_doubleFormat = "") :
            typeOf(i_type),
            doFormatValue(nullptr)
        {
            if (typeOf == typeid(char)) {
                doFormatValue = bind(FormatHandler<char>, placeholders::_1, placeholders::_2, placeholders::_3, "%hhd");
            }
            if (typeOf == typeid(unsigned char)) {
                doFormatValue = bind(FormatHandler<unsigned char>, placeholders::_1, placeholders::_2, placeholders::_3, "%hhu");
            }
            if (typeOf == typeid(short)) {
                doFormatValue = bind(FormatHandler<short>, placeholders::_1, placeholders::_2, placeholders::_3, "%hd");
            }
            if (typeOf == typeid(unsigned short)) {
                doFormatValue = bind(FormatHandler<unsigned short>, placeholders::_1, placeholders::_2, placeholders::_3, "%hu");
            }
            if (typeOf == typeid(int)) {
                doFormatValue = bind(FormatHandler<int>, placeholders::_1, placeholders::_2, placeholders::_3, "%d");
            }
            if (typeOf == typeid(unsigned int)) {
                doFormatValue = bind(FormatHandler<unsigned int>, placeholders::_1, placeholders::_2, placeholders::_3, "%u");
            }
            if (typeOf == typeid(long)) {
                doFormatValue = bind(FormatHandler<long>, placeholders::_1, placeholders::_2, placeholders::_3, "%ld");
            }
            if (typeOf == typeid(unsigned long)) {
                doFormatValue = bind(FormatHandler<unsigned long>, placeholders::_1, placeholders::_2, placeholders::_3, "%lu");
            }
            if (typeOf == typeid(long long)) {
                doFormatValue = bind(FormatHandler<long long>, placeholders::_1, placeholders::_2, placeholders::_3, "%lld");
            }
            if (typeOf == typeid(unsigned long long)) {
                doFormatValue = bind(FormatHandler<unsigned long long>, placeholders::_1, placeholders::_2, placeholders::_3, "%llu");
            }
            if (typeOf == typeid(int8_t)) {
                doFormatValue = bind(FormatHandler<int8_t>, placeholders::_1, placeholders::_2, placeholders::_3, "%hhd");
            }
            if (typeOf == typeid(uint8_t)) {
                doFormatValue = bind(FormatHandler<uint8_t>, placeholders::_1, placeholders::_2, placeholders::_3, "%hhu");
            }
            if (typeOf == typeid(int16_t)) {
                doFormatValue = bind(FormatHandler<int16_t>, placeholders::_1, placeholders::_2, placeholders::_3, "%hd");
            }
            if (typeOf == typeid(uint16_t)) {
                doFormatValue = bind(FormatHandler<uint16_t>, placeholders::_1, placeholders::_2, placeholders::_3, "%hu");
            }
            if (typeOf == typeid(int32_t)) {
                doFormatValue = bind(FormatHandler<int32_t>, placeholders::_1, placeholders::_2, placeholders::_3, "%d");
            }
            if (typeOf == typeid(uint32_t)) {
                doFormatValue = bind(FormatHandler<uint32_t>, placeholders::_1, placeholders::_2, placeholders::_3, "%u");
            }
            if (typeOf == typeid(int64_t)) {
                doFormatValue = bind(FormatHandler<int64_t>, placeholders::_1, placeholders::_2, placeholders::_3, "%lld");
            }
            if (typeOf == typeid(uint64_t)) {
                doFormatValue = bind(FormatHandler<uint64_t>, placeholders::_1, placeholders::_2, placeholders::_3, "%llu");
            }

            if (!i_isSqlFormat) {   // not a SQL constant: simple snprintf formatted values

                if (typeOf == typeid(bool)) {
                    doFormatValue = BoolFormatHandler;
                }
                if (typeOf == typeid(float)) {
                    doFormatValue = bind(
                        FormatHandler<float>,
                        placeholders::_1, placeholders::_2, placeholders::_3,
                        ((i_doubleFormat != nullptr && i_doubleFormat[0] != '\0') ? i_doubleFormat : "%.15g")
                    );
                }
                if (typeOf == typeid(double)) {
                    doFormatValue = bind(
                        FormatHandler<double>,
                        placeholders::_1, placeholders::_2, placeholders::_3,
                        ((i_doubleFormat != nullptr && i_doubleFormat[0] != '\0') ? i_doubleFormat : "%.15g")
                    );
                }
                if (typeOf == typeid(long double)) {
                    doFormatValue = bind(
                        FormatHandler<long double>,
                        placeholders::_1, placeholders::_2, placeholders::_3,
                        ((i_doubleFormat != nullptr && i_doubleFormat[0] != '\0') ? i_doubleFormat : "%.15g")
                    );
                }
                if (typeOf == typeid(string)) {
                    doFormatValue = StrFormatHandler;
                }
            }
            else {      // SQL constant: 'quoted strings', '1' or '0' for booleans, NULL if floats are not finite

                if (typeOf == typeid(bool)) {
                    doFormatValue = BoolSqlFormatHandler;
                }
                if (typeOf == typeid(float)) {
                    doFormatValue = bind(
                        FloatSqlFormatHandler<float>,
                        placeholders::_1, placeholders::_2, placeholders::_3,
                        ((i_doubleFormat != nullptr && i_doubleFormat[0] != '\0') ? i_doubleFormat : "%.15g")
                    );
                }
                if (typeOf == typeid(double)) {
                    doFormatValue = bind(
                        FloatSqlFormatHandler<double>,
                        placeholders::_1, placeholders::_2, placeholders::_3,
                        ((i_doubleFormat != nullptr && i_doubleFormat[0] != '\0') ? i_doubleFormat : "%.15g")
                    );
                }
                if (typeOf == typeid(long double)) {
                    doFormatValue = bind(
                        FloatSqlFormatHandler<long double>,
                        placeholders::_1, placeholders::_2, placeholders::_3,
                        ((i_doubleFormat != nullptr && i_doubleFormat[0] != '\0') ? i_doubleFormat : "%.15g")
                    );
                }
                if (typeOf == typeid(string)) {
                    doFormatValue = StrSqlFormatHandler;
                }
            }

            if (doFormatValue == nullptr)
                throw DbException("invalid value type to convert to string");  // conversion to target type is not supported
        }

        /** converter for value column into string.
         *
         * @param[in] i_type             value type, use std::string type for VARCHAR values
         * @param[in] i_doubleFormat     if not null or empty then printf format for float and doubles, default: %.15g
         */
        ValueFormatterBase(const type_info & i_type, const char * i_doubleFormat = "") :
            ValueFormatterBase(i_type, false, i_doubleFormat) {}

        /**
         * IValueFormatter interface implementation: convert value to string using snprintf.
         *
         * @param[in] i_value   db-field value, casted to the target column type
         * @param[in] i_isNull  if true then value is NULL and return is "null"
         *
         * @return value converted to string, must be copied before next call.
         */
        const char * formatValue(const void * i_value, bool i_isNull = false) override
        {
            if (i_isNull || i_value == nullptr) return nullValueString;

            doFormatValue(i_value, valueLen, valueStr);
            return valueStr;
        }

    private:
        /** value type, use std::string type for VARCHAR values */
        const type_info & typeOf;

        /** value buffer to store string of the value */
        char valueStr[valueLen + 1] = "";

        // value to string converter handler
        function<int(const void * i_value, size_t i_size, char * io_buffer)> doFormatValue;

    private:
        ValueFormatterBase(const ValueFormatterBase &) = delete;
        ValueFormatterBase & operator=(const ValueFormatterBase &) = delete;
    };

    typedef ValueFormatterBase<OM_STR_DB_MAX> ValueFormatter;   /** parameter, accumulator or expression value formatter */
    typedef ValueFormatterBase<OM_CODE_DB_MAX> ShortFormatter;  /** microdata csv value formatter: long strings are not supported */

    /** holder for array of primitive values or array of string */
    struct ValueArray
    {
    public:
        /** create new array of values or array of string */
        ValueArray(const type_info & i_type, size_t i_size);

        /** cleanup resources: free memory */
        ~ValueArray(void) noexcept { try { cleanup(); } catch (...) {} }

        /** cleanup resources: free memory */
        void cleanup(void) noexcept;

        /** type of value */
        const type_info & typeOf(void) const { return valueType; }

        /** array size: value count */
        const size_t sizeOf(void) const { return valueCount; }

        /** return pointer to array */
        void * ptr(void) const { return valueArr; }

    private:
        const type_info & valueType;    // value type
        const size_t valueCount;        // size of array: number of values
        void * valueArr;                // array data

        // create new array of primitive values and fill it with NaN
        template<typename TVal> TVal * newValueArray(size_t i_size)
        {
            TVal * pData = new TVal[i_size];
            fill(static_cast<TVal *>(pData), &(static_cast<TVal *>(pData))[i_size], numeric_limits<TVal>::quiet_NaN());
            return pData;
        }

    private:
        ValueArray(const ValueArray & i_src) = delete;
        ValueArray & operator=(const ValueArray & i_src) = delete;
    };

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
        ~ValueRow(void) noexcept { }
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
        function<void(const void * i_value, DbValue & o_dbVal)> doSetValue;

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

    /** row processor to calculate digest of value table row (parameter, accumulator or expression) */
    class ValueRowDigester : public IRowProcessor
    {
    public:
        /** new row digester for value table row, use std::string type for VARCHAR input parameters */
        ValueRowDigester(int i_idCount, const type_info & i_type, MD5 * io_md5, const char * i_doubleFormat = "");

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
        unique_ptr<IValueFormatter> fmtValue;

    private:
        ValueRowDigester(const ValueRowDigester &) = delete;
        ValueRowDigester & operator=(const ValueRowDigester &) = delete;
    };
}

#endif  // DB_VALUE_H
