// OpenM++ data library: class to produce insert sql statements for new model
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "modelInsertSql.h"

using namespace openm;

// make model prefix from model name and timestamp
const string ModelInsertSql::makeModelPrefix(const string & i_name, const string & i_timestamp)
{
    if (i_name.empty() || trim(i_name).length() < 1) throw DbException("invalid (empty) model name");
    if (i_timestamp.empty() || trim(i_timestamp).length() < 16) throw DbException("invalid (too short) model timestamp");

    // validate model name and timestamp max size
    if (i_name.length() > 32 || i_timestamp.length() > 32) 
        throw DbException("invalid (too long) model name: %s or timestamp: %s", i_name.c_str(), i_timestamp.c_str());

    // in model name use only [a-z,0-9] and _ underscore
    // make sure prefix size not longer than 32 
    string sPrefix = toAlphaNumeric(i_name, 32);
    // toLower(sPrefix);

    // make model prefix from model name and timestamp
    return 
        sPrefix.substr(0, 32 - i_timestamp.length()) + i_timestamp;
}

// make unique part of db table name for parameter or output table, ie: 1234_ageSex
const string ModelInsertSql::makeDbNameSuffix(int i_id, const string & i_src)
{
    if (i_src.empty() || trim(i_src).length() < 1) throw DbException("invalid (empty) source name, id: %d", i_id);

    // in db table name use only [A-Z,a-z,0-9] and _ underscore
    // make sure name size not longer than (32 - max prefix size)
    string sId = to_string(i_id) + "_";
    string sName = toAlphaNumeric(i_src, 32);

    return sId + sName.substr(0, 32 - (sId.length() + OM_DB_TABLE_TYPE_PREFIX_LEN));
}

// return sql to insert into model_dic table if new model created.
template<> const string ModelInsertSql::insertSql<ModelDicRow>(const ModelDicRow & i_row)
{
    // validate field values
    string paramP = !i_row.paramPrefix.empty() ? i_row.paramPrefix : "p";
    string setP = !i_row.setPrefix.empty() ? i_row.setPrefix : "w";
    string subP = !i_row.subPrefix.empty() ? i_row.subPrefix : "s";
    string valueP = !i_row.valuePrefix.empty() ? i_row.valuePrefix : "v";

    if (paramP.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) parameter tables prefix: %s", paramP.c_str());
    if (setP.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) workset tables prefix: %s", setP.c_str());
    if (subP.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) subsample tables prefix: %s", subP.c_str());
    if (valueP.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) value tables prefix: %s", valueP.c_str());

    if (paramP == setP || paramP == subP || paramP == valueP || setP == subP || setP == valueP || subP == valueP)
        throw DbException("invalid (not unique) table prefixes: %s %s %s %s, model name: %s", paramP.c_str(), setP.c_str(), subP.c_str(), valueP.c_str(), i_row.name.c_str());

    // make model prefix from model name and timestamp
    string modelPrefix = makeModelPrefix(i_row.name, i_row.timestamp);

    // creating new model: get new model id from id_lst
    return
        "INSERT INTO model_dic" \
        " (model_id, model_name, model_type, model_ver, model_ts, model_prefix, parameter_prefix, workset_prefix, sub_prefix, value_prefix)" \
        " SELECT" \
        " IL.id_value, " +
        toQuoted(i_row.name) + ", " +
        to_string(i_row.type) + ", " +
        toQuoted(i_row.version) + ", " +
        toQuoted(i_row.timestamp) + ", " +
        toQuoted(modelPrefix) + ", " +
        toQuoted(paramP) + ", " +
        toQuoted(setP) + ", " +
        toQuoted(subP) + ", " +
        toQuoted(valueP) +
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into model_dic_txt table if new model created.
// language name used to select language id
template<> const string ModelInsertSql::insertSql<ModelDicTxtLangRow>(const ModelDicTxtLangRow & i_row)
{
    // validate field values
    if (i_row.langName.empty() || trim(i_row.langName).length() < 1) throw DbException("invalid (empty) language name");
    if (i_row.descr.empty() || trim(i_row.descr).length() < 1) throw DbException("invalid (empty) description");
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes");

    // make sql
    return
        "INSERT INTO model_dic_txt (model_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " \
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(i_row.langName) + "), " +
        toQuoted(i_row.descr) + ", " +
        toQuoted(i_row.note) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into type_dic table if new model created.
template<> const string ModelInsertSql::insertSql<TypeDicRow>(const TypeDicRow & i_row)
{
    // validate field values
    if (i_row.typeId < 0) throw DbException("invalid (negative) type id: %d", i_row.typeId);

    if (i_row.name.empty() || trim(i_row.name).length() < 1) throw DbException("invalid (empty) type name");
    if (i_row.name.length() > 64) throw DbException("invalid (too long) type name: %s", i_row.name.c_str());

    if (i_row.dicId < 0) throw DbException("invalid (negative) dictionary id: %d, type: %s", i_row.dicId, i_row.name.c_str());
    if (i_row.dicId != 0 && i_row.totalEnumId <= 0) 
        throw DbException("invalid id of total item, it must be positive: %d, type: %s", i_row.totalEnumId, i_row.name.c_str());

    // make sql
    return
        "INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.typeId) + ", " +
        toQuoted(i_row.name) + ", " +
        to_string(i_row.dicId) + ", " +
        ((i_row.dicId == 0 && i_row.totalEnumId <= 0) ? "1" : to_string(i_row.totalEnumId)) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into type_dic_txt table if new model created.
// language name used to select language id
template<> const string ModelInsertSql::insertSql<TypeDicTxtLangRow>(const TypeDicTxtLangRow & i_row)
{
    // validate field values
    if (i_row.typeId < 0) throw DbException("invalid (negative) type id: %d", i_row.typeId);

    if (i_row.langName.empty() || trim(i_row.langName).length() < 1) throw DbException("invalid (empty) language name, type id: %d", i_row.typeId);
    if (i_row.descr.empty() || trim(i_row.descr).length() < 1) throw DbException("invalid (empty) description, type id: %d", i_row.typeId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, type id: %d", i_row.typeId);

    // make sql
    return
        "INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.typeId) + ", " +
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(i_row.langName) + "), " +
        toQuoted(i_row.descr) + ", " +
        toQuoted(i_row.note) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// TODO: work in progress
// buffer to format sql statement
static const size_t sqlBufferSize = 1024000;
static char sqlBuffer[sqlBufferSize];

// format sql statement into buffer using vsnprintf()
static void formatSql(const char * i_format, ...)
{
    // format message
    va_list args;
    va_start(args, i_format);
    formatTo(sqlBufferSize, sqlBuffer, i_format, args);
    va_end(args);
}

// TODO: work in progress
// return sql to insert into type_enum_lst table if new model created.
template<> const string ModelInsertSql::insertSql<TypeEnumLstRow>(const TypeEnumLstRow & i_row)
{
    // validate field values
    if (i_row.typeId < 0) throw DbException("invalid (negative) type id: %d", i_row.typeId);

    if (i_row.name.empty() || trim(i_row.name).length() < 1) throw DbException("invalid (empty) enum name, type id: %d", i_row.typeId);
    if (i_row.name.length() > 64) throw DbException("invalid (too long) enum name: %s, type id: %d", i_row.name.c_str(), i_row.typeId);

    if (i_row.enumId < 0) throw DbException("invalid (negative) enum %s id: %d, type id: %d", i_row.name.c_str(), i_row.enumId, i_row.typeId);

    // make sql
    formatSql(
        "INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name)" \
        " SELECT" \
        " IL.id_value, %d, %d, %s" \
        " FROM id_lst IL WHERE IL.id_key = 'model_id';",
        i_row.typeId,
        i_row.enumId,
        toQuoted(i_row.name).c_str()
        );
    return sqlBuffer;

    //return
    //    "INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name)" \
    //    " SELECT" \
    //    " IL.id_value, " +
    //    to_string(i_row.typeId) + ", " +
    //    to_string(i_row.enumId) + ", " +
    //    toQuoted(i_row.name) + 
    //    " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// TODO: work in progress
// return sql to insert into type_enum_txt table if new model created.
// language name used to select language id
template<> const string ModelInsertSql::insertSql<TypeEnumTxtLangRow>(const TypeEnumTxtLangRow & i_row)
{
    // validate field values
    if (i_row.typeId < 0) throw DbException("invalid (negative) type id: %d", i_row.typeId);
    if (i_row.enumId < 0) throw DbException("invalid (negative) enum id: %d, type id: %d", i_row.enumId, i_row.typeId);

    if (i_row.langName.empty() || trim(i_row.langName).length() < 1) throw DbException("invalid (empty) language name, type id: %d, enum id: %d", i_row.typeId, i_row.enumId);
    if (i_row.descr.empty() || trim(i_row.descr).length() < 1) throw DbException("invalid (empty) description, type id: %d, enum id: %d", i_row.typeId, i_row.enumId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, type id: %d, enum id: %d", i_row.typeId, i_row.enumId);

    // make sql
    formatSql(
        "INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value," \
        " %d, %d, " \
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = %s), " \
        " %s, %s"
        " FROM id_lst IL WHERE IL.id_key = 'model_id';",
        i_row.typeId,
        i_row.enumId,
        toQuoted(i_row.langName).c_str(),
        toQuoted(i_row.descr).c_str(),
        toQuoted(i_row.note).c_str()
        );
    return sqlBuffer;

    //return
    //    "INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note)" \
    //    " SELECT" \
    //    " IL.id_value, " +
    //    to_string(i_row.typeId) + ", " +
    //    to_string(i_row.enumId) + ", " +
    //    " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(i_row.langName) + "), " +
    //    toQuoted(i_row.descr) + ", " +
    //    toQuoted(i_row.note) + 
    //    " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into parameter_dic table if new model created.
template<> const string ModelInsertSql::insertSql<ParamDicRow>(const ParamDicRow & i_row)
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) parameter id: %d", i_row.paramId);

    if (i_row.paramName.empty() || trim(i_row.paramName).length() < 1) throw DbException("invalid (empty) parameter name, id: %d", i_row.paramId);
    if (i_row.paramName.length() > 64) throw DbException("invalid (too long) parameter name: %s", i_row.paramName.c_str());
    
    if (i_row.rank < 0) throw DbException("invalid (negative) parameter %s rank: %d", i_row.paramName.c_str(), i_row.rank);
    if (i_row.typeId < 0) throw DbException("invalid (negative) parameter %s type id: %d", i_row.paramName.c_str(), i_row.typeId);
    if (i_row.numCumulated < 0) 
        throw DbException("invalid (negative) number of cumulated dimensions: %d, parameter: %s", i_row.numCumulated, i_row.paramName.c_str());

    // make unique part of db table name from parameter name
    string dbNameSuffix = makeDbNameSuffix(i_row.paramId, i_row.paramName);

    // make sql
    return
        "INSERT INTO parameter_dic" \
        " (model_id, parameter_id, db_name_suffix, parameter_name, parameter_rank, mod_type_id, is_hidden, is_generated, num_cumulated)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.paramId) + ", " +
        toQuoted(dbNameSuffix) + ", " +
        toQuoted(i_row.paramName) + ", " +
        to_string(i_row.rank) + ", " +
        to_string(i_row.typeId) + ", " +
        (i_row.isHidden ? "1" : "0") + ", " +
        (i_row.isGenerated ? "1" : "0") + ", " +
        to_string(i_row.numCumulated) +
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into parameter_dic_txt table if new model created.
// language name used to select language id
template<> const string ModelInsertSql::insertSql<ParamDicTxtLangRow>(const ParamDicTxtLangRow & i_row)
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) parameter id: %d", i_row.paramId);

    if (i_row.langName.empty() || trim(i_row.langName).length() < 1) throw DbException("invalid (empty) language name, parameter id: %d", i_row.paramId);
    if (i_row.descr.empty() || trim(i_row.descr).length() < 1) throw DbException("invalid (empty) description, parameter id: %d", i_row.paramId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, parameter id: %d", i_row.paramId);

    // make sql
    return
        "INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.paramId) + ", " +
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(i_row.langName) + "), " +
        toQuoted(i_row.descr) + ", " +
        toQuoted(i_row.note) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into parameter_dims table if new model created.
template<> const string ModelInsertSql::insertSql<ParamDimsRow>(const ParamDimsRow & i_row)
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) parameter id: %d", i_row.paramId);

    if (i_row.name.empty() || trim(i_row.name).length() < 1) throw DbException("invalid (empty) parameter dimension name");
    if (i_row.name.length() > 8) throw DbException("invalid (too long) parameter dimension name: %s", i_row.name.c_str());
    
    if (i_row.pos < 0) throw DbException("invalid (negative) parameter dimension %s position: %d", i_row.name.c_str(), i_row.pos);
    if (i_row.typeId < 0) throw DbException("invalid (negative) parameter dimension %s type id: %d", i_row.name.c_str(), i_row.typeId);

    // make sql
    return
        "INSERT INTO parameter_dims" \
        " (model_id, parameter_id, dim_name, dim_pos, mod_type_id)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.paramId) + ", " +
        toQuoted(i_row.name) + ", " +
        to_string(i_row.pos) + ", " +
        to_string(i_row.typeId) +
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into table_dic table if new model created.
template<> const string ModelInsertSql::insertSql<TableDicRow>(const TableDicRow & i_row)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);

    if (i_row.tableName.empty() || trim(i_row.tableName).length() < 1) throw DbException("invalid (empty) output table name, id: %d", i_row.tableId);
    if (i_row.tableName.length() > 64) throw DbException("invalid (too long) output table name: %s", i_row.tableName.c_str());
    
    if (i_row.rank < 0) throw DbException("invalid (negative) output table %s rank: %d", i_row.tableName.c_str(), i_row.rank);

    // make db table name from output table name:
    string dbNameSuffix = makeDbNameSuffix(i_row.tableId, i_row.tableName);

    // make sql
    return
        "INSERT INTO table_dic" \
        " (model_id, table_id, db_name_suffix, table_name, is_user, table_rank, is_sparse, is_hidden)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.tableId) + ", " +
        toQuoted(dbNameSuffix) + ", " +
        toQuoted(i_row.tableName) + ", " +
        (i_row.isUser ? "1" : "0") + ", " +
        to_string(i_row.rank) + ", " +
        (i_row.isSparse ? "1" : "0") + ", " +
        (i_row.isHidden ? "1" : "0") +
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into table_dic_txt table if new model created.
// language name used to select language id
template<> const string ModelInsertSql::insertSql<TableDicTxtLangRow>(const TableDicTxtLangRow & i_row)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);

    if (i_row.langName.empty() || trim(i_row.langName).length() < 1) throw DbException("invalid (empty) language name, output table id: %d", i_row.tableId);

    if (i_row.descr.empty() || trim(i_row.descr).length() < 1) throw DbException("invalid (empty) description, output table id: %d", i_row.tableId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, output table id: %d", i_row.tableId);

    if (i_row.unitDescr.empty() || trim(i_row.unitDescr).length() < 1) throw DbException("invalid (empty) analysis dimension description, output table id: %d", i_row.tableId);
    if (i_row.unitDescr.length() > 255 || i_row.unitNote.length() > 32000) throw DbException("invalid (too long) analysis dimension description or notes, output table id: %d", i_row.tableId);

    // make sql
    return
        "INSERT INTO table_dic_txt (model_id, table_id, lang_id, descr, note, unit_descr, unit_note)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.tableId) + ", " +
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(i_row.langName) + "), " +
        toQuoted(i_row.descr) + ", " +
        toQuoted(i_row.note) + ", " +
        toQuoted(i_row.unitDescr) + ", " +
        toQuoted(i_row.unitNote) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into table_dims table if new model created.
template<> const string ModelInsertSql::insertSql<TableDimsRow>(const TableDimsRow & i_row)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);

    if (i_row.name.empty() || trim(i_row.name).length() < 1) throw DbException("invalid (empty) output table dimension name");
    if (i_row.name.length() > 8) throw DbException("invalid (too long) output table dimension name: %s", i_row.name.c_str());
    
    if (i_row.pos < 0) throw DbException("invalid (negative) output table dimension %s position: %d", i_row.name.c_str(), i_row.pos);
    if (i_row.typeId < 0) throw DbException("invalid (negative) output table dimension %s type id: %d", i_row.name.c_str(), i_row.typeId);
    if (i_row.dimSize <= 0) throw DbException("invalid output table dimension %s size: %d", i_row.name.c_str(), i_row.dimSize);

    // make sql
    return
        "INSERT INTO table_dims (model_id, table_id, dim_name, dim_pos, mod_type_id, is_total, dim_size)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.tableId) + ", " +
        toQuoted(i_row.name) + ", " +
        to_string(i_row.pos) + ", " +
        to_string(i_row.typeId) + ", " +
        (i_row.isTotal ? "1" : "0") + ", " +
        to_string(i_row.dimSize) +
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into table_dims_txt table if new model created.
// language name used to select language id
template<> const string ModelInsertSql::insertSql<TableDimsTxtLangRow>(const TableDimsTxtLangRow & i_row)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);

    if (i_row.name.empty() || trim(i_row.name).length() < 1) throw DbException("invalid (empty) output table dimension name");
    if (i_row.name.length() > 8) throw DbException("invalid (too long) output table dimension name: %s", i_row.name.c_str());

    if (i_row.langName.empty() || trim(i_row.langName).length() < 1) 
        throw DbException("invalid (empty) language name, output table id: %d, dimension name: %s", i_row.tableId, i_row.name.c_str());

    if (i_row.descr.empty() || trim(i_row.descr).length() < 1) 
        throw DbException("invalid (empty) description, output table id: %d, dimension name: %s", i_row.tableId, i_row.name.c_str());

    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) 
        throw DbException("invalid (too long) description or notes, output table id: %d, dimension name: %s", i_row.tableId, i_row.name.c_str());

    // make sql
    return
        "INSERT INTO table_dims_txt (model_id, table_id, dim_name, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.tableId) + ", " +
        toQuoted(i_row.name) + ", " +
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(i_row.langName) + "), " +
        toQuoted(i_row.descr) + ", " +
        toQuoted(i_row.note) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into table_acc table if new model created.
template<> const string ModelInsertSql::insertSql<TableAccRow>(const TableAccRow & i_row)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);
    if (i_row.accId < 0) throw DbException("invalid (negative) accumulator id: %d, output table id: %d", i_row.accId, i_row.tableId);

    if (i_row.name.empty() || trim(i_row.name).length() < 1) throw DbException("invalid (empty) accumulator name, id: %d, output table id: %d", i_row.accId, i_row.tableId);
    if (i_row.name.length() > 8) throw DbException("invalid (too long) accumulator name: %s, id: %d, output table id: %d", i_row.name.c_str(), i_row.accId, i_row.tableId);

    if (i_row.expr.empty()) throw DbException("invalid (empty) accumulator expression, id: %d, output table id: %d", i_row.accId, i_row.tableId);
    if (i_row.expr.length() > 255) throw DbException("invalid (too long) accumulator expression: %s, id: %d, output table id: %d", i_row.expr.c_str(), i_row.accId, i_row.tableId);

    // make sql
    return
        "INSERT INTO table_acc (model_id, table_id, acc_id, acc_name, acc_expr)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.tableId) + ", " +
        to_string(i_row.accId) + ", " +
        toQuoted(i_row.name) + ", " +
        toQuoted(i_row.expr) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into table_acc_txt table if new model created.
// language name used to select language id
template<> const string ModelInsertSql::insertSql<TableAccTxtLangRow>(const TableAccTxtLangRow & i_row)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);
    if (i_row.accId < 0) throw DbException("invalid (negative) accumulator id: %d, output table id: %d", i_row.accId, i_row.tableId);

    if (i_row.langName.empty() || trim(i_row.langName).length() < 1) 
        throw DbException("invalid (empty) language name, accumulator id: %d, output table id: %d", i_row.accId, i_row.tableId);

    if (i_row.descr.empty() || trim(i_row.descr).length() < 1) 
        throw DbException("invalid (empty) description, accumulator id: %d, output table id: %d", i_row.accId, i_row.tableId);

    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) 
        throw DbException("invalid (too long) description or notes, accumulator id: %d, output table id: %d", i_row.accId, i_row.tableId);

    // make sql
    return
        "INSERT INTO table_acc_txt (model_id, table_id, acc_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.tableId) + ", " +
        to_string(i_row.accId) + ", " +
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(i_row.langName) + "), " +
        toQuoted(i_row.descr) + ", " +
        toQuoted(i_row.note) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into table_unit table if new model created.
template<> const string ModelInsertSql::insertSql<TableUnitRow>(const TableUnitRow & i_row)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);
    if (i_row.unitId < 0) throw DbException("invalid (negative) output expression id: %d, output table id: %d", i_row.unitId, i_row.tableId);

    if (i_row.name.empty() || trim(i_row.name).length() < 1) throw DbException("invalid (empty) output expression name, output table id: %d", i_row.tableId);
    if (i_row.name.length() > 8) throw DbException("invalid (too long) output expression name: %s, output table id: %d", i_row.name.c_str(), i_row.tableId);

    if (i_row.src.empty()) throw DbException("invalid (empty) source expression, id: %d, output table id: %d", i_row.unitId, i_row.tableId);
    if (i_row.src.length() > 255) throw DbException("invalid (too long) source expression: %s, id: %d, output table id: %d", i_row.src.c_str(), i_row.unitId, i_row.tableId);

    if (i_row.expr.empty()) throw DbException("invalid (empty) output expression, id: %d, output table id: %d", i_row.unitId, i_row.tableId);
    if (i_row.expr.length() > 4000) throw DbException("invalid (too long) output expression: %s, id: %d, output table id: %d", i_row.expr.c_str(), i_row.unitId, i_row.tableId);

    if (i_row.decimals < 0 || i_row.decimals > DBL_DIG) 
        throw DbException("invalid output expression decimals: %d, id: %d, output table id: %d", i_row.decimals, i_row.unitId, i_row.tableId);

    // make sql
    return
        "INSERT INTO table_unit (model_id, table_id, unit_id, unit_name, unit_decimals, unit_src, unit_expr)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.tableId) + ", " +
        to_string(i_row.unitId) + ", " +
        toQuoted(i_row.name) + ", " +
        to_string(i_row.decimals) + ", " +
        toQuoted(i_row.src) + ", " +
        toQuoted(i_row.expr) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into table_unit_txt table if new model created.
// language name used to select language id
template<> const string ModelInsertSql::insertSql<TableUnitTxtLangRow>(const TableUnitTxtLangRow & i_row)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);
    if (i_row.unitId < 0) throw DbException("invalid (negative) output expression id: %d, output table id: %d", i_row.unitId, i_row.tableId);

    if (i_row.langName.empty() || trim(i_row.langName).length() < 1) 
        throw DbException("invalid (empty) language name, expression id: %d, output table id: %d", i_row.unitId, i_row.tableId);

    if (i_row.descr.empty() || trim(i_row.descr).length() < 1) 
        throw DbException("invalid (empty) description, expression id: %d, output table id: %d", i_row.unitId, i_row.tableId);

    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) 
        throw DbException("invalid (too long) description or notes, expression id: %d, output table id: %d", i_row.unitId, i_row.tableId);

    // make sql
    return
        "INSERT INTO table_unit_txt (model_id, table_id, unit_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.tableId) + ", " +
        to_string(i_row.unitId) + ", " +
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(i_row.langName) + "), " +
        toQuoted(i_row.descr) + ", " +
        toQuoted(i_row.note) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into group_lst table if new model created.
template<> const string ModelInsertSql::insertSql<GroupLstRow>(const GroupLstRow & i_row)
{
    // validate field values
    if (i_row.groupId < 0) throw DbException("invalid (negative) group id: %d", i_row.groupId);

    if (i_row.name.empty() || trim(i_row.name).length() < 1) throw DbException("invalid (empty) group name, id: %d", i_row.groupId);
    if (i_row.name.length() > 64) throw DbException("invalid (too long) group name: %s", i_row.name.c_str());
    
    // make sql
    return
        "INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden, is_generated)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.groupId) + ", " +
        (i_row.isParam ? "1" : "0") + ", " +
        toQuoted(i_row.name) + ", " +
        (i_row.isHidden ? "1" : "0") + ", " +
        (i_row.isGenerated ? "1" : "0") +
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into group_txt table if new model created.
// language name used to select language id
template<> const string ModelInsertSql::insertSql<GroupTxtLangRow>(const GroupTxtLangRow & i_row)
{
    // validate field values
    if (i_row.groupId < 0) throw DbException("invalid (negative) group id: %d", i_row.groupId);

    if (i_row.langName.empty() || trim(i_row.langName).length() < 1) throw DbException("invalid (empty) language name, group id: %d", i_row.groupId);

    if (i_row.descr.empty() || trim(i_row.descr).length() < 1) throw DbException("invalid (empty) description, group id: %d", i_row.groupId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, group id: %d", i_row.groupId);

    // make sql
    return
        "INSERT INTO group_txt (model_id, group_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.groupId) + ", " +
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(i_row.langName) + "), " +
        toQuoted(i_row.descr) + ", " +
        toQuoted(i_row.note) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}

// return sql to insert into group_pc table if new model created.
// negative value of i_row.childGroupId or i_row.leafId treated as db-NULL
template<> const string ModelInsertSql::insertSql<GroupPcRow>(const GroupPcRow & i_row)
{
    // validate field values
    if (i_row.groupId < 0) throw DbException("invalid (negative) group id: %d", i_row.groupId);
    if (i_row.childPos < 0) throw DbException("invalid (negative) child position: %d in group id: %d", i_row.childPos, i_row.groupId);
    
    // make sql
    return
        "INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id)" \
        " SELECT" \
        " IL.id_value, " +
        to_string(i_row.groupId) + ", " +
        to_string(i_row.childPos) + ", " +
        (i_row.childGroupId < 0 ? "NULL" : to_string(i_row.childGroupId)) + ", " +
        (i_row.leafId < 0 ? "NULL" : to_string(i_row.leafId)) + 
        " FROM id_lst IL WHERE IL.id_key = 'model_id'";
}
