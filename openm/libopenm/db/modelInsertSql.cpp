// OpenM++ data library: class to produce insert sql statements for new model
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "modelInsertSql.h"

using namespace openm;

// write sql to insert into model_dic table.
template<> void ModelInsertSql::insertSql<ModelDicRow>(const ModelDicRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.version.length() > 255) throw DbException("invalid (too long) model version: %s", i_row.version.c_str());

    if (i_row.paramPrefix.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) parameter tables prefix: %s", i_row.paramPrefix.c_str());
    if (i_row.setPrefix.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) workset tables prefix: %s", i_row.setPrefix.c_str());
    if (i_row.accPrefix.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) accumulator tables prefix: %s", i_row.accPrefix.c_str());
    if (i_row.valuePrefix.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) value tables prefix: %s", i_row.valuePrefix.c_str());

    if (i_row.paramPrefix == i_row.setPrefix || i_row.paramPrefix == i_row.accPrefix || i_row.paramPrefix == i_row.valuePrefix || 
        i_row.setPrefix == i_row.accPrefix || i_row.setPrefix == i_row.valuePrefix || 
        i_row.accPrefix == i_row.valuePrefix)
        throw DbException(
            "invalid (not unique) table prefixes: %s %s %s %s, model name: %s", 
            i_row.paramPrefix.c_str(), i_row.setPrefix.c_str(), i_row.accPrefix.c_str(), i_row.valuePrefix.c_str(), i_row.name.c_str()
            );

    // creating new model: get new model id from id_lst
    io_wr.write(
        "INSERT INTO model_dic" \
        " (model_id, model_name, model_type, model_ver, model_ts," \
        " model_prefix, parameter_prefix, workset_prefix, acc_prefix, value_prefix)" \
        " SELECT" \
        " IL.id_value, ");
    io_wr.writeQuoted(i_row.name, true);
    io_wr.outFs << i_row.type << ", ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.version, true);
    io_wr.writeQuoted(i_row.timestamp, true);
    io_wr.writeQuoted(i_row.modelPrefix, true);
    io_wr.writeQuoted(i_row.paramPrefix, true);
    io_wr.writeQuoted(i_row.setPrefix, true);
    io_wr.writeQuoted(i_row.accPrefix, true);
    io_wr.writeQuoted(i_row.valuePrefix);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into model_dic_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertSql<ModelDicTxtLangRow>(const ModelDicTxtLangRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name");
    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description");
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes");

    // make sql
    io_wr.write(
        "INSERT INTO model_dic_txt (model_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value," \
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ");
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into type_dic table.
template<> void ModelInsertSql::insertSql<TypeDicRow>(const TypeDicRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.typeId < 0) throw DbException("invalid (negative) type id: %d", i_row.typeId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) type name");
    if (i_row.name.length() > 255) throw DbException("invalid (too long) type name: %s", i_row.name.c_str());

    if (i_row.dicId < 0) throw DbException("invalid (negative) dictionary id: %d, type: %s", i_row.dicId, i_row.name.c_str());
    if (i_row.dicId != 0 && i_row.totalEnumId <= 0) 
        throw DbException("invalid id of total item, it must be positive: %d, type: %s", i_row.totalEnumId, i_row.name.c_str());

    // make sql
    io_wr.outFs <<
        "INSERT INTO type_dic (model_id, mod_type_id, mod_type_name, dic_id, total_enum_id)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.typeId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.outFs <<
        i_row.dicId << ", " <<
        (((i_row.dicId == 0 && i_row.totalEnumId <= 0) ? 1 : i_row.totalEnumId)) <<
        " FROM id_lst IL WHERE IL.id_key = 'model_id';\n";
    io_wr.throwOnFail();
}

// write sql to insert into type_dic_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertSql<TypeDicTxtLangRow>(const TypeDicTxtLangRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.typeId < 0) throw DbException("invalid (negative) type id: %d", i_row.typeId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, type id: %d", i_row.typeId);
    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description, type id: %d", i_row.typeId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, type id: %d", i_row.typeId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO type_dic_txt (model_id, mod_type_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.typeId << ",";
    io_wr.throwOnFail();
    io_wr.write(" (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ");
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into type_enum_lst table.
template<> void ModelInsertSql::insertSql<TypeEnumLstRow>(const TypeEnumLstRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.typeId < 0) throw DbException("invalid (negative) type id: %d", i_row.typeId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) enum name, type id: %d", i_row.typeId);
    if (i_row.name.length() > 255) throw DbException("invalid (too long) enum name: %s, type id: %d", i_row.name.c_str(), i_row.typeId);

    if (i_row.enumId < 0) throw DbException("invalid (negative) enum %s id: %d, type id: %d", i_row.name.c_str(), i_row.enumId, i_row.typeId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO type_enum_lst (model_id, mod_type_id, enum_id, enum_name)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.typeId << ", " <<
        i_row.enumId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into type_enum_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertSql<TypeEnumTxtLangRow>(const TypeEnumTxtLangRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.typeId < 0) throw DbException("invalid (negative) type id: %d", i_row.typeId);
    if (i_row.enumId < 0) throw DbException("invalid (negative) enum id: %d, type id: %d", i_row.enumId, i_row.typeId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, type id: %d, enum id: %d", i_row.typeId, i_row.enumId);
    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description, type id: %d, enum id: %d", i_row.typeId, i_row.enumId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, type id: %d, enum id: %d", i_row.typeId, i_row.enumId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO type_enum_txt (model_id, mod_type_id, enum_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.typeId << ", " <<
        i_row.enumId << "," <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName); 
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into parameter_dic table.
template<> void ModelInsertSql::insertSql<ParamDicRow>(const ParamDicRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) parameter id: %d", i_row.paramId);

    if (i_row.paramName.empty() || i_row.paramName.length() < 1) throw DbException("invalid (empty) parameter name, id: %d", i_row.paramId);
    if (i_row.paramName.length() > 255) throw DbException("invalid (too long) parameter name: %s", i_row.paramName.c_str());
    
    if (i_row.rank < 0) throw DbException("invalid (negative) parameter %s rank: %d", i_row.paramName.c_str(), i_row.rank);
    if (i_row.typeId < 0) throw DbException("invalid (negative) parameter %s type id: %d", i_row.paramName.c_str(), i_row.typeId);
    if (i_row.numCumulated < 0) 
        throw DbException("invalid (negative) number of cumulated dimensions: %d, parameter: %s", i_row.numCumulated, i_row.paramName.c_str());

    // make sql
    io_wr.outFs <<
        "INSERT INTO parameter_dic" \
        " (model_id, parameter_id, db_name_suffix, parameter_name, parameter_rank, mod_type_id, is_hidden, is_generated, num_cumulated)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.paramId << ", ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.dbNameSuffix, true);
    io_wr.writeTrimQuoted(i_row.paramName, true);
    io_wr.outFs <<
        i_row.rank << ", " <<
        i_row.typeId << ", " <<
        (i_row.isHidden ? "1, " : "0, ") <<
        (i_row.isGenerated ? "1, " : "0, ") <<
        i_row.numCumulated <<
        " FROM id_lst IL WHERE IL.id_key = 'model_id';\n";
    io_wr.throwOnFail();
}

// write sql to insert into parameter_dic_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertSql<ParamDicTxtLangRow>(const ParamDicTxtLangRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) parameter id: %d", i_row.paramId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, parameter id: %d", i_row.paramId);
    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description, parameter id: %d", i_row.paramId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, parameter id: %d", i_row.paramId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO parameter_dic_txt (model_id, parameter_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.paramId << "," <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into parameter_dims table.
template<> void ModelInsertSql::insertSql<ParamDimsRow>(const ParamDimsRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) parameter id: %d", i_row.paramId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) parameter dimension name");
    if (i_row.name.length() > 8) throw DbException("invalid (too long) parameter dimension name: %s", i_row.name.c_str());
    
    if (i_row.dimId < 0) throw DbException("invalid (negative) parameter dimension %s id: %d", i_row.name.c_str(), i_row.dimId);
    if (i_row.typeId < 0) throw DbException("invalid (negative) parameter dimension %s type id: %d", i_row.name.c_str(), i_row.typeId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO parameter_dims" \
        " (model_id, parameter_id, dim_id, dim_name, mod_type_id)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.paramId << ", " <<
        i_row.dimId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.outFs <<
        i_row.typeId <<
        " FROM id_lst IL WHERE IL.id_key = 'model_id';\n";
    io_wr.throwOnFail();
}

// write sql to insert into table_dic table.
template<> void ModelInsertSql::insertSql<TableDicRow>(const TableDicRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);

    if (i_row.tableName.empty() || i_row.tableName.length() < 1) throw DbException("invalid (empty) output table name, id: %d", i_row.tableId);
    if (i_row.tableName.length() > 255) throw DbException("invalid (too long) output table name: %s", i_row.tableName.c_str());
    
    if (i_row.rank < 0) throw DbException("invalid (negative) output table %s rank: %d", i_row.tableName.c_str(), i_row.rank);
    if (i_row.exprPos < -1 || i_row.exprPos > i_row.rank - 1) throw DbException("invalid output table %s analysis dimension position: %d", i_row.tableName.c_str(), i_row.exprPos);

    // make sql
    io_wr.outFs <<
        "INSERT INTO table_dic" \
        " (model_id, table_id, db_name_suffix, table_name, is_user, table_rank, is_sparse, is_hidden, expr_dim_pos)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.tableId << ", ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.dbNameSuffix, true);
    io_wr.writeTrimQuoted(i_row.tableName, true);
    io_wr.outFs <<
        (i_row.isUser ? "1, " : "0, ") <<
        i_row.rank << ", " <<
        (i_row.isSparse ? "1, " : "0, ") <<
        (i_row.isHidden ? "1, " : "0, ") <<
        i_row.exprPos <<
        " FROM id_lst IL WHERE IL.id_key = 'model_id';\n";
    io_wr.throwOnFail();
}

// write sql to insert into table_dic_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertSql<TableDicTxtLangRow>(const TableDicTxtLangRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, output table id: %d", i_row.tableId);

    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description, output table id: %d", i_row.tableId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, output table id: %d", i_row.tableId);

    if (i_row.exprDescr.empty() || i_row.exprDescr.length() < 1) throw DbException("invalid (empty) analysis dimension description, output table id: %d", i_row.tableId);
    if (i_row.exprDescr.length() > 255 || i_row.exprNote.length() > 32000) throw DbException("invalid (too long) analysis dimension description or notes, output table id: %d", i_row.tableId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO table_dic_txt (model_id, table_id, lang_id, descr, note, expr_descr, expr_note)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.tableId << "," <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note, true);
    io_wr.writeQuoted(i_row.exprDescr, true);
    io_wr.writeQuoted(i_row.exprNote);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into table_dims table.
template<> void ModelInsertSql::insertSql<TableDimsRow>(const TableDimsRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) output table dimension name");
    if (i_row.name.length() > 8) throw DbException("invalid (too long) output table dimension name: %s", i_row.name.c_str());
    
    if (i_row.dimId < 0) throw DbException("invalid (negative) output table dimension %s id: %d", i_row.name.c_str(), i_row.dimId);
    if (i_row.typeId < 0) throw DbException("invalid (negative) output table dimension %s type id: %d", i_row.name.c_str(), i_row.typeId);
    if (i_row.dimSize <= 0) throw DbException("invalid output table dimension %s size: %d", i_row.name.c_str(), i_row.dimSize);

    // make sql
    io_wr.outFs <<
        "INSERT INTO table_dims (model_id, table_id, dim_id, dim_name, mod_type_id, is_total, dim_size)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.tableId << ", " <<
        i_row.dimId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.outFs <<
        i_row.typeId << ", " <<
        (i_row.isTotal ? "1, " : "0, ") <<
        i_row.dimSize <<
        " FROM id_lst IL WHERE IL.id_key = 'model_id';\n";
    io_wr.throwOnFail();
}

// write sql to insert into table_dims_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertSql<TableDimsTxtLangRow>(const TableDimsTxtLangRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);
    if (i_row.dimId < 0) throw DbException("invalid (negative) output table dimension id: %d", i_row.dimId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) 
        throw DbException("invalid (empty) language name, output table id: %d, dimension id: %d", i_row.tableId, i_row.dimId);

    if (i_row.descr.empty() || i_row.descr.length() < 1) 
        throw DbException("invalid (empty) description, output table id: %d, dimension id: %d", i_row.tableId, i_row.dimId);

    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) 
        throw DbException("invalid (too long) description or notes, output table id: %d, dimension id: %d", i_row.tableId, i_row.dimId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO table_dims_txt (model_id, table_id, dim_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.tableId << ", " <<
        i_row.dimId << ", ";
    io_wr.throwOnFail();
    io_wr.write(" (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ");
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into table_acc table.
template<> void ModelInsertSql::insertSql<TableAccRow>(const TableAccRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);
    if (i_row.accId < 0) throw DbException("invalid (negative) accumulator id: %d, output table id: %d", i_row.accId, i_row.tableId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) accumulator name, id: %d, output table id: %d", i_row.accId, i_row.tableId);
    if (i_row.name.length() > 8) throw DbException("invalid (too long) accumulator name: %s, id: %d, output table id: %d", i_row.name.c_str(), i_row.accId, i_row.tableId);

    if (i_row.expr.empty()) throw DbException("invalid (empty) accumulator expression, id: %d, output table id: %d", i_row.accId, i_row.tableId);
    if (i_row.expr.length() > 255) throw DbException("invalid (too long) accumulator expression: %s, id: %d, output table id: %d", i_row.expr.c_str(), i_row.accId, i_row.tableId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO table_acc (model_id, table_id, acc_id, acc_name, acc_expr)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.tableId << ", " <<
        i_row.accId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.writeQuoted(i_row.expr);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into table_acc_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertSql<TableAccTxtLangRow>(const TableAccTxtLangRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);
    if (i_row.accId < 0) throw DbException("invalid (negative) accumulator id: %d, output table id: %d", i_row.accId, i_row.tableId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) 
        throw DbException("invalid (empty) language name, accumulator id: %d, output table id: %d", i_row.accId, i_row.tableId);

    if (i_row.descr.empty() || i_row.descr.length() < 1) 
        throw DbException("invalid (empty) description, accumulator id: %d, output table id: %d", i_row.accId, i_row.tableId);

    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) 
        throw DbException("invalid (too long) description or notes, accumulator id: %d, output table id: %d", i_row.accId, i_row.tableId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO table_acc_txt (model_id, table_id, acc_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.tableId << ", " <<
        i_row.accId << "," <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into table_expr table.
template<> void ModelInsertSql::insertSql<TableExprRow>(const TableExprRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);
    if (i_row.exprId < 0) throw DbException("invalid (negative) output expression id: %d, output table id: %d", i_row.exprId, i_row.tableId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) output expression name, output table id: %d", i_row.tableId);
    if (i_row.name.length() > 8) throw DbException("invalid (too long) output expression name: %s, output table id: %d", i_row.name.c_str(), i_row.tableId);

    if (i_row.src.empty()) throw DbException("invalid (empty) source expression, id: %d, output table id: %d", i_row.exprId, i_row.tableId);
    if (i_row.src.length() > 255) throw DbException("invalid (too long) source expression: %s, id: %d, output table id: %d", i_row.src.c_str(), i_row.exprId, i_row.tableId);

    if (i_row.expr.empty()) throw DbException("invalid (empty) output expression, id: %d, output table id: %d", i_row.exprId, i_row.tableId);
    if (i_row.expr.length() > 2048) throw DbException("invalid (too long) output expression: %s, id: %d, output table id: %d", i_row.expr.c_str(), i_row.exprId, i_row.tableId);

    if (i_row.decimals < 0 || i_row.decimals > DBL_DIG) 
        throw DbException("invalid output expression decimals: %d, id: %d, output table id: %d", i_row.decimals, i_row.exprId, i_row.tableId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO table_expr (model_id, table_id, expr_id, expr_name, expr_decimals, expr_src, expr_sql)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.tableId << ", " <<
        i_row.exprId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.outFs << i_row.decimals << ", ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.src, true);
    io_wr.writeQuoted(i_row.expr);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into table_expr_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertSql<TableExprTxtLangRow>(const TableExprTxtLangRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);
    if (i_row.exprId < 0) throw DbException("invalid (negative) output expression id: %d, output table id: %d", i_row.exprId, i_row.tableId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) 
        throw DbException("invalid (empty) language name, expression id: %d, output table id: %d", i_row.exprId, i_row.tableId);

    if (i_row.descr.empty() || i_row.descr.length() < 1) 
        throw DbException("invalid (empty) description, expression id: %d, output table id: %d", i_row.exprId, i_row.tableId);

    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) 
        throw DbException("invalid (too long) description or notes, expression id: %d, output table id: %d", i_row.exprId, i_row.tableId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO table_expr_txt (model_id, table_id, expr_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.tableId << ", " <<
        i_row.exprId << "," <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into group_lst table.
template<> void ModelInsertSql::insertSql<GroupLstRow>(const GroupLstRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.groupId < 0) throw DbException("invalid (negative) group id: %d", i_row.groupId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) group name, id: %d", i_row.groupId);
    if (i_row.name.length() > 255) throw DbException("invalid (too long) group name: %s", i_row.name.c_str());
    
    // make sql
    io_wr.outFs <<
        "INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden, is_generated)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.groupId << ", " <<
        (i_row.isParam ? "1, " : "0, ");
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.outFs <<
        (i_row.isHidden ? "1, " : "0, ") <<
        (i_row.isGenerated ? "1" : "0") <<
        " FROM id_lst IL WHERE IL.id_key = 'model_id';\n";
    io_wr.throwOnFail();
}

// write sql to insert into group_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertSql<GroupTxtLangRow>(const GroupTxtLangRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.groupId < 0) throw DbException("invalid (negative) group id: %d", i_row.groupId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, group id: %d", i_row.groupId);

    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description, group id: %d", i_row.groupId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, group id: %d", i_row.groupId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO group_txt (model_id, group_id, lang_id, descr, note)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.groupId << "," <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(" FROM id_lst IL WHERE IL.id_key = 'model_id';\n");
}

// write sql to insert into group_pc table.
// negative value of i_row.childGroupId or i_row.leafId treated as db-NULL
template<> void ModelInsertSql::insertSql<GroupPcRow>(const GroupPcRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.groupId < 0) throw DbException("invalid (negative) group id: %d", i_row.groupId);
    if (i_row.childPos < 0) throw DbException("invalid (negative) child position: %d in group id: %d", i_row.childPos, i_row.groupId);
    
    // make sql
    io_wr.outFs <<
        "INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id)" \
        " SELECT" \
        " IL.id_value, " <<
        i_row.groupId + ", " <<
        i_row.childPos + ", " <<
        (i_row.childGroupId < 0 ? "NULL" : to_string(i_row.childGroupId)) << ", " <<
        (i_row.leafId < 0 ? "NULL" : to_string(i_row.leafId)) <<
        " FROM id_lst IL WHERE IL.id_key = 'model_id';\n";
    io_wr.throwOnFail();
}

// write sql to insert into workset_lst table. 
// if i_row.runId <= 0 then it treated as db-NULL
template<> void ModelInsertSql::insertSetSql<WorksetLstRow>(
    const ModelDicRow & i_mdRow, const WorksetLstRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.updateDateTime.length() > 32) throw DbException("invalid (too long) workset update time: %s", i_row.updateDateTime.c_str());

    // creating new workset: get new set id from id_lst
    io_wr.write(
        "INSERT INTO workset_lst" \
        " (set_id, run_id, model_id, set_name, is_readonly, update_dt)" \
        " SELECT" \
        " RSL.id_value, ");
    io_wr.outFs << 
        (i_row.runId <= 0 ? "NULL" : to_string(i_row.runId)) << "," <<
        " (SELECT MD.model_id FROM model_dic MD WHERE MD.model_prefix = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_mdRow.modelPrefix);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.name, true);
    io_wr.write((i_row.isReadonly ? "1, " : "0, "));
    io_wr.writeQuoted(i_row.updateDateTime);
    io_wr.write(" FROM id_lst RSL WHERE RSL.id_key = 'run_id_set_id';\n");
}

// write sql to insert into workset_txt table. 
// language name used to select language id
template<> void ModelInsertSql::insertSetSql<WorksetTxtLangRow>(
    const ModelDicRow & /*i_mdRow*/, const WorksetTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name for workset description and notes");
    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) workset description");
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) workset description or notes");

    // make sql
    io_wr.write(
        "INSERT INTO workset_txt (set_id, model_id, lang_id, descr, note)" \
        " SELECT" \
        " RSL.id_value," \
        " W.model_id," \
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = "
        );
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM id_lst RSL" \
        " INNER JOIN workset_lst W ON (W.set_id = RSL.id_value)" \
        " WHERE RSL.id_key = 'run_id_set_id';\n"
        );
}

// write sql to insert into workset_parameter table. 
template<> void ModelInsertSql::insertSetSql<WorksetParamRow>(
    const ModelDicRow & /*i_mdRow*/, const WorksetParamRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) workset parameter id: %d", i_row.paramId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO workset_parameter (set_id, model_id, parameter_id)" \
        " SELECT" \
        " RSL.id_value," \
        " W.model_id, " <<
        i_row.paramId <<
        " FROM id_lst RSL" \
        " INNER JOIN workset_lst W ON (W.set_id = RSL.id_value)" \
        " WHERE RSL.id_key = 'run_id_set_id';\n";
    io_wr.throwOnFail();
}

// write sql to insert into workset_parameter_txt table. 
// language name used to select language id
template<> void ModelInsertSql::insertSetSql<WorksetParamTxtLangRow>(
    const ModelDicRow & /*i_mdRow*/, const WorksetParamTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) workset parameter id: %d", i_row.paramId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, workset parameter id: %d", i_row.paramId);
    if (i_row.note.length() > 32000) throw DbException("invalid (too long) notes, workset parameter id: %d", i_row.paramId);

    // make sql
    io_wr.outFs <<
        "INSERT INTO workset_parameter_txt (set_id, model_id, parameter_id, lang_id, note)" \
        " SELECT" \
        " RSL.id_value," \
        " W.model_id, " <<
        i_row.paramId << "," <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM id_lst RSL" \
        " INNER JOIN workset_lst W ON (W.set_id = RSL.id_value)" \
        " WHERE RSL.id_key = 'run_id_set_id';\n"
        );
}

// write body of create view sql for parameter compatibility view:
// CREATE VIEW ageSex AS
// SELECT
//  S.dim0 AS "Dim0",
//  S.dim1 AS "Dim1",
//  S.value AS "Value"
// FROM modelone_201208171604590148_p0_ageSex S
// WHERE S.run_id =
// (
//  SELECT MIN(RL.run_id)
//  FROM run_lst RL
//  INNER JOIN model_dic MD ON (MD.model_id = RL.model_id)
//  WHERE MD.model_name = 'modelOne' AND MD.model_ts = '_201208171604590148_'
// );
const void ModelInsertSql::paramCompatibilityViewBody(
    const ModelDicRow & i_modelRow, const string & i_viewName, const string & i_srcTableName, const vector<string> & i_dimNames, ModelSqlWriter & io_wr
    )
{

    io_wr.outFs << "CREATE VIEW " << i_viewName << " AS SELECT";
    io_wr.throwOnFail();

    for (size_t k = 0; k < i_dimNames.size(); k++) {
        io_wr.outFs << " S." << i_dimNames[k] << " AS \"Dim" << k << "\",";
        io_wr.throwOnFail();
    }
    io_wr.write(" S.value AS \"Value\"");

    // from subsample table where run id is first run of that model
    io_wr.outFs <<
        " FROM " << i_srcTableName << " S" <<
        " WHERE S.run_id = (" \
        " SELECT MIN(RL.run_id)" \
        " FROM run_lst RL" \
        " INNER JOIN model_dic MD ON (MD.model_id = RL.model_id)" \
        " WHERE MD.model_name = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_modelRow.name);
    io_wr.write(" AND MD.model_ts = ");
    io_wr.writeQuoted(i_modelRow.timestamp);
    io_wr.write(")\n;\n");
}

// return body of create view sql for output table compatibility view:
// CREATE VIEW salarySex AS
// SELECT
//   S.dim0    AS "Dim0",
//   S.dim1    AS "Dim1",
//   S.expr_id AS "Dim2",
//   S.value   AS "Value"
// FROM modelone_201208171604590148_v0_salarySex S
// WHERE S.run_id =
// (
//  SELECT MIN(RL.run_id)
//  FROM run_lst RL
//  INNER JOIN model_dic MD ON (MD.model_id = RL.model_id)
//  WHERE MD.model_name = 'modelOne' AND MD.model_ts = '_201208171604590148_'
// );
const void ModelInsertSql::outputCompatibilityViewBody(
    const ModelDicRow & i_modelRow, const string & i_viewName, const string & i_srcTableName, const vector<string> & i_dimNames, ModelSqlWriter & io_wr
    )
{
    io_wr.outFs << "CREATE VIEW " << i_viewName << " AS SELECT";
    io_wr.throwOnFail();

    for (size_t k = 0; k < i_dimNames.size(); k++) {
        io_wr.outFs << " S." << i_dimNames[k] << " AS \"Dim" << k << "\",";
        io_wr.throwOnFail();
    }
    io_wr.outFs << 
        " S.expr_id AS \"Dim" << i_dimNames.size() << "\"," << 
        " S.value AS \"Value\"";
    io_wr.throwOnFail();

    // from value table where run id is first run of that model
    io_wr.outFs <<
        " FROM " << i_srcTableName << " S" <<
        " WHERE S.run_id =" \
        " (" \
        " SELECT MIN(RL.run_id)" \
        " FROM run_lst RL" \
        " INNER JOIN model_dic MD ON (MD.model_id = RL.model_id)" \
        " WHERE MD.model_name = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_modelRow.name);
    io_wr.write(" AND MD.model_ts = ");
    io_wr.writeQuoted(i_modelRow.timestamp);
    io_wr.write(")\n;\n");
}

