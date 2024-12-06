// OpenM++ data library: class to produce insert sql statements for new model
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "modelInsertSql.h"

using namespace openm;

// insert model master row into model_dic table.
void ModelInsertSql::insertModelDic(IDbExec * i_dbExec, ModelDicLangRow & io_row, const map<string, int> & i_langMap)
{
    // validate field values
    MessageEllipter me;
    if (io_row.name.empty()) throw DbException(LT("invalid (empty) model name"));
    if (io_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) model name: %s"), OM_NAME_DB_MAX, me.ellipt(io_row.name));
    if (io_row.digest.empty() || io_row.digest.length() > OM_CODE_DB_MAX) throw DbException(LT("invalid (empty or longer than %d) digest, model: %s"), OM_CODE_DB_MAX, io_row.name.c_str());
    if (io_row.version.length() > OM_CODE_DB_MAX) throw DbException(LT("invalid (longer than %d) model version: %s"), OM_CODE_DB_MAX, me.ellipt(io_row.version));
    if (io_row.defaultLangCode.empty()) throw DbException(LT("invalid (empty) default model language code"));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.defaultLangCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s"), io_row.defaultLangCode.c_str());
    io_row.defaultLangId = langIt->second;

    // get new model id: must be positive
    i_dbExec->update(
        "UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'model_id'"
    );
    io_row.modelId = i_dbExec->selectToInt(
        "SELECT id_value FROM id_lst WHERE id_key = 'model_id'", 
        -1);
    if (io_row.modelId <= 0) throw DbException(LT("invalid (not positive) model id, model: %s"), io_row.name.c_str());

    // INSERT INTO model_dic 
    //   (model_id, model_name, model_digest, model_type, model_ver, create_dt) 
    // VALUES 
    //   (1234, 'modelOne', '1234abcd', 0, '1.0.0.0', '2012-08-17 16:04:59.0148')
    i_dbExec->update(
        "INSERT INTO model_dic" \
        " (model_id, model_name, model_digest, model_type, model_ver, create_dt, default_lang_id)" \
        " VALUES (" +
        to_string(io_row.modelId) + ", " +
        toQuoted(io_row.name) + ", " +
        toQuoted(io_row.digest) + ", " +
        to_string(io_row.type) + ", " +
        toQuoted(io_row.version) + ", " +
        toQuoted(io_row.createDateTime) + ", " + 
        to_string(io_row.defaultLangId) + 
        ")");
}
    
// insert language into lang_lst table, if not exist
// update language list id's with actual database values
void ModelInsertSql::insertLangLst(IDbExec * i_dbExec, LangLstRow & io_row)
{
    // validate field values
    MessageEllipter me;
    if (io_row.code.empty()) throw DbException(LT("invalid (empty) language code"));
    if (io_row.code.length() > OM_CODE_DB_MAX) throw DbException(LT("invalid (longer than %d) language code: %s"), OM_CODE_DB_MAX, me.ellipt(io_row.code));
    if (io_row.name.empty()) throw DbException(LT("invalid (empty) language name"));
    if (io_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) language name: %s"), OM_NAME_DB_MAX, me.ellipt(io_row.name));

    // get new language id if language not exist
    //
    // UPDATE id_lst SET id_value =
    //   CASE
    //     WHEN 0 = (SELECT COUNT(*) FROM lang_lst WHERE lang_code = 'EN') THEN id_value + 1
    //     ELSE id_value
    //   END
    // WHERE id_key = 'lang_id'
    i_dbExec->update(
        "UPDATE id_lst SET id_value =" \
        " CASE" \
        " WHEN 0 = (SELECT COUNT(*) FROM lang_lst WHERE lang_code = " + toQuoted(io_row.code) + ")" +
        " THEN id_value + 1" +
        " ELSE id_value" +
        " END" +
        " WHERE id_key = 'lang_id'");

    // insert language if not already exist
    //
    // INSERT INTO lang_lst (lang_id, lang_code, lang_name) 
    // SELECT
    //   IL.id_value, 'EN', 'English'
    // FROM id_lst IL
    // WHERE IL.id_key = 'lang_id'
    // AND NOT EXISTS
    // (
    //   SELECT * FROM lang_lst E WHERE E.lang_code = 'EN'
    // );
    i_dbExec->update(
        "INSERT INTO lang_lst (lang_id, lang_code, lang_name)" \
        " SELECT" \
        " IL.id_value, " + toQuoted(io_row.code) + ", " + toQuoted(io_row.name) +
        " FROM id_lst IL" \
        " WHERE IL.id_key = 'lang_id'" \
        " AND NOT EXISTS" \
        " (" \
        "   SELECT * FROM lang_lst E WHERE E.lang_code = " + toQuoted(io_row.code) +
        " )");

    // update language id with actual database value: select new or existing language id
    io_row.langId = i_dbExec->selectToInt(
        "SELECT lang_id FROM lang_lst WHERE lang_code = " + toQuoted(io_row.code),
        -1);
    if (io_row.langId < 0) throw DbException(LT("invalid (negative) language id, for language: %s"), io_row.code.c_str());
}

// insert row into model_dic_txt table.
void ModelInsertSql::insertModelDicText(IDbExec * i_dbExec, const map<string, int> & i_langMap, ModelDicTxtLangRow & io_row)
{
    // validate field values
    MessageEllipter me;
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code for model description or notes"));
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) model description"));
    if (io_row.descr.length() > OM_DESCR_DB_MAX) throw DbException(LT("invalid (longer than %d) model description: %s"), OM_DESCR_DB_MAX, me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX) throw DbException(LT("invalid (longer than %d) model notes: %s"), OM_STR_DB_MAX, me.ellipt(io_row.note));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s"), io_row.langCode.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (1234, 101, 'First model', 'First model note')
    i_dbExec->update(
        "INSERT INTO model_dic_txt (model_id, lang_id, descr, note) VALUES (" +
        to_string(io_row.modelId) + ", " +
        to_string(io_row.langId) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into type_dic and model_type_dic tables, return type Hid.
int ModelInsertSql::insertTypeDic(IDbExec * i_dbExec, const TypeDicRow & i_row)
{
    // validate field values
    MessageEllipter me;
    if (i_row.typeId < 0) throw DbException(LT("invalid (negative) type id: %d"), i_row.typeId);

    if (i_row.name.empty()) throw DbException(LT("invalid (empty) type name"));
    if (i_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) type name: %s"), OM_NAME_DB_MAX, me.ellipt(i_row.name));

    if (i_row.digest.empty() || i_row.digest.length() > OM_CODE_DB_MAX) throw DbException(LT("invalid (empty or longer then %d) digest of type: %s"), OM_CODE_DB_MAX, i_row.name.c_str());

    if (i_row.dicId < 0) throw DbException(LT("invalid (negative) dictionary id: %d, type: %s"), i_row.dicId, i_row.name.c_str());

    // insert new type_dic row, if type not built-in
    // built-in types must already be inserted
    if (!i_row.isBuiltIn()) {

        // reserve next type Hid
        i_dbExec->update("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'type_hid'");

        // INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id)
        // SELECT IL.id_value, 'char', 'abcdef', 0, 1 
        // FROM id_lst IL WHERE IL.id_key = 'type_hid'
        // AND NOT EXISTS
        // (
        //   SELECT * FROM type_dic E WHERE E.type_digest = 'abcdef'
        // )
        i_dbExec->update(
            "INSERT INTO type_dic (type_hid, type_name, type_digest, dic_id, total_enum_id)" \
            " SELECT" \
            " IL.id_value, " +
            toQuoted(i_row.name) + ", " +
            toQuoted(i_row.digest) + ", " +
            to_string(i_row.dicId) + ", " +
            to_string(((i_row.dicId == 0 && i_row.totalEnumId <= 0) ? 1 : i_row.totalEnumId)) +
            " FROM id_lst IL WHERE IL.id_key = 'type_hid'" \
            " AND NOT EXISTS" \
            " (" \
            " SELECT * FROM type_dic E WHERE E.type_digest = " + toQuoted(i_row.digest) +
            " )");
    }

    // get type Hid, it may be existing built-in type or model-specific type with Hid reserved from id_lst
    int hId = i_dbExec->selectToInt(
        "SELECT type_hid FROM type_dic E WHERE E.type_digest = " + toQuoted(i_row.digest),
        -1);
    if (hId <= 0) throw DbException(LT("invalid (not positive) type Hid, type: %s %s"), i_row.name.c_str(), i_row.digest.c_str());
    
    // INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (1234, 10, 7890)
    i_dbExec->update(
        "INSERT INTO model_type_dic (model_id, model_type_id, type_hid) VALUES (" +
        to_string(i_row.modelId) + ", " +
        to_string(i_row.typeId) + ", " +
        to_string(hId) +
        ")");

    return hId;
}

// insert row into type_dic_txt table. 
void ModelInsertSql::insertTypeText(
    IDbExec * i_dbExec, const map<string, int> & i_langMap, const map<int, int> & i_typeIdMap, TypeDicTxtLangRow & io_row
)
{
    // validate field values
    MessageEllipter me;
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, type id: %d"), io_row.typeId);
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, type id: %d"), io_row.typeId);
    if (io_row.descr.length() > OM_DESCR_DB_MAX) throw DbException(LT("invalid (longer than %d) type description: %s"), OM_DESCR_DB_MAX, me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX) throw DbException(LT("invalid (longer than %d) type notes: %s"), OM_STR_DB_MAX, me.ellipt(io_row.note));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s of type id: %d"), io_row.langCode.c_str(), io_row.typeId);
    io_row.langId = langIt->second;

    // find type Hid by model type id
    map<int, int>::const_iterator typeIt = i_typeIdMap.find(io_row.typeId);
    if (typeIt == i_typeIdMap.cend()) throw DbException(LT("invalid type id: %d"), io_row.typeId);

    // built-in types must already be inserted
    if (isBuiltInType(io_row.typeId)) return; 

    // INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (7890, 101, 'Age', 'Age note')
    i_dbExec->update(
        "INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (" +
        to_string(typeIt->second) + ", " +
        to_string(io_row.langId) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into type_enum_lst table.
void ModelInsertSql::insertTypeEnum(IDbExec * i_dbExec, int i_typeHid, const TypeEnumLstRow & i_row)
{
    // validate field values
    MessageEllipter me;
    if (i_row.name.empty()) throw DbException(LT("invalid (empty) enum name, type id: %d"), i_row.typeId);
    if (i_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) enum name: %s, type id: %d"), OM_NAME_DB_MAX, me.ellipt(i_row.name), i_row.typeId);

    // built-in types must already be inserted
    if (isBuiltInType(i_row.typeId)) return; 

    // INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (7890, 1, 'Middle')
    i_dbExec->update(
        "INSERT INTO type_enum_lst (type_hid, enum_id, enum_name) VALUES (" +
        to_string(i_typeHid) + ", " +
        to_string(i_row.enumId) + ", " +
        toQuoted(i_row.name) +
        ")");
}

// insert row into type_enum_txt table.
void ModelInsertSql::insertTypeEnumText(
    IDbExec * i_dbExec, const map<string, int> & i_langMap, int i_typeHid, TypeEnumTxtLangRow & io_row
)
{
    // validate field values
    MessageEllipter me;
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, type id: %d, enum id: %d"), io_row.typeId, io_row.enumId);
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, type id: %d, enum id: %d"), io_row.typeId, io_row.enumId);
    if (io_row.descr.length() > OM_DESCR_DB_MAX) 
        throw DbException(LT("invalid (longer than %d) enum description, type id: %d, enum id: %d, description: %s"), OM_DESCR_DB_MAX, io_row.typeId, io_row.enumId, me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX)
        throw DbException(LT("invalid (longer than %d) enum notes, type id: %d, enum id: %d, notes: %s"), OM_STR_DB_MAX, io_row.typeId, io_row.enumId, me.ellipt(io_row.note));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s of type id: %d"), io_row.langCode.c_str(), io_row.typeId);
    io_row.langId = langIt->second;

    // built-in types must already be inserted
    if (isBuiltInType(io_row.typeId)) return; 

    // INSERT INTO type_dic_txt (type_hid, lang_id, descr, note) VALUES (7890, 101, 'Age', 'Age note')
    i_dbExec->update(
        "INSERT INTO type_enum_txt (type_hid, enum_id, lang_id, descr, note) VALUES (" +
        to_string(i_typeHid) + ", " +
        to_string(io_row.enumId) + ", " +
        to_string(io_row.langId) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into parameter_dic and model_parameter_dic tables. 
void ModelInsertSql::insertParamDic(IDbExec * i_dbExec, const map<int, int> & i_typeIdMap, ParamDicRow & io_row)
{
    // validate field values
    MessageEllipter me;
    if (io_row.paramId < 0) throw DbException(LT("invalid (negative) parameter id: %d"), io_row.paramId);

    if (io_row.paramName.empty()) throw DbException(LT("invalid (empty) parameter name, id: %d"), io_row.paramId);
    if (io_row.paramName.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) parameter name: %s"), OM_NAME_DB_MAX, me.ellipt(io_row.paramName.c_str()));

    if (io_row.digest.empty() || io_row.digest.length() > OM_CODE_DB_MAX || io_row.importDigest.empty() || io_row.importDigest.length() > OM_CODE_DB_MAX)
        throw DbException(LT("invalid (empty or longer then %d) digest of parameter: %s"), OM_CODE_DB_MAX, io_row.paramName.c_str());

    if (io_row.dbRunTable.empty() || io_row.dbRunTable.length() > 64 || io_row.dbSetTable.empty() || io_row.dbSetTable.length() > 64)
        throw DbException(LT("invalid (longer than 64) db table name, parameter: %s"), io_row.paramName.c_str());

    if (io_row.rank < 0) throw DbException(LT("invalid (negative) parameter %s rank: %d"), io_row.paramName.c_str(), io_row.rank);
    if (io_row.numCumulated < 0)
        throw DbException(LT("invalid (negative) number of cumulated dimensions: %d, parameter: %s"), io_row.numCumulated, io_row.paramName.c_str());

    // find type Hid by model type id
    map<int, int>::const_iterator typeIt = i_typeIdMap.find(io_row.typeId);
    if (typeIt == i_typeIdMap.cend()) throw DbException(LT("invalid parameter %s type id: %d"), io_row.paramName.c_str(), io_row.typeId);

    // get new parameter Hid
    i_dbExec->update(
        "UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'parameter_hid'"
    );
    io_row.paramHid = i_dbExec->selectToInt(
        "SELECT id_value FROM id_lst WHERE id_key = 'parameter_hid'",
        -1);
    if (io_row.paramHid <= 0) throw DbException(LT("invalid (not positive) parameter Hid, parameter: %s"), io_row.paramName.c_str());

    // INSERT INTO parameter_dic
    //   (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)
    // VALUES
    //   (4321, 'ageSex', 'cdef', 2, 7890, 1, 0, 'ageSex_pcdef', 'ageSex_wcdef', 'fe24') 
    i_dbExec->update(
        "INSERT INTO parameter_dic" \
        " (parameter_hid, parameter_name, parameter_digest, parameter_rank, type_hid, is_extendable, num_cumulated, db_run_table, db_set_table, import_digest)" \
        " VALUES (" +
        to_string(io_row.paramHid) + ", " +
        toQuoted(io_row.paramName) + ", " +
        toQuoted(io_row.digest) + ", " +
        to_string(io_row.rank) + ", " +
        to_string(typeIt->second) + ", " +
        (io_row.isExtendable ? " 1" : " 0") + ", " +
        to_string(io_row.numCumulated) + ", " +
        toQuoted(io_row.dbRunTable) + ", " +
        toQuoted(io_row.dbSetTable) + ", " +
        toQuoted(io_row.importDigest) +
        ")");

    // INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (1234, 1, 4321, 0)
    i_dbExec->update(
        "INSERT INTO model_parameter_dic (model_id, model_parameter_id, parameter_hid, is_hidden) VALUES (" +
        to_string(io_row.modelId) + ", " +
        to_string(io_row.paramId) + ", " +
        to_string(io_row.paramHid) + ", " +
        (io_row.isHidden ? " 1" : " 0") +
        ")");
}

// insert row into model_parameter_import table.
void ModelInsertSql::insertParamImport(IDbExec * i_dbExec, ParamImportRow & io_row)
{
    // validate field values
    MessageEllipter me;
    if (io_row.paramId < 0) throw DbException(LT("invalid (negative) parameter id: %d"), io_row.paramId);

    if (io_row.fromName.empty()) throw DbException(LT("invalid (empty) source name for parameter import, id: %d"), io_row.paramId);
    if (io_row.fromName.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) source name: %s for parameter import, id: %d"), OM_NAME_DB_MAX, me.ellipt(io_row.fromName.c_str()), io_row.paramId);

    if (io_row.fromModel.empty()) throw DbException(LT("invalid (empty) source model name for parameter import, id: %d"), io_row.paramId);
    if (io_row.fromModel.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) source model name: %s for parameter import, id: %d"), OM_NAME_DB_MAX, me.ellipt(io_row.fromModel.c_str()), io_row.paramId);

    // INSERT INTO model_parameter_import
    //   (model_id, model_parameter_id, from_name, from_model_name, is_sample_dim)
    // VALUES
    //   (1234, 1, 1, 'ageSex', 0)
    i_dbExec->update(
        "INSERT INTO model_parameter_import (model_id, model_parameter_id, from_name, from_model_name, is_sample_dim)" \
        " VALUES (" +
        to_string(io_row.modelId) + ", " +
        to_string(io_row.paramId) + ", " +
        toQuoted(io_row.fromName) + ", " +
        toQuoted(io_row.fromModel) + ", " +
        (io_row.isSampleDim ? " 1" : " 0") +
        ")");
}

// insert row into parameter_dic_txt table.
void ModelInsertSql::insertParamText(
    IDbExec * i_dbExec, const ParamDicRow & i_paramRow, const map<string, int> & i_langMap, ParamDicTxtLangRow & io_row
)
{
    // validate field values
    MessageEllipter me;
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, parameter: %s"), i_paramRow.paramName.c_str());
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, parameter: %s"), i_paramRow.paramName.c_str());
    if (io_row.descr.length() > OM_DESCR_DB_MAX) throw DbException(LT("invalid (longer than %d) description: %s, parameter: %s"), OM_DESCR_DB_MAX, me.ellipt(io_row.descr), i_paramRow.paramName.c_str());
    if (io_row.note.length() > OM_STR_DB_MAX) throw DbException(LT("invalid (longer than %d) notes: %s, parameter: %s"), OM_STR_DB_MAX, me.ellipt(io_row.note), i_paramRow.paramName.c_str());

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s"), io_row.langCode.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) 
    // VALUES (4321, 10, 'Age by Sex', 'Age by Sex note')
    i_dbExec->update(
        "INSERT INTO parameter_dic_txt (parameter_hid, lang_id, descr, note) VALUES (" +
        to_string(i_paramRow.paramHid) + ", " +
        to_string(io_row.langId) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into parameter_dims table.
void ModelInsertSql::insertParamDims(
    IDbExec * i_dbExec, const ParamDicRow & i_paramRow,  const map<int, int> & i_typeIdMap, const ParamDimsRow & i_row
)
{
    // validate field values
    MessageEllipter me;
    if (i_row.paramId < 0) throw DbException(LT("invalid (negative) parameter id: %d"), i_row.paramId);

    if (i_row.name.empty()) throw DbException(LT("invalid (empty) parameter dimension name, parameter: %s"), i_paramRow.paramName.c_str());
    if (i_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) parameter dimension name: %s, parameter: %s"), OM_NAME_DB_MAX, me.ellipt(i_row.name), i_paramRow.paramName.c_str());
    
    if (i_row.dimId < 0) throw DbException(LT("invalid (negative) parameter dimension %s id: %d, parameter: %s"), i_row.name.c_str(), i_row.dimId, i_paramRow.paramName.c_str());
    if (i_row.typeId < 0) throw DbException(LT("invalid (negative) parameter dimension %s type id: %d, parameter: %s"), i_row.name.c_str(), i_row.typeId, i_paramRow.paramName.c_str());

    // find dimension type Hid by model type id
    map<int, int>::const_iterator typeIt = i_typeIdMap.find(i_row.typeId);
    if (typeIt == i_typeIdMap.cend()) throw DbException(LT("invalid parameter %s dimension %d type id: %d"), i_paramRow.paramName.c_str(), i_row.dimId, i_row.typeId);

    // INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (4321, 1, 'dim1', 7890)
    i_dbExec->update("INSERT INTO parameter_dims (parameter_hid, dim_id, dim_name, type_hid) VALUES (" +
        to_string(i_paramRow.paramHid) + ", " +
        to_string(i_row.dimId) + ", " +
        toQuoted(i_row.name) + ", " +
        to_string(typeIt->second) +
        ")");
}

// insert row into parameter_dims_txt table. 
void ModelInsertSql::insertParamDimsText(
    IDbExec * i_dbExec, const ParamDicRow & i_paramRow, const map<string, int> & i_langMap, ParamDimsTxtLangRow & io_row
)
{
    // validate field values
    MessageEllipter me;
    if (io_row.paramId < 0) throw DbException(LT("invalid (negative) parameter id: %d"), io_row.paramId);
    if (io_row.dimId < 0) throw DbException(LT("invalid (negative) parameter dimension id: %d"), io_row.dimId);
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code of parameter: %s, dimension id: %d"), i_paramRow.paramName.c_str(), io_row.dimId);
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, parameter: %s, dimension id: %d"), i_paramRow.paramName.c_str(), io_row.dimId);
    if (io_row.descr.length() > OM_DESCR_DB_MAX)
        throw DbException(LT("invalid (longer than %d) description of dimension %d parameter: %s, description: %s"), OM_DESCR_DB_MAX, io_row.dimId, i_paramRow.paramName.c_str(), me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX)
        throw DbException(LT("invalid (longer than %d) notes of dimension %d parameter: %s, notes: %s"), OM_STR_DB_MAX, io_row.dimId, i_paramRow.paramName.c_str(), me.ellipt(io_row.note));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s of parameter %s"), io_row.langCode.c_str(), i_paramRow.paramName.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) 
    // VALUES (4321, 1, 101, 'Age', 'Age note')
    i_dbExec->update(
        "INSERT INTO parameter_dims_txt (parameter_hid, dim_id, lang_id, descr, note) VALUES (" +
        to_string(i_paramRow.paramHid) + ", " +
        to_string(io_row.dimId) + ", " +
        to_string(io_row.langId) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into table_dic and model_table_dic tables.
void ModelInsertSql::insertTableDic(IDbExec * i_dbExec, TableDicRow & io_row)
{
    // validate field values
    MessageEllipter me;
    if (io_row.tableId < 0) throw DbException(LT("invalid (negative) output table id: %d"), io_row.tableId);

    if (io_row.tableName.empty()) throw DbException(LT("invalid (empty) output table name, id: %d"), io_row.tableId);
    if (io_row.tableName.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) output table name: %s"), OM_NAME_DB_MAX, me.ellipt(io_row.tableName));

    if (io_row.digest.empty() || io_row.digest.length() > OM_CODE_DB_MAX || io_row.importDigest.empty() || io_row.importDigest.length() > OM_CODE_DB_MAX)
        throw DbException(LT("invalid (empty or longer then %d) digest of output table: %s"), OM_CODE_DB_MAX, io_row.tableName.c_str());

    if (io_row.dbExprTable.empty() || io_row.dbExprTable.length() > 64 || 
        io_row.dbAccTable.empty() || io_row.dbAccTable.length() > 64 ||
        io_row.dbAccAll.empty() || io_row.dbAccAll.length() > 64)
        throw DbException(LT("invalid (empty or longer than 64) db table name(s) for output table: %s"), io_row.tableName.c_str());

    if (io_row.rank < 0) throw DbException(LT("invalid (negative) output table %s rank: %d"), io_row.tableName.c_str(), io_row.rank);
    if (io_row.exprPos < -1 || io_row.exprPos > io_row.rank - 1) throw DbException(LT("invalid output table %s analysis dimension position: %d"), io_row.tableName.c_str(), io_row.exprPos);

    // get new output table Hid
    i_dbExec->update(
        "UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'table_hid'"
    );
    io_row.tableHid = i_dbExec->selectToInt(
        "SELECT id_value FROM id_lst WHERE id_key = 'table_hid'",
        -1);
    if (io_row.tableHid <= 0) throw DbException(LT("invalid (not positive) output table Hid, table: %s"), io_row.tableName.c_str());

    // INSERT INTO table_dic
    //   (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view, import_digest)
    // VALUES 
    //   (9876, 'salarySex', '0fdc', 2, 0, 'salarySex_v0fdc', 'salarySex_a0fdc', 'salarySex_d0fdc', 'c44d')
    i_dbExec->update(
        "INSERT INTO table_dic" \
        " (table_hid, table_name, table_digest, table_rank, is_sparse, db_expr_table, db_acc_table, db_acc_all_view, import_digest)" \
        " VALUES (" +
        to_string(io_row.tableHid) + ", " +
        toQuoted(io_row.tableName) + ", " +
        toQuoted(io_row.digest) + ", " +
        to_string(io_row.rank) + ", " +
        (io_row.isSparse ? "1" : "0") + ", " +
        toQuoted(io_row.dbExprTable) + ", " +
        toQuoted(io_row.dbAccTable) + ", " +
        toQuoted(io_row.dbAccAll) + ", " +
        toQuoted(io_row.importDigest) +
        ")");

    // INSERT INTO model_table_dic 
    //   (model_id, model_table_id, table_hid, is_user, expr_dim_pos, is_hidden)
    // VALUES
    //   (1234, 8, 9876, 0, 1, 0)
    i_dbExec->update(
        "INSERT INTO model_table_dic (model_id, model_table_id, table_hid, is_user, expr_dim_pos, is_hidden) VALUES (" +
        to_string(io_row.modelId) + ", " +
        to_string(io_row.tableId) + ", " +
        to_string(io_row.tableHid) + ", " +
        (io_row.isUser ? "1" : "0") + ", " +
        to_string(io_row.exprPos) + ", " +
        (io_row.isHidden ? "1" : "0") +
        ")");
}

// insert row into table_dic_txt table. 
void ModelInsertSql::insertTableText(
    IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableDicTxtLangRow & io_row
)
{
    // validate field values
    MessageEllipter me;
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, output table: %s"), i_tableRow.tableName.c_str());
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, output table: %s"), i_tableRow.tableName.c_str());
    if (io_row.descr.length() > OM_DESCR_DB_MAX) throw DbException(LT("invalid (longer than %d) output table %s description: %s"), OM_DESCR_DB_MAX, i_tableRow.tableName.c_str(), me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX) throw DbException(LT("invalid (longer than %d) output table %s notes: %s"), OM_STR_DB_MAX, i_tableRow.tableName.c_str(), me.ellipt(io_row.descr));
    if (io_row.exprDescr.empty()) throw DbException(LT("invalid (empty) analysis dimension description, output table: %s"), i_tableRow.tableName.c_str());
    if (io_row.exprDescr.length() > OM_DESCR_DB_MAX) throw DbException(LT("invalid (longer than %d) output table %s analysis dimension description description: %s"), OM_DESCR_DB_MAX, i_tableRow.tableName.c_str(), me.ellipt(io_row.exprDescr));
    if (io_row.exprNote.length() > OM_STR_DB_MAX) throw DbException(LT("invalid (longer than %d) output table %s analysis dimension notes: %s"), OM_STR_DB_MAX, i_tableRow.tableName.c_str(), me.ellipt(io_row.exprNote));


    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s of output table: %s"), io_row.langCode.c_str(), i_tableRow.tableName.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO table_dic_txt 
    //   (table_hid, lang_id, descr, note, expr_descr, expr_note) 
    // VALUES
    //   (9876, 101, 'Salary by Sex', 'Salary by Sex note', '', '', 0)
    i_dbExec->update(
        "INSERT INTO table_dic_txt (table_hid, lang_id, descr, note, expr_descr, expr_note) VALUES (" +
        to_string(i_tableRow.tableHid) + ", " +
        to_string(langIt->second) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) + ", " +
        toQuoted(io_row.exprDescr) + ", " +
        (io_row.exprNote.empty() ? "NULL" : toQuoted(io_row.exprNote)) +
        ")");
}

// insert row into table_dims table. 
void ModelInsertSql::insertTableDims(
    IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<int, int> & i_typeIdMap, const TableDimsRow & i_row
)
{
    // validate field values
    if (i_row.tableId < 0) throw DbException(LT("invalid (negative) output table id: %d"), i_tableRow.tableId);

    if (i_row.name.empty()) throw DbException(LT("invalid (empty) output table dimension name"));
    if (i_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) output table dimension name: %s"), OM_NAME_DB_MAX, i_row.name.c_str());

    if (i_row.dimId < 0) throw DbException(LT("invalid (negative) output table dimension %s id: %d"), i_row.name.c_str(), i_row.dimId);
    if (i_row.typeId < 0) throw DbException(LT("invalid (negative) output table dimension %s type id: %d"), i_row.name.c_str(), i_row.typeId);
    if (i_row.dimSize <= 0) throw DbException(LT("invalid output table dimension %s size: %d"), i_row.name.c_str(), i_row.dimSize);

    // find dimension type Hid by model type id
    map<int, int>::const_iterator typeIt = i_typeIdMap.find(i_row.typeId);
    if (typeIt == i_typeIdMap.cend()) throw DbException(LT("invalid type id: %d of dimension %d output table %s"), i_row.typeId, i_row.dimId, i_tableRow.tableName.c_str());

    // INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (9876, 1, 'dim1', 6789, 0, 4)
    i_dbExec->update(
        "INSERT INTO table_dims (table_hid, dim_id, dim_name, type_hid, is_total, dim_size) VALUES (" +
        to_string(i_tableRow.tableHid) + ", " +
        to_string(i_row.dimId) + ", " +
        toQuoted(i_row.name) + ", " +
        to_string(typeIt->second) + ", " +
        (i_row.isTotal ? "1, " : "0, ") +
        to_string(i_row.dimSize) +
        ")");
}

// insert row into table_dims_txt table. 
void ModelInsertSql::insertTableDimsText(
    IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableDimsTxtLangRow & io_row
)
{
    // validate field values
    MessageEllipter me;
    if (io_row.dimId < 0) throw DbException(LT("invalid (negative) output table dimension id: %d"), io_row.dimId);
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, output table: %s, dimension id: %d"), i_tableRow.tableName.c_str(), io_row.dimId);
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, output table: %s, dimension id: %d"), i_tableRow.tableName.c_str(), io_row.dimId);
    if (io_row.descr.length() > OM_DESCR_DB_MAX)
        throw DbException(LT("invalid (longer than %d) description of dimension id: %d, output table: %s, description: %s"), OM_DESCR_DB_MAX, io_row.dimId, i_tableRow.tableName.c_str(), me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX)
        throw DbException(LT("invalid (longer than %d) notes for dimension id: %d, output table: %s, notes: %s"), OM_DESCR_DB_MAX, io_row.dimId, i_tableRow.tableName.c_str(), me.ellipt(io_row.note));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s"), io_row.langCode.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO table_dims_txt
    //   (table_hid, dim_id, lang_id, descr, note)
    // VALUES
    //   (9876, 1, 101, 'Salary', 'Salary note')
    i_dbExec->update(
        "INSERT INTO table_dims_txt (table_hid, dim_id, lang_id, descr, note) VALUES (" +
        to_string(i_tableRow.tableHid) + ", " +
        to_string(io_row.dimId) + ", " +
        to_string(langIt->second) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into table_acc table. 
void ModelInsertSql::insertTableAcc(IDbExec * i_dbExec, const TableDicRow & i_tableRow,  const TableAccRow & i_row)
{ 
    // validate field values
    MessageEllipter me;
    if (i_row.accId < 0) throw DbException(LT("invalid (negative) accumulator id: %d, output table: %s"), i_row.accId, i_tableRow.tableName.c_str());

    if (i_row.name.empty()) throw DbException(LT("invalid (empty) accumulator name, id: %d, output table: %s"), i_row.accId, i_tableRow.tableName.c_str());
    if (i_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) accumulator name: %s, id: %d, output table: %s"), OM_NAME_DB_MAX, i_row.name.c_str(), i_row.accId, i_tableRow.tableName.c_str());

    if (i_row.accSrc.empty()) throw DbException(LT("invalid (empty) accumulator expression, id: %d, output table: %s"), i_row.accId, i_tableRow.tableName.c_str());
    if (i_row.accSrc.length() > OM_NAME_DB_MAX)
        throw DbException(LT("invalid (longer than %d) accumulator expression, id: %d, output table: %s, expression: %s"), OM_NAME_DB_MAX, i_row.accId, i_tableRow.tableName.c_str(), me.ellipt(i_row.accSrc));

    if (i_row.accSql.empty()) throw DbException(LT("invalid (empty) accumulator sql, id: %d, output table: %s"), i_row.accId, i_tableRow.tableName.c_str());
    if (i_row.accSql.length() > OM_SQL_EXPR_DB_MAX) throw DbException(LT("invalid (longer than %d) accumulator sql, id: %d, output table: %s"), OM_SQL_EXPR_DB_MAX, i_row.accId, i_tableRow.tableName.c_str());

    // INSERT INTO table_acc (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) 
    // VALUES (9876, 1, 'acc1', 1, '2 * acc0', '2 * A.acc_value')
    i_dbExec->update(
        "INSERT INTO table_acc (table_hid, acc_id, acc_name, is_derived, acc_src, acc_sql) VALUES (" +
        to_string(i_tableRow.tableHid) + ", " +
        to_string(i_row.accId) + ", " +
        toQuoted(i_row.name) + ", " +
        (i_row.isDerived ? "1, " : "0, ") +
        toQuoted(i_row.accSrc) + ", " +
        toQuoted(i_row.accSql) + 
        ")");
}

// insert row into table_acc_txt table. 
void ModelInsertSql::insertTableAccText(
    IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableAccTxtLangRow & io_row
)
{ 
    // validate field values
    MessageEllipter me;
    if (io_row.accId < 0) throw DbException(LT("invalid (negative) accumulator id: %d, output table: %s"), io_row.accId, i_tableRow.tableName.c_str());
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, accumulator id: %d, output table: %s"), io_row.accId, i_tableRow.tableName.c_str());
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, accumulator id: %d, output table: %s"), io_row.accId, i_tableRow.tableName.c_str());
    if (io_row.descr.length() > OM_DESCR_DB_MAX)
        throw DbException(LT("invalid (longer than %d) description of accumulator id: %d, output table: %s, description:"), OM_DESCR_DB_MAX, io_row.accId, i_tableRow.tableName.c_str(), me.ellipt(io_row.descr));
    if (io_row.descr.length() > OM_STR_DB_MAX)
        throw DbException(LT("invalid (longer than %d) notes for accumulator id: %d, output table: %s, OM_STR_DB_MAX:"), OM_STR_DB_MAX, io_row.accId, i_tableRow.tableName.c_str(), me.ellipt(io_row.descr));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s"), io_row.langCode.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (9876, 101, 'Sum of salary by sex', '')
    i_dbExec->update(
        "INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (" +
        to_string(i_tableRow.tableHid) + ", " +
        to_string(io_row.accId) + ", " +
        to_string(langIt->second) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}
        
// insert row into table_expr table. 
void ModelInsertSql::insertTableExpr(IDbExec * i_dbExec, const TableDicRow & i_tableRow, const TableExprRow & i_row)
{
    // validate field values
    MessageEllipter me;
    if (i_row.exprId < 0) throw DbException(LT("invalid (negative) output expression id: %d, output table: %s"), i_row.exprId, i_tableRow.tableName.c_str());

    if (i_row.name.empty()) throw DbException(LT("invalid (empty) output expression name, output table: %s"), i_tableRow.tableName.c_str());
    if (i_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) output expression name: %s, output table: %s"), OM_NAME_DB_MAX, i_row.name.c_str(), i_tableRow.tableName.c_str());

    if (i_row.srcExpr.empty()) throw DbException(LT("invalid (empty) source expression, id: %d, output table: %s"), i_row.exprId, i_row.tableId);
    if (i_row.srcExpr.length() > OM_NAME_DB_MAX) 
        throw DbException(LT("invalid (longer than %d) source expression, id: %d, output table: %s, expression: %s"), OM_NAME_DB_MAX, i_row.exprId, i_tableRow.tableName.c_str(), me.ellipt(i_row.srcExpr));

    if (i_row.sqlExpr.empty()) throw DbException(LT("invalid (empty) output expression sql, id: %d, output table: %s"), i_row.exprId, i_tableRow.tableName.c_str());
    if (i_row.sqlExpr.length() > OM_SQL_EXPR_DB_MAX) throw DbException(LT("invalid (longer than %d) output expression sql, id: %d, output table: %s"), OM_SQL_EXPR_DB_MAX, i_row.exprId, i_tableRow.tableName.c_str());

    if (i_row.decimals < -1 || i_row.decimals > DBL_DIG)
        throw DbException(LT("invalid output expression decimals: %d, id: %d, output table: %s"), i_row.decimals, i_row.exprId, i_tableRow.tableName.c_str());

    // INSERT INTO table_expr (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql)
    // VALUES (
    //   9876,  1, 'expr1', 4, 
    //   'OM_SUM(acc1)',
    //   'SELECT M1.run_id, M1.dim0, M1.dim1, AVG(M1.acc_value) AS expr1 FROM salarySex_a98765432 M1 WHERE M1.acc_id = 0 GROUP BY M1.run_id, M1.dim0, M1.dim1'
    // )
    i_dbExec->update(
        "INSERT INTO table_expr (table_hid, expr_id, expr_name, expr_decimals, expr_src, expr_sql) VALUES (" +
        to_string(i_tableRow.tableHid) + ", " +
        to_string(i_row.exprId) + ", " +
        toQuoted(i_row.name) + ", " +
        to_string(i_row.decimals) + ", " +
        toQuoted(i_row.srcExpr) + ", " +
        toQuoted(i_row.sqlExpr) +
        ")");
}

// insert row into table_expr_txt table. 
void ModelInsertSql::insertTableExprText(
    IDbExec * i_dbExec, const TableDicRow & i_tableRow, const map<string, int> & i_langMap, TableExprTxtLangRow & io_row
)
{
    // validate field values
    MessageEllipter me;
    if (io_row.exprId < 0) throw DbException(LT("invalid (negative) output expression id: %d, output table: %s"), io_row.exprId, i_tableRow.tableName.c_str());
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, expression id: %d, output table: %s"), io_row.exprId, i_tableRow.tableName.c_str());
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, expression id: %d, output table: %s"), io_row.exprId, i_tableRow.tableName.c_str());
    if (io_row.descr.length() > OM_DESCR_DB_MAX) 
        throw DbException(LT("invalid (longer than %d) description of expression id: %d, output table: %s, description: %s"), OM_DESCR_DB_MAX, io_row.exprId, i_tableRow.tableName.c_str(), me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX)
        throw DbException(LT("invalid (longer than %d) notes for expression id: %d, output table: %s, notes: %s"), OM_STR_DB_MAX, io_row.exprId, i_tableRow.tableName.c_str(), me.ellipt(io_row.note));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s"), io_row.langCode.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO table_acc_txt (table_hid, acc_id, lang_id, descr, note) VALUES (9876, 101, 'Sum of salary by sex', '')
    i_dbExec->update(
        "INSERT INTO table_expr_txt (table_hid, expr_id, lang_id, descr, note) VALUES (" +
        to_string(i_tableRow.tableHid) + ", " +
        to_string(io_row.exprId) + ", " +
        to_string(langIt->second) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into entity_dic and model_entity_dic tables.
void ModelInsertSql::insertEntityDic(IDbExec * i_dbExec, EntityDicRow & io_row)
{
    // validate field values
    MessageEllipter me;
    if (io_row.entityId < 0) throw DbException(LT("invalid (negative) entity id: %d"), io_row.entityId);

    if (io_row.entityName.empty()) throw DbException(LT("invalid (empty) entity name, id: %d"), io_row.entityId);
    if (io_row.entityName.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) entity name: %s"), OM_NAME_DB_MAX, me.ellipt(io_row.entityName));

    if (io_row.digest.empty() || io_row.digest.length() > OM_CODE_DB_MAX)
        throw DbException(LT("invalid (empty or longer then %d) digest of entity: %s"), OM_CODE_DB_MAX, io_row.entityName.c_str());

    // get new entity Hid
    i_dbExec->update(
        "UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'entity_hid'"
    );
    io_row.entityHid = i_dbExec->selectToInt(
        "SELECT id_value FROM id_lst WHERE id_key = 'entity_hid'",
        -1);
    if (io_row.entityHid <= 0) throw DbException(LT("invalid (not positive) entity Hid of: %s"), io_row.entityName.c_str());

    // INSERT INTO entity_dic
    //   (entity_hid, entity_name, entity_digest)
    // VALUES 
    //   (4567, 'person', '0fde')
    i_dbExec->update(
        "INSERT INTO entity_dic" \
        " (entity_hid, entity_name, entity_digest) VALUES (" +
        to_string(io_row.entityHid) + ", " +
        toQuoted(io_row.entityName) + ", " +
        toQuoted(io_row.digest) +
        ")");

    // INSERT INTO model_entity_dic 
    //   (model_id, model_entity_id, entity_hid)
    // VALUES
    //   (1234, 8, 4567)
    i_dbExec->update(
        "INSERT INTO model_entity_dic (model_id, model_entity_id, entity_hid) VALUES (" +
        to_string(io_row.modelId) + ", " +
        to_string(io_row.entityId) + ", " +
        to_string(io_row.entityHid) +
        ")");
}

// insert row into entity_dic_txt table.
void ModelInsertSql::insertEntityText(
    IDbExec * i_dbExec, const EntityDicRow & i_entityRow, const map<string, int> & i_langMap, EntityDicTxtLangRow & io_row
)
{
    // validate field values
    MessageEllipter me;
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, entity: %s"), i_entityRow.entityName.c_str());
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, entity: %s"), i_entityRow.entityName.c_str());
    if (io_row.descr.length() > OM_DESCR_DB_MAX) throw DbException(LT("invalid (longer than %d) entity %s description: %s"), OM_DESCR_DB_MAX, i_entityRow.entityName.c_str(), me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX) throw DbException(LT("invalid (longer than %d) entity %s notes: %s"), OM_STR_DB_MAX, i_entityRow.entityName.c_str(), me.ellipt(io_row.descr));


    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s of entity: %s"), io_row.langCode.c_str(), i_entityRow.entityName.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO entity_dic_txt 
    //   (entity_hid, lang_id, descr, note) 
    // VALUES
    //   (4567, 101, 'Person entity', 'Person note')
    i_dbExec->update(
        "INSERT INTO entity_dic_txt (entity_hid, lang_id, descr, note) VALUES (" +
        to_string(i_entityRow.entityHid) + ", " +
        to_string(langIt->second) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into entity_attr table.
void ModelInsertSql::insertEntityAttr(
    IDbExec* i_dbExec, const EntityDicRow & i_entityRow, const map<int, int> & i_typeIdMap, const EntityAttrRow & i_row
)
{
    // validate field values
    if (i_row.entityId < 0) throw DbException(LT("invalid (negative) entity id: %d"), i_entityRow.entityId);

    if (i_row.name.empty()) throw DbException(LT("invalid (empty) entity attribute name"));
    if (i_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) entity attribute name: %s"), OM_NAME_DB_MAX, i_row.name.c_str());

    if (i_row.attrId < 0) throw DbException(LT("invalid (negative) entity attribute %s id: %d"), i_row.name.c_str(), i_row.attrId);
    if (i_row.typeId < 0) throw DbException(LT("invalid (negative) entity attribute %s type id: %d"), i_row.name.c_str(), i_row.typeId);

    // find attribute type Hid by model type id
    map<int, int>::const_iterator typeIt = i_typeIdMap.find(i_row.typeId);
    if (typeIt == i_typeIdMap.cend()) throw DbException(LT("invalid type id: %d of attribute %d entity %s"), i_row.typeId, i_row.attrId, i_entityRow.entityName.c_str());

    // INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (4567, 1, 'attr1', 6789, 0)
    i_dbExec->update(
        "INSERT INTO entity_attr (entity_hid, attr_id, attr_name, type_hid, is_internal) VALUES (" +
        to_string(i_entityRow.entityHid) + ", " +
        to_string(i_row.attrId) + ", " +
        toQuoted(i_row.name) + ", " +
        to_string(typeIt->second) + ", " +
        (i_row.isInternal ? "1" : "0") +
        ")");
}

// insert row into entity_attr_txt table.
void ModelInsertSql::insertEntityAttrText(
    IDbExec* i_dbExec, const EntityDicRow & i_entityRow, const map<string, int> & i_langMap, EntityAttrTxtLangRow & io_row
)
{
    // validate field values
    MessageEllipter me;
    if (io_row.attrId < 0) throw DbException(LT("invalid (negative) entity attribute id: %d"), io_row.attrId);
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, entity: %s, attribute id: %d"), i_entityRow.entityName.c_str(), io_row.attrId);
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, entity: %s, attribute id: %d"), i_entityRow.entityName.c_str(), io_row.attrId);
    if (io_row.descr.length() > OM_DESCR_DB_MAX)
        throw DbException(LT("invalid (longer than %d) description of attribute id: %d, entity: %s, description: %s"), OM_DESCR_DB_MAX, io_row.attrId, i_entityRow.entityName.c_str(), me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX)
        throw DbException(LT("invalid (longer than %d) notes for attribute id: %d, entity: %s, notes: %s"), OM_DESCR_DB_MAX, io_row.attrId, i_entityRow.entityName.c_str(), me.ellipt(io_row.note));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s"), io_row.langCode.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO
    //   entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) 
    // VALUES
    //   (4567, 1, 101, 'Person age', 'Person age note')
    i_dbExec->update(
        "INSERT INTO entity_attr_txt (entity_hid, attr_id, lang_id, descr, note) VALUES (" +
        to_string(i_entityRow.entityHid) + ", " +
        to_string(io_row.attrId) + ", " +
        to_string(langIt->second) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into group_lst table. 
void ModelInsertSql::insertGroupLst(IDbExec * i_dbExec, const GroupLstRow & i_row)
{
    // validate field values
    MessageEllipter me;
    if (i_row.groupId < 0) throw DbException(LT("invalid (negative) group id: %d"), i_row.groupId);

    if (i_row.name.empty()) throw DbException(LT("invalid (empty) group name, id: %d"), i_row.groupId);
    if (i_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) group name: %s"), OM_NAME_DB_MAX, me.ellipt(i_row.name));

    // INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden) VALUES (1234, 0, 0, 'TableGroup', 0, 0)
    i_dbExec->update(
        "INSERT INTO group_lst (model_id, group_id, is_parameter, group_name, is_hidden) VALUES (" +
        to_string(i_row.modelId) + ", " +
        to_string(i_row.groupId) + ", " +
        (i_row.isParam ? "1, " : "0, ") +
        toQuoted(i_row.name) + ", " +
        (i_row.isHidden ? "1" : "0") +
        ")");
}

// insert row into group_txt table.
void ModelInsertSql::insertGroupText(IDbExec * i_dbExec, const map<string, int> & i_langMap, GroupTxtLangRow & io_row)
{
    // validate field values
    MessageEllipter me;
    if (io_row.groupId < 0) throw DbException(LT("invalid (negative) group id: %d"), io_row.groupId);
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, group id: %d"), io_row.groupId);
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, group id: %d"), io_row.groupId);
    if (io_row.descr.length() > OM_DESCR_DB_MAX) throw DbException(LT("invalid (longer than %d) group %d description: %s"), OM_DESCR_DB_MAX, io_row.groupId, me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX) throw DbException(LT("invalid (longer than %d) group %d notes: %s"), OM_STR_DB_MAX, io_row.groupId, me.ellipt(io_row.note));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s"), io_row.langCode.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) 
    // VALUES (1234, 0, 101, 'Table group', 'Table group notes')
    i_dbExec->update(
        "INSERT INTO group_txt (model_id, group_id, lang_id, descr, note) VALUES (" +
        to_string(io_row.modelId) + ", " +
        to_string(io_row.groupId) + ", " +
        to_string(langIt->second) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into group_pc table. 
// negative value of i_row.childGroupId or i_row.leafId treated as db-NULL
void ModelInsertSql::insertGroupPc(IDbExec * i_dbExec, const GroupPcRow & i_row)
{
    // validate field values
    if (i_row.groupId < 0) throw DbException(LT("invalid (negative) group id: %d"), i_row.groupId);

    // INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (1234, 0, 1, NULL, 22)
    i_dbExec->update(
        "INSERT INTO group_pc (model_id, group_id, child_pos, child_group_id, leaf_id) VALUES (" +
        to_string(i_row.modelId) + ", " +
        to_string(i_row.groupId) + ", " +
        to_string(i_row.childPos) + ", " +
        (i_row.childGroupId < 0 ? "NULL" : to_string(i_row.childGroupId)) + ", " +
        (i_row.leafId < 0 ? "NULL" : to_string(i_row.leafId)) +
        ")");
}

// insert row into entity_group_lst table.
void ModelInsertSql::insertEntityGroupLst(IDbExec * i_dbExec, const EntityGroupLstRow & i_row)
{
    // validate field values
    MessageEllipter me;
    if (i_row.entityId < 0) throw DbException(LT("invalid (negative) entity id: %d"), i_row.entityId);
    if (i_row.groupId < 0) throw DbException(LT("invalid (negative) group id: %d"), i_row.groupId);

    if (i_row.name.empty()) throw DbException(LT("invalid (empty) group name, entity id: %d, group id: %d"), i_row.entityId, i_row.groupId);
    if (i_row.name.length() > OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) group name: %s"), OM_NAME_DB_MAX, me.ellipt(i_row.name));

    // INSERT INTO entity_group_lst (model_id, model_entity_id, group_id, group_name, is_hidden) VALUES (1234, 0, 0, 'AttributeGroup', 0)
    i_dbExec->update(
        "INSERT INTO entity_group_lst (model_id, model_entity_id, group_id, group_name, is_hidden) VALUES (" +
        to_string(i_row.modelId) + ", " +
        to_string(i_row.entityId) + ", " +
        to_string(i_row.groupId) + ", " +
        toQuoted(i_row.name) + ", " +
        (i_row.isHidden ? "1" : "0") +
        ")");
}

// insert row into entity_group_txt table.
void ModelInsertSql::insertEntityGroupText(IDbExec * i_dbExec, const map<string, int> & i_langMap, EntityGroupTxtLangRow & io_row)
{
    // validate field values
    MessageEllipter me;
    if (io_row.entityId < 0) throw DbException(LT("invalid (negative) entity id: %d"), io_row.entityId);
    if (io_row.groupId < 0) throw DbException(LT("invalid (negative) group id: %d"), io_row.groupId);
    if (io_row.langCode.empty()) throw DbException(LT("invalid (empty) language code, entity id: %d, group id: %d"), io_row.entityId, io_row.groupId);
    if (io_row.descr.empty()) throw DbException(LT("invalid (empty) description, entity id: %d, group id: %d"), io_row.entityId, io_row.groupId);
    if (io_row.descr.length() > OM_DESCR_DB_MAX) throw DbException(LT("invalid (longer than %d) entity id: %d, group %d description: %s"), OM_DESCR_DB_MAX, io_row.entityId, io_row.groupId, me.ellipt(io_row.descr));
    if (io_row.note.length() > OM_STR_DB_MAX) throw DbException(LT("invalid (longer than %d) entity id: %d, group %d notes: %s"), OM_STR_DB_MAX, io_row.entityId, io_row.groupId, me.ellipt(io_row.note));

    // update language id with actual database value
    map<string, int>::const_iterator langIt = i_langMap.find(io_row.langCode);
    if (langIt == i_langMap.cend()) throw DbException(LT("invalid language code: %s"), io_row.langCode.c_str());
    io_row.langId = langIt->second;

    // INSERT INTO entity_group_txt (model_id, model_entity_id, group_id, lang_id, descr, note)
    // VALUES (1234, 0, 101, 1, 'Table group', 'Table group notes')
    i_dbExec->update(
        "INSERT INTO entity_group_txt (model_id, model_entity_id, group_id, lang_id, descr, note) VALUES (" +
        to_string(io_row.modelId) + ", " +
        to_string(io_row.entityId) + ", " +
        to_string(io_row.groupId) + ", " +
        to_string(langIt->second) + ", " +
        toQuoted(io_row.descr) + ", " +
        (io_row.note.empty() ? "NULL" : toQuoted(io_row.note)) +
        ")");
}

// insert row into entity_group_pc table.
// negative value of i_row.childGroupId or i_row.attrId treated as db-NULL
void ModelInsertSql::insertEntityGroupPc(IDbExec * i_dbExec, const EntityGroupPcRow & i_row)
{
    // validate field values
    if (i_row.entityId < 0) throw DbException(LT("invalid (negative) entity id: %d"), i_row.entityId);
    if (i_row.groupId < 0) throw DbException(LT("invalid (negative) group id: %d"), i_row.groupId);

    // INSERT INTO entity_group_pc (model_id, model_entity_id, group_id, child_pos, child_group_id, attr_id) VALUES (1234, 0, 1, 2, NULL, 22)
    i_dbExec->update(
        "INSERT INTO entity_group_pc (model_id, model_entity_id, group_id, child_pos, child_group_id, attr_id) VALUES (" +
        to_string(i_row.modelId) + ", " +
        to_string(i_row.entityId) + ", " +
        to_string(i_row.groupId) + ", " +
        to_string(i_row.childPos) + ", " +
        (i_row.childGroupId < 0 ? "NULL" : to_string(i_row.childGroupId)) + ", " +
        (i_row.attrId < 0 ? "NULL" : to_string(i_row.attrId)) +
        ")");
}

// if default model profile not empty then insert rows into profile_lst and profile_option tables
void ModelInsertSql::insertModelProfile(IDbExec * i_dbExec, const string & i_profileName, const NoCaseMap & i_profileRows)
{
    if (i_profileName.empty()) return;  // model profile empty

    // validate and insert profile_lst master row
    string pName = toQuoted(i_profileName);
    if (pName.length() > 2 + OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) model profile name: %s"), OM_NAME_DB_MAX, i_profileName.c_str());

    i_dbExec->update("INSERT INTO profile_lst (profile_name) VALUES (" + pName + ")");

    for (NoCaseMap::const_iterator it = i_profileRows.cbegin(); it != i_profileRows.cend(); it++) {

        if (it->first.empty()) continue;    // skip empty option

        string pKey = toQuoted(it->first);
        if (pKey.length() > 2 + OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) model profile option name: %s"), OM_NAME_DB_MAX, it->first.c_str());

        string pVal = toQuoted(it->second);
        if (pVal.length() > 2 + OM_NAME_DB_MAX) throw DbException(LT("invalid (longer than %d) model profile value: %s"), OM_NAME_DB_MAX, it->second.c_str());

        // INSERT INTO profile_option (profile_name, option_key, option_value)
        // VALUES ('modelOne_v1.1.3', 'Memory.ScaleParameter', 'PopulationSize')
        i_dbExec->update(
            "INSERT INTO profile_option (profile_name, option_key, option_value) VALUES (" +
            pName + ", " +
            pKey + ", " +
            pVal +
            ")");
    }
}

// create new workset: insert metadata and delete existing workset parameters, if required
void ModelInsertSql::createWorksetMeta(
    IDbExec * i_dbExec, const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet
)
{
    // validate field values
    MessageEllipter me;
    if (io_metaSet.worksetRow.updateDateTime.length() > OM_CODE_DB_MAX) throw DbException(LT("invalid (longer than %d) update time of workset: %s"), OM_CODE_DB_MAX, me.ellipt(io_metaSet.worksetRow.name));

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
    i_dbExec->update(
        "UPDATE id_lst SET id_value =" \
        " CASE" \
        " WHEN 0 =" \
        " (" \
        " SELECT COUNT(*) FROM workset_lst EW" \
        " INNER JOIN model_dic EM ON (EM.model_id = EW.model_id)" \
        " WHERE EW.set_name = " + toQuoted(io_metaSet.worksetRow.name) +
        " AND EM.model_digest = " + toQuoted(i_metaRows.modelDic.digest) +
        " )" \
        " THEN id_value + 1" \
        " ELSE id_value" \
        " END" \
        " WHERE id_key = 'run_id_set_id'");

    // insert into workset_lst table
    // if i_row.baseRunId <= 0 then it treated as db-NULL
    //
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
    i_dbExec->update(
        "INSERT INTO workset_lst" \
        " (set_id, base_run_id, model_id, set_name, is_readonly, update_dt)" \
        " SELECT IL.id_value, " +
        (io_metaSet.worksetRow.baseRunId <= 0 ? "NULL" : to_string(io_metaSet.worksetRow.baseRunId)) + "," +
        " (SELECT M.model_id FROM model_dic M WHERE M.model_digest = " + toQuoted(i_metaRows.modelDic.digest) + "), " +
        toQuoted(io_metaSet.worksetRow.name) + ", " +
        (io_metaSet.worksetRow.isReadonly ? "1, " : "0, ") +
        toQuoted(io_metaSet.worksetRow.updateDateTime) +
        " FROM id_lst IL" \
        " WHERE IL.id_key = 'run_id_set_id'" \
        " AND NOT EXISTS" \
        " (" \
        " SELECT * FROM workset_lst EW" \
        " INNER JOIN model_dic EM ON (EM.model_id = EW.model_id)" \
        " WHERE EW.set_name = " + toQuoted(io_metaSet.worksetRow.name) +
        " AND EM.model_digest = " + toQuoted(i_metaRows.modelDic.digest) +
        ")");

    // select workset id: must be positive
    io_metaSet.worksetRow.setId = i_dbExec->selectToInt(
        "SELECT set_id FROM workset_lst W" \
        " INNER JOIN model_dic M ON (M.model_id = W.model_id)" \
        " WHERE W.set_name = " + toQuoted(io_metaSet.worksetRow.name) +
        " AND M.model_digest = " + toQuoted(i_metaRows.modelDic.digest), 
        -1);
    if (io_metaSet.worksetRow.setId <= 0) throw DbException(LT("invalid (not positive) set id for workset: %s"), io_metaSet.worksetRow.name.c_str());

    string setIdStr = to_string(io_metaSet.worksetRow.setId);    // save current workset id

    // delete existing workset parameters
    for (const ParamDicRow & row : i_metaRows.paramDic) {
        i_dbExec->update("DELETE FROM " + row.dbSetTable + " WHERE set_id = " + setIdStr);
    }

    // delete workset metadata except of workset_lst master row
    i_dbExec->update("DELETE FROM workset_parameter_txt WHERE set_id = " + setIdStr);
    i_dbExec->update("DELETE FROM workset_parameter WHERE set_id = " + setIdStr);
    i_dbExec->update("DELETE FROM workset_txt WHERE set_id = " + setIdStr);

    // insert workset text (description and notes)
    for (WorksetTxtLangRow & row : io_metaSet.worksetTxt) {

        // validate field values
        if (row.langCode.empty()) throw DbException(LT("invalid (empty) language code for workset description and notes"));
        if (row.descr.empty()) throw DbException(LT("invalid (empty) workset description"));
        if (row.descr.length() > OM_DESCR_DB_MAX) throw DbException(LT("invalid (longer than %d) workset %s description: %s"), OM_DESCR_DB_MAX, io_metaSet.worksetRow.name.c_str(), me.ellipt(row.descr));
        if (row.note.length() > OM_STR_DB_MAX) throw DbException(LT("invalid (longer than %d) workset %s notes: %s"), OM_STR_DB_MAX, io_metaSet.worksetRow.name.c_str(), me.ellipt(row.note));

        // INSERT INTO workset_txt (set_id, lang_id, descr, note) 
        // SELECT 
        //   W.set_id, 
        //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
        //   'default workset',
        //   'default workset notes'
        // FROM workset_lst W WHERE W.set_id = 1234;
        i_dbExec->update(
            "INSERT INTO workset_txt (set_id, lang_id, descr, note)" \
            " SELECT" \
            " W.set_id," \
            " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(row.langCode) + "), " +
            toQuoted(row.descr) + ", " +
            (row.note.empty() ? "NULL" : toQuoted(row.note)) +
            " FROM workset_lst W WHERE W.set_id = " + setIdStr);
    }

    // insert workset parameters list
    for (const WorksetParamRow & row : io_metaSet.worksetParam) {

        // validate field values
        if (row.paramId < 0) throw DbException(LT("invalid (negative) workset parameter id: %d"), row.paramId);
        if (row.subCount < 1) throw DbException(LT("invalid sub-values count %d for workset parameter id: %d"), row.subCount, row.paramId);

        // INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id)
        // SELECT 
        //   W.set_id, P.parameter_hid, 1, 0
        // FROM workset_lst W
        // INNER JOIN model_parameter_dic P ON (P.model_id = W.model_id AND P.model_parameter_id = 1)
        // WHERE W.set_id = 1234;
        i_dbExec->update(
            "INSERT INTO workset_parameter (set_id, parameter_hid, sub_count, default_sub_id)" \
            " SELECT" \
            " W.set_id, P.parameter_hid, " + to_string(row.subCount) + ", " + to_string(row.defaultSubId) +
            " FROM workset_lst W" \
            " INNER JOIN model_parameter_dic P ON (P.model_id = W.model_id AND P.model_parameter_id = " + to_string(row.paramId) + ")"
            " WHERE W.set_id = " + setIdStr);
    }

    // insert workset parameters text: parameter value notes
    for (WorksetParamTxtLangRow & row : io_metaSet.worksetParamTxt) {

        // validate field values
        if (row.paramId < 0) throw DbException(LT("invalid (negative) workset parameter id: %d"), row.paramId);
        if (row.langCode.empty()) throw DbException(LT("invalid (empty) language code, workset parameter id: %d"), row.paramId);
        if (row.note.length() > OM_STR_DB_MAX) throw DbException(LT("invalid (longer than %d) notes, workset %s parameter id: %d, notes: %s"), OM_STR_DB_MAX, io_metaSet.worksetRow.name.c_str(), row.paramId, me.ellipt(row.note));

        // INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note)
        // SELECT 
        //   W.set_id, P.parameter_hid,
        //   (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = 'EN'),
        //   'parameter value notes'
        // FROM workset_lst W
        // INNER JOIN model_parameter_dic P ON (P.model_id = W.model_id AND P.model_parameter_id = 1)
        // WHERE W.set_id = 1234;
        i_dbExec->update(
            "INSERT INTO workset_parameter_txt (set_id, parameter_hid, lang_id, note)" \
            " SELECT" \
            " W.set_id, P.parameter_hid, " \
            " (SELECT LL.lang_id FROM lang_lst LL WHERE LL.lang_code = " + toQuoted(row.langCode) + "), " +
            (row.note.empty() ? "NULL" : toQuoted(row.note)) +
            " FROM workset_lst W" \
            " INNER JOIN model_parameter_dic P ON (P.model_id = W.model_id AND P.model_parameter_id = " + to_string(row.paramId) + ")"
            " WHERE W.set_id = " + setIdStr);
    }
}


