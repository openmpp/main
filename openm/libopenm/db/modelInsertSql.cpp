// OpenM++ data library: class to produce insert sql statements for new model
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "modelInsertSql.h"

using namespace openm;

// write sql to insert into model_dic table.
template<> void ModelInsertSql::insertTopSql<ModelDicRow>(const ModelDicRow & i_row, ModelSqlWriter & io_wr)
{
    // validate field values
    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) model name");
    if (i_row.name.length() > 255) throw DbException("invalid (too long) model name: %s", i_row.name.c_str());

    if (i_row.digest.empty() || i_row.digest.length() > 32) throw DbException("invalid (or empty) digest, model: %s", i_row.name.c_str());

    if (i_row.version.length() > 32) throw DbException("invalid (too long) model version: %s", i_row.version.c_str());

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

    // UPDATE id_lst 
    // SET id_value = 
    //   CASE
    //     WHEN 
    //       0 = (SELECT COUNT(*) FROM model_dic WHERE model_digest = '1234abcd') THEN id_value + 1 
    //     ELSE id_value
    //   END
    // WHERE id_key = 'model_id';
    io_wr.outFs <<
        "UPDATE id_lst SET id_value =" \
        " CASE" \
        " WHEN 0 = (SELECT COUNT(*) FROM model_dic WHERE model_digest = "; 
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.digest);
    io_wr.outFs <<
        ") THEN id_value + 1" \
        " ELSE id_value" \
        " END" \
        " WHERE id_key = 'model_id';\n";
    io_wr.throwOnFail();
    
    // INSERT INTO model_dic 
    //   (model_id, model_name, model_digest, model_type, model_ver, create_dt, parameter_prefix, workset_prefix, acc_prefix, value_prefix) 
    // SELECT IL.id_value, 'modelOne', '1234abcd', 0, '1.0.0.0', '2012-08-17 16:04:59.0148', 'p', 'w', 'a', 'v'
    // FROM id_lst IL 
    // WHERE IL.id_key = 'model_id'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM model_dic WHERE model_digest = '1234abcd'
    // );
    io_wr.write(
        "INSERT INTO model_dic" \
        " (model_id, model_name, model_digest, model_type, model_ver," \
        " create_dt, parameter_prefix, workset_prefix, acc_prefix, value_prefix)" \
        " SELECT" \
        " IL.id_value, ");
    io_wr.writeQuoted(i_row.name, true);
    io_wr.writeQuoted(i_row.digest, true);
    io_wr.outFs << i_row.type << ", ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.version, true);
    io_wr.writeQuoted(i_row.createDateTime, true);
    io_wr.writeQuoted(i_row.paramPrefix, true);
    io_wr.writeQuoted(i_row.setPrefix, true);
    io_wr.writeQuoted(i_row.accPrefix, true);
    io_wr.writeQuoted(i_row.valuePrefix);
    io_wr.throwOnFail();
    io_wr.write(
        " FROM id_lst IL WHERE IL.id_key = 'model_id'" \
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM model_dic MD WHERE MD.model_digest = ");
    io_wr.writeQuoted(i_row.digest);
    io_wr.writeLine(");");
}

// write sql to insert into model_dic_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertDetailSql<ModelDicRow, ModelDicTxtLangRow>(
    const ModelDicRow & i_modelRow, const ModelDicTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name");
    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description");
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes");

    // INSERT INTO model_dic_txt (model_id, lang_id, descr, note) 
    // SELECT 
    //   MD.model_id, LL.lang_id, 'First model', 'First model note (EN)' 
    // FROM lang_lst LL, model_dic MD
    // WHERE LL.lang_code = 'EN'
    // AND MD.model_digest = '1234abcd'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM model_dic_txt T WHERE T.model_id = MD.model_id AND T.lang_id = LL.lang_id
    // );
    io_wr.write(
        "INSERT INTO model_dic_txt (model_id, lang_id, descr, note)" \
        " SELECT" \
        " MD.model_id, LL.lang_id,");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM lang_lst LL, model_dic MD" \
        " WHERE LL.lang_code = ");
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write(" AND MD.model_digest = ");
    io_wr.writeQuoted(i_modelRow.digest);
    io_wr.write(
        " AND NOT EXISTS" \
        " (SELECT * FROM model_dic_txt T WHERE T.model_id = MD.model_id AND T.lang_id = LL.lang_id);\n");
}

// write sql to insert into type_dic and model_type_dic tables.
template<> void ModelInsertSql::insertDetailSql<ModelDicRow, TypeDicRow>(
    const ModelDicRow & i_modelRow, const TypeDicRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.typeId < 0) throw DbException("invalid (negative) type id: %d", i_row.typeId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) type name");
    if (i_row.name.length() > 255) throw DbException("invalid (too long) type name: %s", i_row.name.c_str());

    if (i_row.digest.empty() || i_row.digest.length() > 32) throw DbException("invalid (or empty) digest, type: %s", i_row.name.c_str());

    if (i_row.dicId < 0) throw DbException("invalid (negative) dictionary id: %d, type: %s", i_row.dicId, i_row.name.c_str());
    if (i_row.dicId != 0 && i_row.totalEnumId <= 0) 
        throw DbException("invalid id of total item, it must be positive: %d, type: %s", i_row.totalEnumId, i_row.name.c_str());

    // UPDATE id_lst SET id_value = 
    //   CASE
    //     WHEN 0 = (SELECT COUNT(*) FROM type_dic WHERE type_digest = 'abcdef') THEN id_value + 1 
    //     ELSE id_value
    //   END
    // WHERE id_key = 'type_hid';
    io_wr.outFs <<
        "UPDATE id_lst SET id_value =" \
        " CASE" \
        " WHEN 0 = (SELECT COUNT(*) FROM type_dic WHERE type_digest = "; 
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.digest);
    io_wr.outFs <<
        ") THEN id_value + 1" \
        " ELSE id_value" \
        " END" \
        " WHERE id_key = 'type_hid';\n";
    io_wr.throwOnFail();

    // INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id)
    // SELECT IL.id_value, 'char', 'abcdef', 0, 1 
    // FROM id_lst IL WHERE IL.id_key = 'type_hid'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM type_dic E WHERE E.type_digest = 'abcdef'
    // );
    io_wr.outFs <<
        "INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id)" \
        " SELECT" \
        " IL.id_value, ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.writeQuoted(i_row.digest, true);
    io_wr.outFs <<
        i_row.dicId << ", " <<
        (((i_row.dicId == 0 && i_row.totalEnumId <= 0) ? 1 : i_row.totalEnumId)) <<
        " FROM id_lst IL WHERE IL.id_key = 'type_hid'" \
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM type_dic E WHERE E.type_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.digest);
    io_wr.writeLine(");");

    // INSERT INTO model_type_dic (model_id, model_type_id, type_hid)
    // SELECT M.model_id, 0, D.type_hid
    // FROM type_dic D, model_dic M
    // WHERE D.type_digest = 'abcdef'
    // AND M.model_digest = '1234abcd'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM model_type_dic E WHERE E.model_id = M.model_id AND E.model_type_id = 0
    // );
    io_wr.outFs <<
        "INSERT INTO model_type_dic (model_id, model_type_id, type_hid)" \
        " SELECT" \
        " M.model_id, " <<
        i_row.typeId <<
        ", D.type_hid" \
        " FROM type_dic D, model_dic M" \
        " WHERE D.type_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.digest);
    io_wr.outFs <<
        " AND M.model_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_modelRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM model_type_dic E WHERE E.model_id = M.model_id AND E.model_type_id = " << i_row.typeId << 
        ");\n";
    io_wr.throwOnFail();
}

// write sql to insert into type_dic_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertDetailSql<TypeDicRow, TypeDicTxtLangRow>(
    const TypeDicRow & i_typeRow, const TypeDicTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, type: %s", i_typeRow.name.c_str());
    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description, type: %s", i_typeRow.name.c_str());
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, type: %s", i_typeRow.name.c_str());

    // INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) 
    // SELECT 
    //   H.type_hid, 
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'Age',
    //   'Age note (EN)' 
    // FROM type_dic H
    // WHERE H.type_digest = 'abcdef'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM type_dic_txt E 
    //   WHERE E.type_hid = H.type_hid 
    //   AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = 'EN')
    // );
    io_wr.write(
        "INSERT INTO type_dic_txt (type_hid, lang_id, descr, note)" \
        " SELECT" \
        " H.type_hid," \
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ");
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM type_dic H" \
        " WHERE H.type_digest = ");
    io_wr.writeQuoted(i_typeRow.digest);
    io_wr.write(
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM type_dic_txt E WHERE E.type_hid = H.type_hid" \
        " AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = "
        );
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.writeLine(") );");
}

// write sql to insert into type_enum_lst table.
template<> void ModelInsertSql::insertDetailSql<TypeDicRow, TypeEnumLstRow>(
    const TypeDicRow & i_typeRow, const TypeEnumLstRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) enum name, type: %s", i_typeRow.name.c_str());
    if (i_row.name.length() > 255) throw DbException("invalid (too long) enum name: %s, type: %s", i_row.name.c_str(), i_typeRow.name.c_str());

    if (i_row.enumId < 0) throw DbException("invalid (negative) enum %s id: %d, type: %s", i_row.name.c_str(), i_row.enumId, i_typeRow.name.c_str());

    // INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) 
    // SELECT 
    //   H.type_hid, 1, 'Middle'
    // FROM type_dic H
    // WHERE H.type_digest = 'abcdef'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM type_enum_lst E WHERE E.type_hid = H.type_hid AND E.enum_id = 1
    // );
    io_wr.outFs <<
        "INSERT INTO type_enum_lst (type_hid, enum_id, enum_name)" \
        " SELECT" \
        "  H.type_hid, " <<
        i_row.enumId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name);
    io_wr.write(" FROM type_dic H  WHERE H.type_digest = ");
    io_wr.writeTrimQuoted(i_typeRow.digest);
    io_wr.outFs << 
        " AND NOT EXISTS" \
        " (SELECT * FROM type_enum_lst E WHERE E.type_hid = H.type_hid AND E.enum_id = " << i_row.enumId << ");\n";
    io_wr.throwOnFail();
}

// write sql to insert into type_enum_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertDetailSql<TypeDicRow, TypeEnumTxtLangRow>(
    const TypeDicRow & i_typeRow, const TypeEnumTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.enumId < 0) throw DbException("invalid (negative) enum id: %d, type: %s", i_row.enumId, i_typeRow.name.c_str());

    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, type: %s, enum id: %d", i_typeRow.name.c_str(), i_row.enumId);
    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description, type: %s, enum id: %d", i_typeRow.name.c_str(), i_row.enumId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, type: %s, enum id: %d", i_typeRow.name.c_str(), i_row.enumId);

    // INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) 
    // SELECT 
    //   H.type_hid,
    //   1, 
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'Middle',
    //   'Middle note (EN)' 
    // FROM type_dic H
    // WHERE H.type_digest = 'abcdef'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM type_enum_txt E WHERE E.type_hid = H.type_hid AND E.enum_id = 1 
    //   AND E.lang_id = 
    //      (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = 'EN')
    // );
    io_wr.outFs <<
        "INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note)" \
        " SELECT" \
        " H.type_hid, " << 
        i_row.enumId << "," <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM type_dic H" \
        " WHERE H.type_digest = ");
    io_wr.writeQuoted(i_typeRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM type_enum_txt E WHERE E.type_hid = H.type_hid" \
        " AND E.enum_id = " << i_row.enumId <<
        " AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.writeLine(") );");
}

// write sql to insert into parameter_dic table.
template<> void ModelInsertSql::insertDetailSql<ModelDicRow, ParamDicRow>(
    const ModelDicRow & i_modelRow, const ParamDicRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) parameter id: %d", i_row.paramId);

    if (i_row.paramName.empty() || i_row.paramName.length() < 1) throw DbException("invalid (empty) parameter name, id: %d", i_row.paramId);
    if (i_row.paramName.length() > 255) throw DbException("invalid (too long) parameter name: %s", i_row.paramName.c_str());
    
    if (i_row.digest.empty() || i_row.digest.length() > 32) throw DbException("invalid (or empty) digest, parameter: %s", i_row.paramName.c_str());
    if (i_row.dbPrefix.empty() || i_row.dbPrefix.length() > 32) throw DbException("invalid (or empty) db name prefix, parameter: %s", i_row.paramName.c_str());
    if (i_row.dbSuffix.empty() || i_row.dbSuffix.length() > 32) throw DbException("invalid (or empty) db name suffix, parameter: %s", i_row.paramName.c_str());

    if (i_row.rank < 0) throw DbException("invalid (negative) parameter %s rank: %d", i_row.paramName.c_str(), i_row.rank);
    if (i_row.typeId < 0) throw DbException("invalid (negative) parameter %s type id: %d", i_row.paramName.c_str(), i_row.typeId);
    if (i_row.numCumulated < 0) 
        throw DbException("invalid (negative) number of cumulated dimensions: %d, parameter: %s", i_row.numCumulated, i_row.paramName.c_str());

    // UPDATE id_lst SET id_value = 
    //   CASE
    //     WHEN 0 = (SELECT COUNT(*) FROM parameter_dic WHERE parameter_digest = 'cd1234ab') THEN id_value + 1 
    //     ELSE id_value
    //   END
    // WHERE id_key = 'parameter_hid';
    io_wr.outFs <<
        "UPDATE id_lst SET id_value =" \
        " CASE" \
        " WHEN 0 = (SELECT COUNT(*) FROM parameter_dic WHERE parameter_digest = "; 
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.digest);
    io_wr.outFs <<
        ") THEN id_value + 1" \
        " ELSE id_value" \
        " END" \
        " WHERE id_key = 'parameter_hid';\n";
    io_wr.throwOnFail();

    // INSERT INTO parameter_dic
    //   (parameter_hid, parameter_name, parameter_digest, db_prefix, db_suffix, parameter_rank, type_hid, num_cumulated)
    // SELECT 
    //   IL.id_value, 
    //   'ageSex', 'cd1234ab', 'ageSex', '87654321', 2, 
    //   (
    //     SELECT T.type_hid 
    //     FROM model_type_dic T 
    //     INNER JOIN model_dic M ON (M.model_id = T.model_id)
    //     WHERE T.model_type_id = 23
    //     AND M.model_digest = '1234abcd'
    //   ),
    //   0
    // FROM id_lst IL 
    // WHERE IL.id_key = 'parameter_hid'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM parameter_dic E WHERE E.parameter_digest = 'cd1234ab'
    // );
    io_wr.write(
        "INSERT INTO parameter_dic" \
        " (parameter_hid, parameter_name, parameter_digest, db_prefix, db_suffix, parameter_rank, type_hid, num_cumulated)" \
        " SELECT IL.id_value, ");
    io_wr.writeTrimQuoted(i_row.paramName, true);
    io_wr.writeQuoted(i_row.digest, true);
    io_wr.writeQuoted(i_row.dbPrefix, true);
    io_wr.writeQuoted(i_row.dbSuffix, true);
    io_wr.outFs <<
        i_row.rank << "," <<
        " (" <<
        " SELECT T.type_hid" \
        " FROM model_type_dic T INNER JOIN model_dic M ON (M.model_id = T.model_id)" \
        " WHERE T.model_type_id = " << i_row.typeId <<
        " AND M.model_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_modelRow.digest);
    io_wr.outFs <<
        "), " <<
        i_row.numCumulated <<
        " FROM id_lst IL" \
        " WHERE IL.id_key = 'parameter_hid'" \
        " AND NOT EXISTS" \
        " (SELECT * FROM parameter_dic E WHERE E.parameter_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.digest);
    io_wr.writeLine(");");

    // INSERT INTO model_parameter_dic 
    //   (model_id, model_parameter_id, parameter_hid, is_hidden, is_generated)
    // SELECT 
    //   M.model_id, 44, D.parameter_hid, 0, 0
    // FROM parameter_dic D, model_dic M
    // WHERE D.parameter_digest = 'cd1234ab'
    // AND M.model_digest = '1234abcd'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM model_parameter_dic E WHERE E.model_id = M.model_id AND E.model_parameter_id = 44
    // );
    io_wr.outFs <<
        "INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden, is_generated)" \
        " SELECT" \
        " M.model_id, " <<
        i_row.paramId << "," <<
        " D.parameter_hid," <<
        (i_row.isHidden ? " 1," : " 0,") <<
        (i_row.isGenerated ? " 1" : " 0") <<
        " FROM parameter_dic D, model_dic M" \
        " WHERE D.parameter_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.digest);
    io_wr.write(" AND M.model_digest = ");
    io_wr.writeQuoted(i_modelRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM model_parameter_dic E WHERE E.model_id = M.model_id AND E.model_parameter_id = " << i_row.paramId << 
        " );\n";
    io_wr.throwOnFail();
}

// write sql to insert into parameter_dic_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertDetailSql<ParamDicRow, ParamDicTxtLangRow>(
    const ParamDicRow & i_paramRow, const ParamDicTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, parameter: %s", i_paramRow.paramName.c_str());
    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description, parameter: %s", i_paramRow.paramName.c_str());
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, parameter: %s", i_paramRow.paramName.c_str());

    // INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) 
    // SELECT 
    //   H.parameter_hid, 
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'Age by Sex',
    //   'Age by Sex note (EN)' 
    // FROM parameter_dic H
    // WHERE H.parameter_digest = 'abcdef'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM parameter_dic_txt E 
    //   WHERE E.parameter_hid = H.parameter_hid 
    //   AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = 'EN')
    // );
    io_wr.write(
        "INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note)" \
        " SELECT" \
        " H.parameter_hid," \
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ");
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM parameter_dic H" \
        " WHERE H.parameter_digest = ");
    io_wr.writeQuoted(i_paramRow.digest);
    io_wr.write(
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM parameter_dic_txt E WHERE E.parameter_hid = H.parameter_hid" \
        " AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = "
        );
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.writeLine(") );");
}

// write sql to insert into parameter_dims table.
template<> void ModelInsertSql::insertDetailSql<ParamDicRow, ParamDimsRow>(
    const ParamDicRow & i_paramRow, const ParamDimsRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) parameter id: %d", i_row.paramId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) parameter dimension name, parameter: %s", i_paramRow.paramName.c_str());
    if (i_row.name.length() > 8) throw DbException("invalid (too long) parameter dimension name: %s, parameter: %s", i_row.name.c_str(), i_paramRow.paramName.c_str());
    
    if (i_row.dimId < 0) throw DbException("invalid (negative) parameter dimension %s id: %d, parameter: %s", i_row.name.c_str(), i_row.dimId, i_paramRow.paramName.c_str());
    if (i_row.typeId < 0) throw DbException("invalid (negative) parameter dimension %s type id: %d, parameter: %s", i_row.name.c_str(), i_row.typeId, i_paramRow.paramName.c_str());

    // INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid)
    // SELECT 
    //   H.parameter_hid, 1, 'dim1', 
    //   (
    //     SELECT T.type_hid 
    //     FROM model_type_dic T 
    //     INNER JOIN model_parameter_dic MP ON (MP.model_id = T.model_id)
    //     WHERE T.model_type_id = 23
    //     AND MP.model_parameter_id = 44
    //   )
    // FROM parameter_dic H
    // WHERE H.parameter_digest = 'abcdef'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM parameter_dims E 
    //   WHERE E.parameter_hid = H.parameter_hid AND E.dim_id = 1
    // );
    io_wr.outFs <<
        "INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid)" \
        " SELECT" \
        " H.parameter_hid, " <<
        i_row.dimId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.outFs <<
        " (" <<
        " SELECT T.type_hid" \
        " FROM model_type_dic T" \
        " INNER JOIN model_parameter_dic MP ON (MP.model_id = T.model_id)" \
        " WHERE T.model_type_id = " << i_row.typeId <<
        " AND MP.model_parameter_id = " << i_paramRow.paramId <<
        " )";
    io_wr.throwOnFail();
    io_wr.write(
        " FROM parameter_dic H" \
        " WHERE H.parameter_digest = ");
    io_wr.writeQuoted(i_paramRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM parameter_dims E WHERE E.parameter_hid = H.parameter_hid" \
        " AND E.dim_id = " << i_row.dimId <<
        " );\n";
    io_wr.throwOnFail();
}

// write sql to insert into parameter_dims_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertDetailSql<ParamDicRow, ParamDimsTxtLangRow>(
    const ParamDicRow & i_paramRow, const ParamDimsTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) parameter id: %d", i_row.paramId);
    if (i_row.dimId < 0) throw DbException("invalid (negative) parameter dimension id: %d", i_row.dimId);

    if (i_row.langName.empty() || i_row.langName.length() < 1)
        throw DbException("invalid (empty) language name, parameter: %s, dimension id: %d", i_paramRow.paramName.c_str(), i_row.dimId);

    if (i_row.descr.empty() || i_row.descr.length() < 1)
        throw DbException("invalid (empty) description, parameter: %s, dimension id: %d", i_paramRow.paramName.c_str(), i_row.dimId);

    if (i_row.descr.length() > 255 || i_row.note.length() > 32000)
        throw DbException("invalid (too long) description or notes, parameter: %s, dimension id: %d", i_paramRow.paramName.c_str(), i_row.dimId);

    // INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) 
    // SELECT 
    //   H.parameter_hid, 
    //   1,
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'Age',
    //   'Age note (EN)' 
    // FROM parameter_dic H
    // WHERE H.parameter_digest = 'abcdef'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM parameter_dims_txt E 
    //   WHERE E.parameter_hid = H.parameter_hid
    //   AND E.dim_id = 1
    //   AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = 'EN')
    // );
    io_wr.outFs <<
        "INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note)" \
        " SELECT" \
        " H.parameter_hid, " <<
        i_row.dimId << ", " <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM parameter_dic H" \
        " WHERE H.parameter_digest = ");
    io_wr.writeQuoted(i_paramRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM parameter_dims_txt E WHERE E.parameter_hid = H.parameter_hid" \
        " AND E.dim_id = " << i_row.dimId <<
        " AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.writeLine(") );");
}

// write sql to insert into table_dic table.
template<> void ModelInsertSql::insertDetailSql<ModelDicRow, TableDicRow>(
    const ModelDicRow & i_modelRow, const TableDicRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_row.tableId);

    if (i_row.tableName.empty() || i_row.tableName.length() < 1) throw DbException("invalid (empty) output table name, id: %d", i_row.tableId);
    if (i_row.tableName.length() > 255) throw DbException("invalid (too long) output table name: %s", i_row.tableName.c_str());

    if (i_row.digest.empty() || i_row.digest.length() > 32) throw DbException("invalid (or empty) digest, parameter: %s", i_row.tableName.c_str());
    if (i_row.dbPrefix.empty() || i_row.dbPrefix.length() > 32) throw DbException("invalid (or empty) db name prefix, parameter: %s", i_row.tableName.c_str());
    if (i_row.dbSuffix.empty() || i_row.dbSuffix.length() > 32) throw DbException("invalid (or empty) db name suffix, parameter: %s", i_row.tableName.c_str());

    if (i_row.rank < 0) throw DbException("invalid (negative) output table %s rank: %d", i_row.tableName.c_str(), i_row.rank);
    if (i_row.exprPos < -1 || i_row.exprPos > i_row.rank - 1) throw DbException("invalid output table %s analysis dimension position: %d", i_row.tableName.c_str(), i_row.exprPos);

    // UPDATE id_lst SET id_value = 
    //   CASE
    //     WHEN 0 = (SELECT COUNT(*) FROM table_dic WHERE table_digest = '987654ab') THEN id_value + 1 
    //     ELSE id_value
    //   END
    // WHERE id_key = 'table_hid';
    io_wr.outFs <<
        "UPDATE id_lst SET id_value =" \
        " CASE" \
        " WHEN 0 = (SELECT COUNT(*) FROM table_dic WHERE table_digest = "; 
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.digest);
    io_wr.outFs <<
        ") THEN id_value + 1" \
        " ELSE id_value" \
        " END" \
        " WHERE id_key = 'table_hid';\n";
    io_wr.throwOnFail();

    // INSERT INTO table_dic
    //   (table_hid, table_name, table_digest, db_prefix, db_suffix, table_rank, is_sparse)
    // SELECT 
    //   IL.id_value, 
    //   'salarySex', '98765432', 'salarySex', 'ffeeddcc', 2, 0
    // FROM id_lst IL 
    // WHERE IL.id_key = 'table_hid'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM table_dic E WHERE E.table_digest = '98765432'
    // );
    io_wr.write(
        "INSERT INTO table_dic" \
        " (table_hid, table_name, table_digest, db_prefix, db_suffix, table_rank, is_sparse)" \
        " SELECT IL.id_value, ");
    io_wr.writeTrimQuoted(i_row.tableName, true);
    io_wr.writeQuoted(i_row.digest, true);
    io_wr.writeQuoted(i_row.dbPrefix, true);
    io_wr.writeQuoted(i_row.dbSuffix, true);
    io_wr.outFs <<
        i_row.rank << ", " <<
        (i_row.isSparse ? "1" : "0") <<
        " FROM id_lst IL" \
        " WHERE IL.id_key = 'table_hid'" \
        " AND NOT EXISTS" \
        " (SELECT * FROM table_dic E WHERE E.table_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.digest);
    io_wr.writeLine(");");

    // INSERT INTO model_table_dic 
    //   (model_id, model_table_id, table_hid, is_user, expr_dim_pos)
    // SELECT 
    //   M.model_id, 0, D.table_hid, 0, 0, 1
    // FROM table_dic D, model_dic M
    // WHERE D.table_digest = '98765432'
    // AND M.model_digest = '1234abcd'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM model_table_dic E WHERE E.model_id = M.model_id AND E.model_table_id = 0
    // );
    io_wr.outFs <<
        "INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos)" \
        " SELECT" \
        " M.model_id, " <<
        i_row.tableId << "," <<
        " D.table_hid," <<
        (i_row.isUser ? "1, " : "0, ") <<
        i_row.exprPos <<
        " FROM table_dic D, model_dic M" \
        " WHERE D.table_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.digest);
    io_wr.write(" AND M.model_digest = ");
    io_wr.writeQuoted(i_modelRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM model_table_dic E WHERE E.model_id = M.model_id AND E.model_table_id = " << i_row.tableId << 
        " );\n";
    io_wr.throwOnFail();
}

// write sql to insert into table_dic_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableDicTxtLangRow>(
    const TableDicRow & i_tableRow, const TableDicTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, output table: %s", i_tableRow.tableName.c_str());

    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description, output table: %s", i_tableRow.tableName.c_str());
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, output table: %s", i_tableRow.tableName.c_str());

    if (i_row.exprDescr.empty() || i_row.exprDescr.length() < 1) throw DbException("invalid (empty) analysis dimension description, output table: %s", i_tableRow.tableName.c_str());
    if (i_row.exprDescr.length() > 255 || i_row.exprNote.length() > 32000) throw DbException("invalid (too long) analysis dimension description or notes, output table: %s", i_tableRow.tableName.c_str());

    // INSERT INTO table_dic_txt (table_hid, lang_id, descr, note, expr_descr, expr_note) 
    // SELECT 
    //   H.table_hid, 
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'Salary by Sex',
    //   'Salary by Sex note (EN)',
    //   '',
    //   ''
    // FROM table_dic H
    // WHERE H.table_digest = '98765432'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM table_dic_txt E 
    //   WHERE E.table_hid = H.table_hid 
    //   AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = 'EN')
    // );
    io_wr.write(
        "INSERT INTO table_dic_txt (table_hid, lang_id, descr, note, expr_descr, expr_note)" \
        " SELECT" \
        " H.table_hid," \
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ");
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note, true);
    io_wr.writeQuoted(i_row.exprDescr, true);
    io_wr.writeQuoted(i_row.exprNote);
    io_wr.write(
        " FROM table_dic H" \
        " WHERE H.table_digest = ");
    io_wr.writeQuoted(i_tableRow.digest);
    io_wr.write(
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM table_dic_txt E WHERE E.table_hid = H.table_hid" \
        " AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = "
        );
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.writeLine(") );");
}

// write sql to insert into table_dims table.
template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableDimsRow>(
    const TableDicRow & i_tableRow, const TableDimsRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.tableId < 0) throw DbException("invalid (negative) output table id: %d", i_tableRow.tableId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) output table dimension name");
    if (i_row.name.length() > 8) throw DbException("invalid (too long) output table dimension name: %s", i_row.name.c_str());
    
    if (i_row.dimId < 0) throw DbException("invalid (negative) output table dimension %s id: %d", i_row.name.c_str(), i_row.dimId);
    if (i_row.typeId < 0) throw DbException("invalid (negative) output table dimension %s type id: %d", i_row.name.c_str(), i_row.typeId);
    if (i_row.dimSize <= 0) throw DbException("invalid output table dimension %s size: %d", i_row.name.c_str(), i_row.dimSize);

    // INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size)
    // SELECT 
    //   H.table_hid, 1, 'dim1', 
    //   (
    //     SELECT T.type_hid 
    //     FROM model_type_dic T 
    //     INNER JOIN model_table_dic MT ON (MT.model_id = T.model_id)
    //     WHERE T.model_type_id = 23
    //     AND MT.model_table_id = 44
    //   )
    // FROM table_dic H
    // WHERE H.table_digest = '98765432'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM table_dims E 
    //   WHERE E.table_hid = H.table_hid AND E.dim_id = 1
    // );
    io_wr.outFs <<
        "INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size)" \
        " SELECT" \
        " H.table_hid, " <<
        i_row.dimId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.outFs <<
        " (" <<
        " SELECT T.type_hid" \
        " FROM model_type_dic T" \
        " INNER JOIN model_table_dic MT ON (MT.model_id = T.model_id)" \
        " WHERE T.model_type_id = " << i_row.typeId <<
        " AND MT.model_table_id = " << i_tableRow.tableId <<
        " ), " <<
        (i_row.isTotal ? "1, " : "0, ") <<
        i_row.dimSize <<
        " FROM table_dic H" \
        " WHERE H.table_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_tableRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM table_dims E WHERE E.table_hid = H.table_hid" \
        " AND E.dim_id = " << i_row.dimId <<
        " );\n";
    io_wr.throwOnFail();
}

// write sql to insert into table_dims_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableDimsTxtLangRow>(
    const TableDicRow & i_tableRow, const TableDimsTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.dimId < 0) throw DbException("invalid (negative) output table dimension id: %d", i_row.dimId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) 
        throw DbException("invalid (empty) language name, output table: %s, dimension id: %d", i_tableRow.tableName.c_str(), i_row.dimId);

    if (i_row.descr.empty() || i_row.descr.length() < 1) 
        throw DbException("invalid (empty) description, output table: %s, dimension id: %d", i_tableRow.tableName.c_str(), i_row.dimId);

    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) 
        throw DbException("invalid (too long) description or notes, output table: %s, dimension id: %d", i_tableRow.tableName.c_str(), i_row.dimId);

    // INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) 
    // SELECT 
    //   H.table_hid, 
    //   1,
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'Salary',
    //   'Salary note (EN)' 
    // FROM table_dic H
    // WHERE H.table_digest = '98765432'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM table_dims_txt E 
    //   WHERE E.table_hid = H.table_hid
    //   AND E.dim_id = 1
    //   AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = 'EN')
    // );
    io_wr.outFs <<
        "INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note)" \
        " SELECT" \
        " H.table_hid, " <<
        i_row.dimId << ", " <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM table_dic H" \
        " WHERE H.table_digest = ");
    io_wr.writeQuoted(i_tableRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM table_dims_txt E WHERE E.table_hid = H.table_hid" \
        " AND E.dim_id = " << i_row.dimId << 
        " AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.writeLine(") );");
}

// write sql to insert into table_acc table.
template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableAccRow>(
    const TableDicRow & i_tableRow, const TableAccRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.accId < 0) throw DbException("invalid (negative) accumulator id: %d, output table: %s", i_row.accId, i_tableRow.tableName.c_str());

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) accumulator name, id: %d, output table: %s", i_row.accId, i_tableRow.tableName.c_str());
    if (i_row.name.length() > 8) throw DbException("invalid (too long) accumulator name: %s, id: %d, output table: %s", i_row.name.c_str(), i_row.accId, i_tableRow.tableName.c_str());

    if (i_row.expr.empty()) throw DbException("invalid (empty) accumulator expression, id: %d, output table: %s", i_row.accId, i_tableRow.tableName.c_str());
    if (i_row.expr.length() > 255) throw DbException("invalid (too long) accumulator expression: %s, id: %d, output table: %s", i_row.expr.c_str(), i_row.accId, i_tableRow.tableName.c_str());

    // INSERT INTO table_acc (table_hid, acc_id, acc_name, acc_expr)
    // SELECT 
    //   H.table_hid, 1, 'acc1', 'value_sum()'
    // FROM table_dic H
    // WHERE H.table_digest = '98765432'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM table_acc E 
    //   WHERE E.table_hid = H.table_hid AND E.acc_id = 1
    // );
    io_wr.outFs <<
        "INSERT INTO table_acc (table_hid, acc_id, acc_name, acc_expr)" \
        " SELECT" \
        " H.table_hid, " <<
        i_row.accId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.writeTrimQuoted(i_row.expr);
    io_wr.write(" FROM table_dic H WHERE H.table_digest = ");
    io_wr.writeQuoted(i_tableRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM table_acc E WHERE E.table_hid = H.table_hid" \
        " AND E.acc_id = " << i_row.accId <<
        " );\n";
    io_wr.throwOnFail();
}

// write sql to insert into table_acc_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableAccTxtLangRow>(
    const TableDicRow & i_tableRow, const TableAccTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.accId < 0) throw DbException("invalid (negative) accumulator id: %d, output table: %s", i_row.accId, i_tableRow.tableName.c_str());

    if (i_row.langName.empty() || i_row.langName.length() < 1) 
        throw DbException("invalid (empty) language name, accumulator id: %d, output table: %s", i_row.accId, i_tableRow.tableName.c_str());

    if (i_row.descr.empty() || i_row.descr.length() < 1) 
        throw DbException("invalid (empty) description, accumulator id: %d, output table: %s", i_row.accId, i_tableRow.tableName.c_str());

    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) 
        throw DbException("invalid (too long) description or notes, accumulator id: %d, output table: %s", i_row.accId, i_tableRow.tableName.c_str());

    // INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) 
    // SELECT 
    //   H.table_hid, 
    //   1,
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'Sum of salary by sex',
    //   '' 
    // FROM table_dic H
    // WHERE H.table_digest = '98765432'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM table_acc_txt E 
    //   WHERE E.table_hid = H.table_hid
    //   AND E.acc_id = 1
    //   AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = 'EN')
    // );
    io_wr.outFs <<
        "INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note)" \
        " SELECT" \
        " H.table_hid, " <<
        i_row.accId << ", " <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM table_dic H" \
        " WHERE H.table_digest = ");
    io_wr.writeQuoted(i_tableRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM table_acc_txt E WHERE E.table_hid = H.table_hid" \
        " AND E.acc_id = " << i_row.accId << 
        " AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.writeLine(") );");
}

// write sql to insert into table_expr table.
template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableExprRow>(
    const TableDicRow & i_tableRow, const TableExprRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.exprId < 0) throw DbException("invalid (negative) output expression id: %d, output table: %s", i_row.exprId, i_tableRow.tableName.c_str());

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) output expression name, output table: %s", i_tableRow.tableName.c_str());
    if (i_row.name.length() > 8) throw DbException("invalid (too long) output expression name: %s, output table: %s", i_row.name.c_str(), i_tableRow.tableName.c_str());

    if (i_row.srcExpr.empty()) throw DbException("invalid (empty) source expression, id: %d, output table: %s", i_row.exprId, i_row.tableId);
    if (i_row.srcExpr.length() > 255) throw DbException("invalid (too long) source expression: %s, id: %d, output table: %s", i_row.srcExpr.c_str(), i_row.exprId, i_tableRow.tableName.c_str());

    if (i_row.sqlExpr.empty()) throw DbException("invalid (empty) output expression, id: %d, output table: %s", i_row.exprId, i_tableRow.tableName.c_str());
    if (i_row.sqlExpr.length() > 2048) throw DbException("invalid (too long) output expression: %s, id: %d, output table: %s", i_row.sqlExpr.c_str(), i_row.exprId, i_tableRow.tableName.c_str());

    if (i_row.decimals < 0 || i_row.decimals > DBL_DIG) 
        throw DbException("invalid output expression decimals: %d, id: %d, output table: %s", i_row.decimals, i_row.exprId, i_tableRow.tableName.c_str());

    // INSERT INTO table_expr (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql)
    // SELECT 
    //   H.table_hid, 1, 'expr1', 4, 'OM_SUM(acc1)',
    //   'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc_value) AS expr1 FROM salarySex_a98765432 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
    // FROM table_dic H
    // WHERE H.table_digest = '98765432'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM table_expr E 
    //   WHERE E.table_hid = H.table_hid AND E.expr_id = 1
    // );
    io_wr.outFs <<
        "INSERT INTO table_expr (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql)" \
        " SELECT" \
        " H.table_hid, " <<
        i_row.exprId << ", ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.outFs << i_row.decimals << ", ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_row.srcExpr, true);
    io_wr.writeQuoted(i_row.sqlExpr);
    io_wr.write(" FROM table_dic H WHERE H.table_digest = ");
    io_wr.writeQuoted(i_tableRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM table_expr E WHERE E.table_hid = H.table_hid" \
        " AND E.expr_id = " << i_row.exprId <<
        " );\n";
    io_wr.throwOnFail();
}

// write sql to insert into table_expr_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertDetailSql<TableDicRow, TableExprTxtLangRow>(
    const TableDicRow & i_tableRow, const TableExprTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.exprId < 0) throw DbException("invalid (negative) output expression id: %d, output table: %s", i_row.exprId, i_tableRow.tableName.c_str());

    if (i_row.langName.empty() || i_row.langName.length() < 1)
        throw DbException("invalid (empty) language name, expression id: %d, output table: %s", i_row.exprId, i_tableRow.tableName.c_str());

    if (i_row.descr.empty() || i_row.descr.length() < 1)
        throw DbException("invalid (empty) description, expression id: %d, output table: %s", i_row.exprId, i_tableRow.tableName.c_str());

    if (i_row.descr.length() > 255 || i_row.note.length() > 32000)
        throw DbException("invalid (too long) description or notes, expression id: %d, output table: %s", i_row.exprId, i_tableRow.tableName.c_str());

    // INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) 
    // SELECT 
    //   H.table_hid, 
    //   1,
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'Sum of acc1',
    //   '' 
    // FROM table_dic H
    // WHERE H.table_digest = '98765432'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM table_expr_txt E 
    //   WHERE E.table_hid = H.table_hid
    //   AND E.expr_id = 1
    //   AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = 'EN')
    // );
    io_wr.outFs <<
        "INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note)" \
        " SELECT" \
        " H.table_hid, " <<
        i_row.exprId << ", " <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM table_dic H" \
        " WHERE H.table_digest = ");
    io_wr.writeQuoted(i_tableRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM table_expr_txt E WHERE E.table_hid = H.table_hid" \
        " AND E.expr_id = " << i_row.exprId << 
        " AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.writeLine(") );");
}

// write sql to insert into group_lst table.
template<> void ModelInsertSql::insertDetailSql<ModelDicRow, GroupLstRow>(
    const ModelDicRow & i_modelRow, const GroupLstRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.groupId < 0) throw DbException("invalid (negative) group id: %d", i_row.groupId);

    if (i_row.name.empty() || i_row.name.length() < 1) throw DbException("invalid (empty) group name, id: %d", i_row.groupId);
    if (i_row.name.length() > 255) throw DbException("invalid (too long) group name: %s", i_row.name.c_str());
    
    // INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden, is_generated)
    // SELECT 
    //   M.model_id, 0, 0, 'TableGroup', 0, 0
    // FROM model_dic M
    // WHERE M.model_digest = '1234abcd'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM group_lst E WHERE E.model_id = M.model_id AND E.group_id = 0
    // );
    io_wr.outFs <<
        "INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden, is_generated)" \
        " SELECT" \
        " M.model_id, " <<
        i_row.groupId << ", " <<
        (i_row.isParam ? "1, " : "0, ");
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.name, true);
    io_wr.outFs <<
        (i_row.isHidden ? "1, " : "0, ") <<
        (i_row.isGenerated ? "1" : "0") <<
        " FROM model_dic M" \
        " WHERE M.model_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_modelRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM group_lst E WHERE E.model_id = M.model_id AND E.group_id = " << i_row.groupId << 
        " );\n";
    io_wr.throwOnFail();
}

// write sql to insert into group_txt table.
// language name used to select language id
template<> void ModelInsertSql::insertDetailSql<ModelDicRow, GroupTxtLangRow>(
    const ModelDicRow & i_modelRow, const GroupTxtLangRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.groupId < 0) throw DbException("invalid (negative) group id: %d", i_row.groupId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, group id: %d", i_row.groupId);

    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) description, group id: %d", i_row.groupId);
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) description or notes, group id: %d", i_row.groupId);

    // INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) 
    // SELECT 
    //   M.model_id, 
    //   0,
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'Table group',
    //   'Table group notes'
    // FROM model_dic M
    // WHERE M.model_digest = '1234abcd'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM group_txt E 
    //   WHERE E.model_id = M.model_id 
    //   AND E.group_id = 0
    //   AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = 'EN')
    // );
    io_wr.outFs <<
        "INSERT INTO group_txt (model_id, group_id, lang_id, descr, note)" \
        " SELECT" \
        " M.model_id, " <<
        i_row.groupId << ", " <<
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.write(
        " FROM model_dic M" \
        " WHERE M.model_digest = ");
    io_wr.writeQuoted(i_modelRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM group_txt E WHERE E.model_id = M.model_id" \
        " AND E.group_id = " << i_row.groupId << ", " <<
        " AND E.lang_id = (SELECT EL.lang_id FROM lang_lst EL WHERE EL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.writeLine(") );");
}

// write sql to insert into group_pc table.
// negative value of i_row.childGroupId or i_row.leafId treated as db-NULL
template<> void ModelInsertSql::insertDetailSql<ModelDicRow, GroupPcRow>(
    const ModelDicRow & i_modelRow, const GroupPcRow & i_row, ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.groupId < 0) throw DbException("invalid (negative) group id: %d", i_row.groupId);
    if (i_row.childPos < 0) throw DbException("invalid (negative) child position: %d in group id: %d", i_row.childPos, i_row.groupId);
    
    // INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id)
    // SELECT 
    //   M.model_id, 0, 1, NULL, NULL
    // FROM model_dic M
    // WHERE M.model_digest = '1234abcd'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM group_pc E WHERE E.model_id = M.model_id AND E.group_id = 0 AND E.child_pos = 1
    // );
    io_wr.outFs <<
        "INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id)" \
        " SELECT" \
        " M.model_id, " <<
        i_row.groupId << ", " <<
        i_row.childPos << ", " <<
        (i_row.childGroupId < 0 ? "NULL" : to_string(i_row.childGroupId)) << ", " <<
        (i_row.leafId < 0 ? "NULL" : to_string(i_row.leafId)) <<
        " FROM model_dic M" \
        " WHERE M.model_digest = ";
    io_wr.throwOnFail();
    io_wr.writeQuoted(i_modelRow.digest);
    io_wr.outFs <<
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM group_pc E WHERE E.model_id = M.model_id" \
        " AND E.group_id = " << i_row.groupId << 
        " AND E.child_pos = " << i_row.childPos << 
        " );\n";
    io_wr.throwOnFail();
}

// write sql to insert into workset_lst table. 
// if i_row.baseRunId <= 0 then it treated as db-NULL
template<> void ModelInsertSql::insertSetSql<WorksetLstRow>(
    const string & i_modelDigestQuoted, 
    const string & i_worksetNameQuoted,
    const WorksetLstRow & i_row, 
    ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.updateDateTime.length() > 32) throw DbException("invalid (too long) workset update time: %s", i_row.updateDateTime.c_str());

    // UPDATE id_lst SET id_value = 
    //   CASE
    //     WHEN 0 = 
    //       (
    //       SELECT COUNT(*) FROM workset_lst EW 
    //       INNER JOIN model_dic EM ON (EM.model_id = EW.model_id)
    //       WHERE EW.set_name = 'modelOne'
    //       AND EM.model_digest = '1234abcd'
    //       ) 
    //       THEN id_value + 1 
    //     ELSE id_value
    //   END
    // WHERE id_key = 'run_id_set_id';
    io_wr.outFs <<
        "UPDATE id_lst SET id_value =" \
        " CASE" \
        " WHEN 0 =" \
        " (" \
        " SELECT COUNT(*) FROM workset_lst EW" \
        " INNER JOIN model_dic EM ON (EM.model_id = EW.model_id)" \
        " WHERE EW.set_name = " << i_worksetNameQuoted <<
        " AND EM.model_digest = " << i_modelDigestQuoted <<
        " )" \
        " THEN id_value + 1" \
        " ELSE id_value" \
        " END" \
        " WHERE id_key = 'run_id_set_id';\n";
    io_wr.throwOnFail();

    // INSERT INTO workset_lst
    //   (set_id, base_run_id, model_id, set_name, is_readonly, update_dt)
    // SELECT 
    //   IL.id_value, 
    //   NULL, 
    //   (SELECT M.model_id FROM model_dic M WHERE M.model_digest = '1234abcd'), 
    //   'modelOne', 
    //   0, 
    //   '2012-08-17 17:43:57.1234'
    // FROM id_lst IL 
    // WHERE IL.id_key = 'run_id_set_id'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM workset_lst EW 
    //   INNER JOIN model_dic EM ON (EM.model_id = EW.model_id)
    //   WHERE EW.set_name = 'modelOne'
    //   AND EM.model_digest = '1234abcd'
    // );
    io_wr.outFs <<
        "INSERT INTO workset_lst" \
        " (set_id, base_run_id, model_id, set_name, is_readonly, update_dt)" \
        " SELECT IL.id_value, " <<
        (i_row.baseRunId <= 0 ? "NULL" : to_string(i_row.baseRunId)) << "," <<
        " (SELECT M.model_id FROM model_dic M WHERE M.model_digest = " << i_modelDigestQuoted << "), " <<
        i_worksetNameQuoted << ", ";
    io_wr.throwOnFail();
    io_wr.write((i_row.isReadonly ? "1, " : "0, "));
    io_wr.writeQuoted(i_row.updateDateTime);
    io_wr.outFs <<
        " FROM id_lst IL" \
        " WHERE IL.id_key = 'run_id_set_id'" \
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM workset_lst EW" \
        " INNER JOIN model_dic EM ON (EM.model_id = EW.model_id)" \
        " WHERE EW.set_name = " << i_worksetNameQuoted <<
        " AND EM.model_digest = " << i_modelDigestQuoted <<
        " );\n";
    io_wr.throwOnFail();
}

// write sql to insert into workset_txt table. 
// language name used to select language id
template<> void ModelInsertSql::insertSetSql<WorksetTxtLangRow>(
    const string & i_modelDigestQuoted, 
    const string & i_worksetNameQuoted,
    const WorksetTxtLangRow & i_row, 
    ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name for workset description and notes");
    if (i_row.descr.empty() || i_row.descr.length() < 1) throw DbException("invalid (empty) workset description");
    if (i_row.descr.length() > 255 || i_row.note.length() > 32000) throw DbException("invalid (too long) workset description or notes");

    // INSERT INTO workset_txt (set_id, lang_id, descr, note) 
    // SELECT 
    //   W.set_id, 
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'default workset',
    //   'default workset notes'
    // FROM workset_lst W
    // INNER JOIN model_dic M ON (M.model_id = W.model_id)
    // WHERE W.set_name = 'modelOne'
    // AND M.model_digest = '1234abcd';
    io_wr.write(
        "INSERT INTO workset_txt (set_id, lang_id, descr, note)" \
        " SELECT" \
        " W.set_id," \
        " (" \
        " SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = "
        );
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.descr, true);
    io_wr.writeQuoted(i_row.note);
    io_wr.outFs <<
        " FROM workset_lst W" \
        " INNER JOIN model_dic M ON (M.model_id = W.model_id)" \
        " WHERE W.set_name = " << i_worksetNameQuoted <<
        " AND M.model_digest = " << i_modelDigestQuoted << ";\n";
    io_wr.throwOnFail();
}

// write sql to insert into workset_parameter table. 
template<> void ModelInsertSql::insertSetSql<WorksetParamRow>(
    const string & i_modelDigestQuoted, 
    const string & i_worksetNameQuoted,
    const WorksetParamRow & i_row, 
    ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) workset parameter id: %d", i_row.paramId);

    // INSERT INTO workset_parameter (set_id, parameter_hid)
    // SELECT 
    //   W.set_id, P.parameter_hid
    // FROM workset_lst W
    // INNER JOIN model_dic M ON (M.model_id = W.model_id)
    // INNER JOIN model_parameter_dic P ON (P.model_id = W.model_id AND P.model_parameter_id = 1)
    // WHERE W.set_name = 'modelOne'
    // AND M.model_digest = '1234abcd';
    io_wr.outFs <<
        "INSERT INTO workset_parameter (set_id, parameter_hid)" \
        " SELECT" \
        " W.set_id, P.parameter_hid" \
        " FROM workset_lst W" \
        " INNER JOIN model_dic M ON (M.model_id = W.model_id)" \
        " INNER JOIN model_parameter_dic P ON (P.model_id = W.model_id AND P.model_parameter_id = " << i_row.paramId << ")"
        " WHERE W.set_name = " << i_worksetNameQuoted <<
        " AND M.model_digest = " << i_modelDigestQuoted << ";\n";
    io_wr.throwOnFail();
}

// write sql to insert into workset_parameter_txt table. 
// language name used to select language id
template<> void ModelInsertSql::insertSetSql<WorksetParamTxtLangRow>(
    const string & i_modelDigestQuoted, 
    const string & i_worksetNameQuoted,
    const WorksetParamTxtLangRow & i_row, 
    ModelSqlWriter & io_wr
    )
{
    // validate field values
    if (i_row.paramId < 0) throw DbException("invalid (negative) workset parameter id: %d", i_row.paramId);

    if (i_row.langName.empty() || i_row.langName.length() < 1) throw DbException("invalid (empty) language name, workset parameter id: %d", i_row.paramId);
    if (i_row.note.length() > 32000) throw DbException("invalid (too long) notes, workset parameter id: %d", i_row.paramId);

    // INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note)
    // SELECT 
    //   W.set_id, P.parameter_hid,
    //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
    //   'parameter value notes'
    // FROM workset_lst W
    // INNER JOIN model_dic M ON (M.model_id = W.model_id)
    // INNER JOIN model_parameter_dic P ON (P.model_id = W.model_id AND P.model_parameter_id = 1)
    // WHERE W.set_name = 'modelOne'
    // AND M.model_digest = '1234abcd';
    io_wr.outFs <<
        "INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note)" \
        " SELECT" \
        " W.set_id, P.parameter_hid" \
        " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = ";
    io_wr.throwOnFail();
    io_wr.writeTrimQuoted(i_row.langName);
    io_wr.write("), ");
    io_wr.writeQuoted(i_row.note);
    io_wr.outFs <<
        " FROM workset_lst W" \
        " INNER JOIN model_dic M ON (M.model_id = W.model_id)" \
        " INNER JOIN model_parameter_dic P ON (P.model_id = W.model_id AND P.model_parameter_id = " << i_row.paramId << ")"
        " WHERE W.set_name = " << i_worksetNameQuoted <<
        " AND M.model_digest = " << i_modelDigestQuoted << ";\n";
    io_wr.throwOnFail();
}

