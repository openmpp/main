/**
 * @file
 * OpenM++ data library: db value classes access value rows: input parameter, accumulator or expression tables
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "dbValue.h"
using namespace openm;

/** clear value with zeros */
void DbValue::clear(void)
{
    isVal = false;
    fVal = 0.0;
    dVal = 0.0;
    dlVal = 0.0;
    llVal = 0;
    ullVal = 0;
    szVal = nullptr;
}

/** return double part of db value */
template<> double DbValue::castDouble<double, double>(const DbValue & i_value)
{
    return i_value.dVal;
}

/** cast to double and return float part of db value */
template<> double DbValue::castDouble<float, double>(const DbValue & i_value)
{
    double dVal = static_cast<double>(i_value.fVal);
    return dVal;
}

/** cast to double and return long double part of db value */
template<> double DbValue::castDouble<long double, double>(const DbValue & i_value)
{
    double dVal = static_cast<double>(i_value.dlVal);
    return dVal;
}

/** convert bool value to string : return "true" or "false" */
int openm::BoolFormatHandler(const void * i_value, size_t i_size, char * io_buffer)
{
    bool isVal = *static_cast<const bool *>(i_value);

    strncpy(io_buffer, (isVal ? trueValueString : falseValueString), i_size);
    io_buffer[i_size - 1] = '\0';
    return (int)strnlen(io_buffer, i_size);
}

/** convert bool value to SQL constant: return "1" or "0" */
int openm::BoolSqlFormatHandler(const void * i_value, size_t i_size, char * io_buffer)
{
    bool isVal = *static_cast<const bool *>(i_value);

    strncpy(io_buffer, (isVal ? "1" : "0"), i_size);
    io_buffer[i_size - 1] = '\0';
    return (int)strnlen(io_buffer, i_size);
}

/** convert varchar value to string: make a copy of source string */
int openm::StrFormatHandler(const void * i_value, size_t i_size, char * io_buffer)
{
    strncpy(io_buffer, static_cast<const char *>(i_value), i_size);
    io_buffer[i_size - 1] = '\0';
    return (int)strnlen(io_buffer, i_size);
}

/** convert string value into SQL constant: return 'quoted source value' */
int openm::StrSqlFormatHandler(const void * i_value, size_t i_size, char * io_buffer)
{
    strncpy(io_buffer, toQuoted(static_cast<const char *>(i_value)).c_str(), i_size);
    io_buffer[i_size - 1] = '\0';
    return (int)strnlen(io_buffer, i_size);
}

/** create DbValue setter.
*
* @param[in] i_type    value type, use std::string type for VARCHAR values
*/
DbValueSetter::DbValueSetter(const type_info & i_type) :
    typeOf(i_type),
    doSetValue(nullptr)
{
    if (i_type == typeid(char)) doSetValue = setValue<char>;
    if (i_type == typeid(unsigned char)) doSetValue = setValue<unsigned char>;
    if (i_type == typeid(short)) doSetValue = setValue<short>;
    if (i_type == typeid(unsigned short)) doSetValue = setValue<unsigned short>;
    if (i_type == typeid(int)) doSetValue = setValue<int>;
    if (i_type == typeid(unsigned int)) doSetValue = setValue<unsigned int>;
    if (i_type == typeid(long)) doSetValue = setValue<long>;
    if (i_type == typeid(unsigned long)) doSetValue = setValue<unsigned long>;
    if (i_type == typeid(long long)) doSetValue = setValue<long long>;
    if (i_type == typeid(unsigned long long)) doSetValue = setValue<unsigned long long>;
    if (i_type == typeid(int8_t)) doSetValue = setValue<int8_t>;
    if (i_type == typeid(uint8_t)) doSetValue = setValue<uint8_t>;
    if (i_type == typeid(int16_t)) doSetValue = setValue<int16_t>;
    if (i_type == typeid(uint16_t)) doSetValue = setValue<uint16_t>;
    if (i_type == typeid(int32_t)) doSetValue = setValue<int32_t>;
    if (i_type == typeid(uint32_t)) doSetValue = setValue<uint32_t>;
    if (i_type == typeid(int64_t)) doSetValue = setValue<int64_t>;
    if (i_type == typeid(uint64_t)) doSetValue = setValue<uint64_t>;
    if (i_type == typeid(bool)) doSetValue = setValue<bool>;
    if (i_type == typeid(float)) doSetValue = setValue<float>;
    if (i_type == typeid(double)) doSetValue = setValue<double>;
    if (i_type == typeid(long double)) doSetValue = setValue<long double>;
    if (i_type == typeid(char *)) doSetValue = setValue<char *>;

    if (doSetValue == nullptr) throw DbException("invalid type to set database value"); // conversion to target type is not supported
}

/** set DbValue by casting a pointer */
void DbValueSetter::set(const void * i_value, DbValue & o_dbVal)
{
    if (i_value == nullptr) throw DbException("invalid NULL pointer to database value");

    doSetValue(i_value, o_dbVal);
}


/** create new array of values or array of string */
ValueArray::ValueArray(const type_info & i_type, size_t i_size) : valueType(i_type), valueCount(i_size), valueArr(nullptr)
{
    if (i_size <= 0 || i_size >= INT_MAX) throw DbException("invalid value array size: %zd", i_size);

    if (i_type == typeid(char)) valueArr = newValueArray<char>(i_size);
    if (i_type == typeid(unsigned char)) valueArr = newValueArray<unsigned char>(i_size);
    if (i_type == typeid(short)) valueArr = newValueArray<short>(i_size);
    if (i_type == typeid(unsigned short)) valueArr = newValueArray<unsigned short>(i_size);
    if (i_type == typeid(int)) valueArr = newValueArray<int>(i_size);
    if (i_type == typeid(unsigned int)) valueArr = newValueArray<unsigned int>(i_size);
    if (i_type == typeid(long)) valueArr = newValueArray<long>(i_size);
    if (i_type == typeid(unsigned long)) valueArr = newValueArray<unsigned long>(i_size);
    if (i_type == typeid(long long)) valueArr = newValueArray<long long>(i_size);
    if (i_type == typeid(unsigned long long)) valueArr = newValueArray<unsigned long long>(i_size);
    if (i_type == typeid(int8_t)) valueArr = newValueArray<int8_t>(i_size);
    if (i_type == typeid(uint8_t)) valueArr = newValueArray<uint8_t>(i_size);
    if (i_type == typeid(int16_t)) valueArr = newValueArray<int16_t>(i_size);
    if (i_type == typeid(uint16_t)) valueArr = newValueArray<uint16_t>(i_size);
    if (i_type == typeid(int32_t)) valueArr = newValueArray<int32_t>(i_size);
    if (i_type == typeid(uint32_t)) valueArr = newValueArray<uint32_t>(i_size);
    if (i_type == typeid(int64_t)) valueArr = newValueArray<int64_t>(i_size);
    if (i_type == typeid(uint64_t)) valueArr = newValueArray<uint64_t>(i_size);
    if (i_type == typeid(bool)) valueArr = newValueArray<bool>(i_size);
    if (i_type == typeid(float)) valueArr = newValueArray<float>(i_size);
    if (i_type == typeid(double)) valueArr = newValueArray<double>(i_size);
    if (i_type == typeid(long double)) valueArr = newValueArray<long double>(i_size);
    if (i_type == typeid(string)) valueArr = new string[i_size];

    if (valueArr == nullptr) throw DbException("invalid value type: %s", i_type.name());   // target type is not supported
}

/** cleanup resources: free memory */
void ValueArray::cleanup(void) noexcept {
    try {
        if (valueType != typeid(string)) {
            delete[] valueArr;      // expected g++ warning: deleting void* is undefined
        }
        else {
            string * strArr = static_cast<string *>(valueArr);
            delete[] strArr;
        }
        valueArr = nullptr;
    }
    catch (...) {}
}

/** create empty row of value table */
ValueRow::ValueRow(int i_idCount, const type_info & i_type) : 
    idCount(i_idCount), 
    isNotNull(false), 
    typeOf(i_type)
{
    if (typeOf == typeid(float)) dbVal = DbValue(numeric_limits<float>::quiet_NaN());
    if (typeOf == typeid(double)) dbVal = DbValue(numeric_limits<double>::quiet_NaN());
    if (typeOf == typeid(long double)) dbVal = DbValue(numeric_limits<long double>::quiet_NaN());

    if (idCount > 0) {
        idArr = unique_ptr<int32_t[]>(new int32_t[idCount]);
        for (int k = 0; k < idCount; k++) {
            idArr[k] = 0;
        }
    }
}

/** new row adapter for value table row, use std::string type for VARCHAR input parameters */
ValueRowAdapter::ValueRowAdapter(int i_idCount, const type_info & i_type) : 
    idCount(i_idCount), 
    typeOf(i_type), 
    doSetValue(nullptr)
{ 
    // column types: integer id columns and value column
    for (int k = 0; k < idCount; k++) {
        typeVec.push_back(&typeid(int32_t));
    }
    typeVec.push_back(&typeOf);     // value column type

    // setter for value column
    if (typeOf == typeid(char)) doSetValue = setValueColumn<char>;
    if (typeOf == typeid(unsigned char)) doSetValue = setValueColumn<unsigned char>;
    if (typeOf == typeid(short)) doSetValue = setValueColumn<short>;
    if (typeOf == typeid(unsigned short)) doSetValue = setValueColumn<unsigned short>;
    if (typeOf == typeid(int)) doSetValue = setValueColumn<int>;
    if (typeOf == typeid(unsigned int)) doSetValue = setValueColumn<unsigned int>;
    if (typeOf == typeid(long)) doSetValue = setValueColumn<long>;
    if (typeOf == typeid(unsigned long)) doSetValue = setValueColumn<unsigned long>;
    if (typeOf == typeid(long long)) doSetValue = setValueColumn<long long>;
    if (typeOf == typeid(unsigned long long)) doSetValue = setValueColumn<unsigned long long>;
    if (typeOf == typeid(int8_t)) doSetValue = setValueColumn<int8_t>;
    if (typeOf == typeid(uint8_t)) doSetValue = setValueColumn<uint8_t>;
    if (typeOf == typeid(int16_t)) doSetValue = setValueColumn<int16_t>;
    if (typeOf == typeid(uint16_t)) doSetValue = setValueColumn<uint16_t>;
    if (typeOf == typeid(int32_t)) doSetValue = setValueColumn<int32_t>;
    if (typeOf == typeid(uint32_t)) doSetValue = setValueColumn<uint32_t>;
    if (typeOf == typeid(int64_t)) doSetValue = setValueColumn<int64_t>;
    if (typeOf == typeid(uint64_t)) doSetValue = setValueColumn<uint64_t>;
    if (typeOf == typeid(bool)) doSetValue = setValueColumn<bool>;
    if (typeOf == typeid(float)) doSetValue = setValueColumn<float>;
    if (typeOf == typeid(double)) doSetValue = setValueColumn<double>;
    if (typeOf == typeid(long double)) doSetValue = setValueColumn<long double>;

    if (doSetValue == nullptr && typeOf != typeid(string)) 
        throw DbException("invalid type to use as input parameter or output table value");  // conversion to target type is not supported
}

/** IRowAdapter interface implementation: set column value */
void ValueRowAdapter::set(IRowBase * i_row, int i_column, const void * i_value) const
{
    if (i_column < 0 || i_column >= 1 + idCount) throw DbException("db column number out of range: %d", i_column);

    ValueRow * row = dynamic_cast<ValueRow *>(i_row);

    if (i_column != idCount) {
        row->idArr[i_column] = (*(int32_t *)i_value);
    }
    else {
        row->isNotNull = i_value != nullptr;
        if (row->isNotNull && typeOf == typeid(float)) row->isNotNull = isfinite(*(float *)i_value);
        if (row->isNotNull && typeOf == typeid(double)) row->isNotNull = isfinite(*(double *)i_value);
        if (row->isNotNull && typeOf == typeid(long double)) row->isNotNull = isfinite(*(long double *)i_value);

        if (row->isNotNull) {
            if (typeOf == typeid(string)) {
                row->strVal = (char *)i_value;
            }
            else {
                doSetValue(i_value, row->dbVal);
            }
        }
    }
}

/** new row digester for value table row, use std::string type for VARCHAR input parameters */
ValueRowDigester::ValueRowDigester(int i_idCount, const type_info & i_type, MD5 * io_md5, const char * i_doubleFormat) :
    idCount(i_idCount),
    typeOf(i_type),
    md5(io_md5)
{
    fmtValue.reset(new ValueFormatter(i_type, i_doubleFormat));
}

/** append row to digest */
void ValueRowDigester::processRow(IRowBaseUptr & i_row)
{
    ValueRow * row = dynamic_cast<ValueRow *>(i_row.get());

    // append expression id, accumulator id, sub-value index and dimensions to digest
    char buf[numeric_limits<long>::digits10 + 1];
    for (int k = 0; k < idCount; k++) {
        int n = snprintf(buf, sizeof(buf), "%d,", row->idArr[k]);
        md5->add(buf, n);
    }

    // append value
    if (typeOf != typeid(string)) {
        const char * sv = fmtValue->formatValue((void *)&row->dbVal, !row->isNotNull);
        md5->add(sv, strnlen(sv, OM_STR_DB_MAX));
    }
    else {
        if (row->isNotNull) {
            md5->add(row->strVal.c_str(), row->strVal.length());
        }
        else {
            md5->add(nullValueString, strnlen(nullValueString, OM_STR_DB_MAX));
        }
    }

    md5->add("\n", 1);   // row delimiter
}
