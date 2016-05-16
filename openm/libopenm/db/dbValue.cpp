/**
 * @file
 * OpenM++ data library: db value classes access value rows: input parameter, accumulator or expression tables
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "dbValue.h"

using namespace openm;

/** NULL value constant for digest calculation */
const char * ValueRowDigester::nullValueForDigest = "_NULL_";

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
    if (i_column >= 0 && i_column <= idCount) {

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
        return;
    }
    throw DbException("db column number out of range");
}

/** append row to digest */
void ValueRowDigester::processRow(IRowBaseUptr & i_row) {
        
    ValueRow * row = dynamic_cast<ValueRow *>(i_row.get());

    // append dimensions, expression id, accumulator id, subsample number to digest
    for (int k = 0; k < idCount; k++) {
        md5->add(&row->idArr[k], sizeof(row->idArr[k]));
    }
                
    // append value
    if (!row->isNotNull) {
        md5->add(&nullValueForDigest, sizeof(nullValueForDigest));
    }
    else {
        if (typeOf == typeid(char)) md5->add(&row->dbVal.llVal, sizeof(DbValue::llVal));
        if (typeOf == typeid(unsigned char)) md5->add(&row->dbVal.ullVal, sizeof(DbValue::ullVal));
        if (typeOf == typeid(short)) md5->add(&row->dbVal.llVal, sizeof(DbValue::llVal));
        if (typeOf == typeid(unsigned short)) md5->add(&row->dbVal.ullVal, sizeof(DbValue::ullVal));
        if (typeOf == typeid(int)) md5->add(&row->dbVal.llVal, sizeof(DbValue::llVal));
        if (typeOf == typeid(unsigned int)) md5->add(&row->dbVal.ullVal, sizeof(DbValue::ullVal));
        if (typeOf == typeid(long)) md5->add(&row->dbVal.llVal, sizeof(DbValue::llVal));
        if (typeOf == typeid(unsigned long)) md5->add(&row->dbVal.ullVal, sizeof(DbValue::ullVal));
        if (typeOf == typeid(long long)) md5->add(&row->dbVal.llVal, sizeof(DbValue::llVal));
        if (typeOf == typeid(unsigned long long)) md5->add(&row->dbVal.ullVal, sizeof(DbValue::ullVal));
        if (typeOf == typeid(int8_t)) md5->add(&row->dbVal.llVal, sizeof(DbValue::llVal));
        if (typeOf == typeid(uint8_t)) md5->add(&row->dbVal.ullVal, sizeof(DbValue::ullVal));
        if (typeOf == typeid(int16_t)) md5->add(&row->dbVal.llVal, sizeof(DbValue::llVal));
        if (typeOf == typeid(uint16_t)) md5->add(&row->dbVal.ullVal, sizeof(DbValue::ullVal));
        if (typeOf == typeid(int32_t)) md5->add(&row->dbVal.llVal, sizeof(DbValue::llVal));
        if (typeOf == typeid(uint32_t)) md5->add(&row->dbVal.ullVal, sizeof(DbValue::ullVal));
        if (typeOf == typeid(int64_t)) md5->add(&row->dbVal.llVal, sizeof(DbValue::llVal));
        if (typeOf == typeid(uint64_t)) md5->add(&row->dbVal.ullVal, sizeof(DbValue::ullVal));
        if (typeOf == typeid(bool)) md5->add(&row->dbVal.isVal, sizeof(DbValue::isVal));
        if (typeOf == typeid(float)) md5->add(&row->dbVal.dVal, sizeof(DbValue::dVal));
        if (typeOf == typeid(double)) md5->add(&row->dbVal.dVal, sizeof(DbValue::dVal));
        if (typeOf == typeid(long double)) md5->add(&row->dbVal.dlVal, sizeof(DbValue::dlVal));
        if (typeOf == typeid(string)) md5->add(row->strVal.c_str(), row->strVal.length());
    }
    md5->add("\n", strlen("\n"));    // row delimiter
}
