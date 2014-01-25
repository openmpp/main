// OpenM++ data library: class to produce new model sql from metadata
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "modelSqlBuilder.h"

using namespace openm;

// create new model builder
IModelBuilder * IModelBuilder::create(void)
{
    return new ModelSqlBuilder();
}

// release builder resources
IModelBuilder::~IModelBuilder() throw() { }

// create new model builder
ModelSqlBuilder::ModelSqlBuilder(void)
{
    // convert log timestamp to model timestamp: _20120817_160459_0148 => _201208171604590148_
    modelTs = theLog->timeStampSuffix();
    size_t len = modelTs.length();
    if (len < 16) throw DbException("invalid (too short) log timestamp");

    modelTs = "_" + modelTs.substr(1, 8) + modelTs.substr(10, 6) + ((len >= 18) ? modelTs.substr(17) : "") + "_";
}

// validate metadata and return sql script to create new model from supplied metadata rows
const vector<string> ModelSqlBuilder::build(MetaModelHolder & i_metaRows)
{
    try {
        // validate input rows: uniqueness and referential integrity
        validate(i_metaRows);

        // set model_dic row field values: model prefix and db table prefixes
        setModelDicRow(i_metaRows.modelDic);

        // collect info for db parameter tables, db subsample tables and value views
        setParamTableInfo(i_metaRows);
        setOutTableInfo(i_metaRows);

        // return sql script to create new model from supplied metadata rows
        return buildCreateModel(i_metaRows);
    }
    catch(HelperException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (DbException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw DbException(ex.what());
    }
}

// return sql script to create new model from supplied metadata rows
const vector<string> ModelSqlBuilder::buildCreateModel(MetaModelHolder & i_metaRows) const
{
    // start transaction and get new model id
    vector<string> lineVec;

    lineVec.push_back("--");
    lineVec.push_back("-- create new model: " + i_metaRows.modelDic.name);
    lineVec.push_back("-- model timestamp:  " + modelTs);
    lineVec.push_back("-- db names prefix:  " + i_metaRows.modelDic.modelPrefix);
    lineVec.push_back("-- script created:   " + toDateTimeString(theLog->timeStampSuffix()));
    lineVec.push_back("--");

    lineVec.push_back("BEGIN TRANSACTION;");
    lineVec.push_back("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'model_id';");
    lineVec.push_back("");

    // model header
    lineVec.push_back("--");
    lineVec.push_back("-- model description");
    lineVec.push_back("--");

    lineVec.push_back(ModelInsertSql::insertSql(i_metaRows.modelDic) + ";");
    lineVec.push_back("");

    for (const ModelDicTxtLangRow & row : i_metaRows.modelTxt) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    // model types
    lineVec.push_back("--");
    lineVec.push_back("-- model types");
    lineVec.push_back("--");

    for (const TypeDicRow & row : i_metaRows.typeDic) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const TypeDicTxtLangRow & row : i_metaRows.typeTxt) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const TypeEnumLstRow & row : i_metaRows.typeEnum) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const TypeEnumTxtLangRow & row : i_metaRows.typeEnumTxt) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    // model input parameters
    lineVec.push_back("--");
    lineVec.push_back("-- model input parameters");
    lineVec.push_back("--");

    for (const ParamDicRow & row : i_metaRows.paramDic) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const ParamDicTxtLangRow & row : i_metaRows.paramTxt) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const ParamDimsRow & row : i_metaRows.paramDims) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    // model output tables
    lineVec.push_back("--");
    lineVec.push_back("-- model output tables");
    lineVec.push_back("--");

    for (const TableDicRow & row : i_metaRows.tableDic) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const TableDicTxtLangRow & row : i_metaRows.tableTxt) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const TableDimsRow & row : i_metaRows.tableDims) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const TableDimsTxtLangRow & row : i_metaRows.tableDimsTxt) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const TableAccRow & row : i_metaRows.tableAcc) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const TableAccTxtLangRow & row : i_metaRows.tableAccTxt) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const TableUnitRow & row : i_metaRows.tableUnit) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    for (const TableUnitTxtLangRow & row : i_metaRows.tableUnitTxt) {
        lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
    }
    lineVec.push_back("");

    // group of parameters or output tables
    if (i_metaRows.groupLst.size() > 0) {
        lineVec.push_back("--");
        lineVec.push_back("-- group of parameters or output tables");
        lineVec.push_back("--");

        for (const GroupLstRow & row : i_metaRows.groupLst) {
            lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
        }
        lineVec.push_back("");

        for (const GroupTxtLangRow & row : i_metaRows.groupTxt) {
            lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
        }
        lineVec.push_back("");

        for (const GroupPcRow & row : i_metaRows.groupPc) {
            lineVec.push_back(ModelInsertSql::insertSql(row) + ";");
        }
        lineVec.push_back("");
    }

    // create tables for model input parameters
    lineVec.push_back("--");
    lineVec.push_back("-- model input parameters");
    lineVec.push_back("--");

    for (const ParamTblInfo & tblInfo : paramInfoVec) {
        lineVec.push_back("CREATE TABLE " + tblInfo.paramTableName + " " + paramCreateTableBody("run_id", tblInfo) + ";");
        lineVec.push_back("CREATE TABLE " + tblInfo.setTableName + " " + paramCreateTableBody("set_id", tblInfo) + ";");
    }
    lineVec.push_back("");

    // create tables for model output tables
    lineVec.push_back("--");
    lineVec.push_back("-- model output tables");
    lineVec.push_back("--");

    for (const OutTblInfo & tblInfo : outInfoVec) {

        string body = subCreateTableBody(tblInfo);
        lineVec.push_back("CREATE TABLE " + tblInfo.subTableName + " " + body + ";");

        body = valueCreateTableBody(tblInfo);
        lineVec.push_back("CREATE TABLE " + tblInfo.valueTableName + " " + body + ";");
    }
    lineVec.push_back("");

    lineVec.push_back("COMMIT;");   // done
    return lineVec;
}

// return sql script to create backward compatibility views (Modgen compatibility)
const vector<string> ModelSqlBuilder::buildCompatibilityViews(const MetaModelHolder & i_metaRows) const
{
    try {
        // put descriptive header
        vector<string> lineVec;

        lineVec.push_back("--");
        lineVec.push_back("-- compatibility views for model: " + i_metaRows.modelDic.name);
        lineVec.push_back("-- model timestamp:  " + modelTs);
        lineVec.push_back("-- db names prefix:  " + i_metaRows.modelDic.modelPrefix);
        lineVec.push_back("-- script created:   " + toDateTimeString(theLog->timeStampSuffix()));
        lineVec.push_back("--");
        lineVec.push_back("-- Dear user:");
        lineVec.push_back("--   this part of database is optional and NOT used by openM++");
        lineVec.push_back("--   if you want it for any reason please enjoy else just ignore it");
        lineVec.push_back("-- Or other words:");
        lineVec.push_back("--   if you don't know what is this then you don't need it");
        lineVec.push_back("--");
        lineVec.push_back("");

        // input parameters compatibility views
        lineVec.push_back("--");
        lineVec.push_back("-- input parameters compatibility views");
        lineVec.push_back("--");

        for (const ParamTblInfo & tblInfo : paramInfoVec) {
            string body = paramCompatibilityViewBody(i_metaRows, tblInfo);
            lineVec.push_back("CREATE VIEW " + tblInfo.name + " AS " + body + ";");
        }
        lineVec.push_back("");
        
        // output tables compatibility views
        lineVec.push_back("--");
        lineVec.push_back("-- output tables compatibility views");
        lineVec.push_back("--");

        for (const OutTblInfo & tblInfo : outInfoVec) {
            string body = outputCompatibilityViewBody(i_metaRows, tblInfo);
            lineVec.push_back("CREATE VIEW " + tblInfo.name + " AS " + body + ";");
        }
        lineVec.push_back("");

        return lineVec;     // done
    }
    catch(HelperException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (DbException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw DbException(ex.what());
    }
}

// return sql script to insert parameters if template file exists
const string ModelSqlBuilder::buildInsertParameters(const MetaModelHolder & i_metaRows, const string & i_sqlTemplateFilePath) const
{
    // check if template for sql default parametrs is exists
    ifstream ist;
    exit_guard<ifstream> onExit(&ist, &ifstream::close);  // close on exit

    ist.open(i_sqlTemplateFilePath, ios_base::in | ios_base::binary);
    if (ist.fail()) return "";                                                  // file not exists

    // read until the end
    string sql((istreambuf_iterator<char>(ist)), istreambuf_iterator<char>());
    if (ist.fail()) return "";                                                  // read failed

    // replace script parameters
    sql = replaceAll(sql, "${OM_MODEL_TIMESTAMP}", modelTs.c_str());
    sql = replaceAll(sql, "${OM_MODEL_PREFIX}", i_metaRows.modelDic.modelPrefix.c_str());
    sql = replaceAll(sql, "${OMC_COMPILE_TIME}", toDateTimeString(theLog->timeStampSuffix()).c_str());

    return sql;
}

// return body of create table sql for parameter:
// (
//  run_id INT   NOT NULL,
//  dim0   INT   NOT NULL,
//  dim1   INT   NOT NULL,
//  value  FLOAT NOT NULL,
//  PRIMARY KEY (run_id, dim0, dim1)
// )
const string ModelSqlBuilder::paramCreateTableBody(const string & i_runSetId, const ParamTblInfo & i_tblInfo) const
{
    string sql = "(" + i_runSetId + " INT NOT NULL, ";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sql += dimName + " INT NOT NULL, ";
    }

    sql += 
        "value " + i_tblInfo.valueTypeName + " NOT NULL, " \
        "PRIMARY KEY (" + i_runSetId;

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sql += ", " + dimName;
    }
    sql += "))";

    return sql;
}

// return body of create table sql for subsample table:
// (
//  run_id    INT   NOT NULL,
//  dim0      INT   NOT NULL,
//  dim1      INT   NOT NULL,
//  sub_id    INT   NOT NULL,
//  acc0      FLOAT NULL,
//  acc1      FLOAT NULL,
//  PRIMARY KEY (run_id, dim0, dim1, sub_id)
// )
const string ModelSqlBuilder::subCreateTableBody(const OutTblInfo & i_tblInfo) const
{
    string sql = "(run_id INT NOT NULL, ";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sql += dimName + " INT NOT NULL, ";
    }

    sql += "sub_id INT NOT NULL, ";

    for (const string & accName : i_tblInfo.accNameVec) {
        sql += accName + " FLOAT NULL, ";
    }

    sql += "PRIMARY KEY (run_id";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sql += ", " + dimName;
    }
    sql += ", sub_id))";

    return sql;
}

// return body of create table sql for value table:
// (
//  run_id    INT   NOT NULL,
//  dim0      INT   NOT NULL,
//  dim1      INT   NOT NULL,
//  unit_id   INT   NOT NULL,
//  value     FLOAT NULL,
//  PRIMARY KEY (run_id, dim0, dim1, unit_id)
// )
const string ModelSqlBuilder::valueCreateTableBody(const OutTblInfo & i_tblInfo) const
{
    string sql = "(run_id INT NOT NULL, ";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sql += dimName + " INT NOT NULL, ";
    }

    sql += "unit_id INT NOT NULL, value FLOAT NULL, PRIMARY KEY (run_id";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sql += ", " + dimName;
    }
    sql += ", unit_id))";

    return sql;
}

// return body of create view sql for parameter compatibility view:
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
// )
const string ModelSqlBuilder::paramCompatibilityViewBody(const MetaModelHolder & i_metaRows, const ParamTblInfo & i_tblInfo) const
{
    string sql = "SELECT";

    for (size_t k = 0; k < i_tblInfo.dimNameVec.size(); k++) {
        sql += " S." + i_tblInfo.dimNameVec[k] + " AS \"Dim" + to_string(k) + "\",";
    }
    sql += " S.value AS \"Value\"";

    // from subsample table where run id is first run of that model
    sql += " FROM " + i_tblInfo.paramTableName + " S" +
        " WHERE S.run_id = (" +
        " SELECT MIN(RL.run_id)" +
        " FROM run_lst RL" +
        " INNER JOIN model_dic MD ON (MD.model_id = RL.model_id)" +
        " WHERE MD.model_name = " + toQuoted(i_metaRows.modelDic.name) + 
        " AND MD.model_ts = " + toQuoted(i_metaRows.modelDic.timestamp) +
        ")";
    return sql;
}

// return body of create view sql for output table compatibility view:
// SELECT
//   S.dim0    AS "Dim0",
//   S.dim1    AS "Dim1",
//   S.unit_id AS "Dim2",
//   S.value   AS "Value"
// FROM modelone_201208171604590148_v0_salarySex S
// WHERE S.run_id =
// (
//  SELECT MIN(RL.run_id)
//  FROM run_lst RL
//  INNER JOIN model_dic MD ON (MD.model_id = RL.model_id)
//  WHERE MD.model_name = 'modelOne' AND MD.model_ts = '_201208171604590148_'
// )
const string ModelSqlBuilder::outputCompatibilityViewBody(const MetaModelHolder & i_metaRows, const OutTblInfo & i_tblInfo) const
{
    string sql = "SELECT";

    for (size_t k = 0; k < i_tblInfo.dimNameVec.size(); k++) {
        sql += " S." + i_tblInfo.dimNameVec[k] + " AS \"Dim" + to_string(k) + "\",";
    }
    sql += " S.unit_id AS \"Dim" + to_string(i_tblInfo.dimNameVec.size()) + "\",";
    sql += " S.value AS \"Value\"";

    // from value table where run id is first run of that model
    sql += " FROM " + i_tblInfo.valueTableName + " S" + 
    " WHERE S.run_id =" +
    " (" +
    " SELECT MIN(RL.run_id)" +
    " FROM run_lst RL" + 
    " INNER JOIN model_dic MD ON (MD.model_id = RL.model_id)" +
    " WHERE MD.model_name = " + toQuoted(i_metaRows.modelDic.name) + 
    " AND MD.model_ts = " + toQuoted(i_metaRows.modelDic.timestamp) +
    ")";

    return sql;
}

// validate metadata rows: uniqueness and referential integrity
void ModelSqlBuilder::validate(const MetaModelHolder & i_metaRows) const
{
    // validate model timestamp: must be same as in model source code
    if (i_metaRows.modelDic.timestamp != modelTs) 
        throw DbException("invalid model timestamp: %s, expected: %s", i_metaRows.modelDic.timestamp.c_str(), modelTs.c_str());

    int mId = i_metaRows.modelDic.modelId;
        
    // model_dic_txt table
    // unique: model id and language; master key: model id
    for (vector<ModelDicTxtLangRow>::const_iterator rowIt = i_metaRows.modelTxt.cbegin(); rowIt != i_metaRows.modelTxt.cend(); ++rowIt) {

        if (rowIt->modelId != mId) 
            throw DbException("in model_dic_txt invalid model id: %d, expected: %d in row with language id: %d and name: %s", rowIt->modelId, mId, rowIt->langId, rowIt->langName.c_str());

        for (vector<ModelDicTxtLangRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.modelTxt.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->langName == otherIt->langName) 
                throw DbException("in model_dic_txt not unique model id: %d and language name: %s", rowIt->modelId, rowIt->langName.c_str());
        }
    }

    // type_dic table
    // unique: model id and type; master key: model id
    for (vector<TypeDicRow>::const_iterator rowIt = i_metaRows.typeDic.cbegin(); rowIt != i_metaRows.typeDic.cend(); ++rowIt) {

        if (rowIt->modelId != mId) 
            throw DbException("in type_dic invalid model id: %d, expected: %d in row with type id: %d", rowIt->modelId, mId, rowIt->typeId);

        for (vector<TypeDicRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.typeDic.cend(); ++otherIt) {
            
            if (rowIt->modelId == otherIt->modelId && rowIt->typeId == otherIt->typeId) 
                throw DbException("in type_dic not unique model id: %d and type id: %d", rowIt->modelId, rowIt->typeId);

            if (rowIt->modelId == otherIt->modelId && rowIt->name == otherIt->name) 
                throw DbException("in type_dic not unique model id: %d and type name: %s", rowIt->modelId, rowIt->name.c_str());
        }
    }

    // type_dic_txt table
    // unique: model id, type id, language; master key: model id, type id
    for (vector<TypeDicTxtLangRow>::const_iterator rowIt = i_metaRows.typeTxt.cbegin(); rowIt != i_metaRows.typeTxt.cend(); ++rowIt) {

        for (vector<TypeDicTxtLangRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.typeTxt.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->typeId == otherIt->typeId && rowIt->langName == otherIt->langName) 
                throw DbException("in type_dic_txt not unique model id: %d, type id: %d and language name: %s", rowIt->modelId, rowIt->typeId, rowIt->langName.c_str());
        }

        if (std::none_of(
            i_metaRows.typeDic.cbegin(), 
            i_metaRows.typeDic.cend(),
            [rowIt](const TypeDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.typeId == rowIt->typeId; }
            ))
            throw DbException("in type_dic_txt invalid model id: %d and type id: %d: not found in type_dic", rowIt->modelId, rowIt->typeId);
    }

    // type_enum_lst table
    // unique: model id, type id, enum id; master key: model id, type id
    for (vector<TypeEnumLstRow>::const_iterator rowIt = i_metaRows.typeEnum.cbegin(); rowIt != i_metaRows.typeEnum.cend(); ++rowIt) {

        for (vector<TypeEnumLstRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.typeEnum.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->typeId == otherIt->typeId && rowIt->enumId == otherIt->enumId) 
                throw DbException("in type_enum_lst not unique model id: %d, type id: %d and enum id: %d", rowIt->modelId, rowIt->typeId, rowIt->enumId);
        }

        if (std::none_of(
            i_metaRows.typeDic.cbegin(), 
            i_metaRows.typeDic.cend(),
            [rowIt](const TypeDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.typeId == rowIt->typeId; }
            ))
            throw DbException("in type_enum_lst invalid model id: %d and type id: %d: not found in type_dic", rowIt->modelId, rowIt->typeId);
    }

    // type_enum_txt table
    // unique: model id, type id, enum id, language; master key: model id, type id, enum id
    for (vector<TypeEnumTxtLangRow>::const_iterator rowIt = i_metaRows.typeEnumTxt.cbegin(); rowIt != i_metaRows.typeEnumTxt.cend(); ++rowIt) {

        for (vector<TypeEnumTxtLangRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.typeEnumTxt.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->typeId == otherIt->typeId && rowIt->enumId == otherIt->enumId && rowIt->langName == otherIt->langName) 
                throw DbException("in type_enum_txt not unique model id: %d, type id: %d enum id: %d and language name: %s", rowIt->modelId, rowIt->typeId, rowIt->enumId, rowIt->langName.c_str());
        }

        if (std::none_of(
            i_metaRows.typeEnum.cbegin(), 
            i_metaRows.typeEnum.cend(),
            [rowIt](const TypeEnumLstRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.typeId == rowIt->typeId && i_row.enumId == rowIt->enumId; }
            ))
            throw DbException("in type_enum_txt invalid model id: %d, type id: %d and enum id: %d not found in type_enum_lst", rowIt->modelId, rowIt->typeId, rowIt->enumId);
    }

    // parameter_dic table
    // unique: model id, parameter id and name; master key: model id; foreign key: model id, type id;
    for (vector<ParamDicRow>::const_iterator rowIt = i_metaRows.paramDic.cbegin(); rowIt != i_metaRows.paramDic.cend(); ++rowIt) {

        if (rowIt->modelId != mId) 
            throw DbException("in parameter_dic invalid model id: %d, expected: %d in row with type id: %d", rowIt->modelId, mId, rowIt->typeId);

        for (vector<ParamDicRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.paramDic.cend(); ++otherIt) {
            
            if (rowIt->modelId == otherIt->modelId && rowIt->paramId == otherIt->paramId) 
                throw DbException("in parameter_dic not unique model id: %d and parameter id: %d", rowIt->modelId, rowIt->paramId);

            if (rowIt->modelId == otherIt->modelId && rowIt->paramName == otherIt->paramName) 
                throw DbException("in parameter_dic not unique model id: %d and parameter name: %s", rowIt->modelId, rowIt->paramName.c_str());
        }

        if (std::none_of(
            i_metaRows.typeDic.cbegin(), 
            i_metaRows.typeDic.cend(),
            [rowIt](const TypeDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.typeId == rowIt->typeId; }
            ))
            throw DbException("in parameter_dic invalid model id: %d and type id: %d: not found in type_dic", rowIt->modelId, rowIt->typeId);
    }

    // parameter_dic_txt table
    // unique: model id, parameter id, language; master key: model id, parameter id
    for (vector<ParamDicTxtLangRow>::const_iterator rowIt = i_metaRows.paramTxt.cbegin(); rowIt != i_metaRows.paramTxt.cend(); ++rowIt) {

        for (vector<ParamDicTxtLangRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.paramTxt.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->paramId == otherIt->paramId && rowIt->langName == otherIt->langName) 
                throw DbException("in parameter_dic_txt not unique model id: %d, parameter id: %d and language name: %s", rowIt->modelId, rowIt->paramId, rowIt->langName.c_str());
        }

        if (std::none_of(
            i_metaRows.paramDic.cbegin(), 
            i_metaRows.paramDic.cend(),
            [rowIt](const ParamDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.paramId == rowIt->paramId; }
            ))
            throw DbException("in parameter_dic_txt invalid model id: %d and parameter id: %d: not found in parameter_dic", rowIt->modelId, rowIt->paramId);
    }

    // parameter_dims table
    // unique: model id, parameter id, dimension name; master key: model id, parameter id; foreign key: model id, type id;
    for (vector<ParamDimsRow>::const_iterator rowIt = i_metaRows.paramDims.cbegin(); rowIt != i_metaRows.paramDims.cend(); ++rowIt) {

        for (vector<ParamDimsRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.paramDims.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->paramId == otherIt->paramId && rowIt->name == otherIt->name) 
                throw DbException("in parameter_dims not unique model id: %d, parameter id: %d and dimension name: %s", rowIt->modelId, rowIt->paramId, rowIt->name.c_str());
        }

        if (std::none_of(
            i_metaRows.paramDic.cbegin(), 
            i_metaRows.paramDic.cend(),
            [rowIt](const ParamDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.paramId == rowIt->paramId; }
            ))
            throw DbException("in parameter_dims invalid model id: %d and parameter id: %d: not found in parameter_dic", rowIt->modelId, rowIt->paramId);

        if (std::none_of(
            i_metaRows.typeDic.cbegin(), 
            i_metaRows.typeDic.cend(),
            [rowIt](const TypeDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.typeId == rowIt->typeId; }
            ))
            throw DbException("in parameter_dims invalid model id: %d and type id: %d: not found in type_dic", rowIt->modelId, rowIt->typeId);
    }

    // table_dic table
    // unique: model id, table id and name; master key: model id
    for (vector<TableDicRow>::const_iterator rowIt = i_metaRows.tableDic.cbegin(); rowIt != i_metaRows.tableDic.cend(); ++rowIt) {

        if (rowIt->modelId != mId) 
            throw DbException("in table_dic invalid model id: %d, expected: %d in row with table id: %d, name: %s", rowIt->modelId, mId, rowIt->tableId, rowIt->tableName.c_str());

        for (vector<TableDicRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.tableDic.cend(); ++otherIt) {
            
            if (rowIt->modelId == otherIt->modelId && rowIt->tableId == otherIt->tableId) 
                throw DbException("in table_dic not unique model id: %d and table id: %d", rowIt->modelId, rowIt->tableId);

            if (rowIt->modelId == otherIt->modelId && rowIt->tableName == otherIt->tableName) 
                throw DbException("in table_dic not unique model id: %d and table name: %s", rowIt->modelId, rowIt->tableName.c_str());
        }
    }

    // table_dic_txt table
    // unique: model id, table id, language; master key: model id, table id
    for (vector<TableDicTxtLangRow>::const_iterator rowIt = i_metaRows.tableTxt.cbegin(); rowIt != i_metaRows.tableTxt.cend(); ++rowIt) {

        for (vector<TableDicTxtLangRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.tableTxt.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->tableId == otherIt->tableId && rowIt->langName == otherIt->langName) 
                throw DbException("in table_dic_txt not unique model id: %d, table id: %d and language name: %s", rowIt->modelId, rowIt->tableId, rowIt->langName.c_str());
        }

        if (std::none_of(
            i_metaRows.tableDic.cbegin(), 
            i_metaRows.tableDic.cend(),
            [rowIt](const TableDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId; }
            ))
            throw DbException("in table_dic_txt invalid model id: %d and table id: %d: not found in table_dic", rowIt->modelId, rowIt->tableId);
    }

    // table_dims table
    // unique: model id, table id, dimension name; master key: model id, table id; foreign key: model id, type id;
    for (vector<TableDimsRow>::const_iterator rowIt = i_metaRows.tableDims.cbegin(); rowIt != i_metaRows.tableDims.cend(); ++rowIt) {

        for (vector<TableDimsRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.tableDims.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->tableId == otherIt->tableId && rowIt->name == otherIt->name) 
                throw DbException("in table_dims not unique model id: %d, table id: %d and dimension name: %s", rowIt->modelId, rowIt->tableId, rowIt->name.c_str());
        }

        if (std::none_of(
            i_metaRows.tableDic.cbegin(), 
            i_metaRows.tableDic.cend(),
            [rowIt](const TableDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId; }
            ))
            throw DbException("in table_dims invalid model id: %d and table id: %d: not found in table_dic", rowIt->modelId, rowIt->tableId);

        if (std::none_of(
            i_metaRows.typeDic.cbegin(), 
            i_metaRows.typeDic.cend(),
            [rowIt](const TypeDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.typeId == rowIt->typeId; }
            ))
            throw DbException("in table_dims invalid model id: %d and type id: %d: not found in type_dic", rowIt->modelId, rowIt->typeId);
    }

    // table_dims_txt table
    // unique: model id, table id, dimension name, language; master key: model id, table id, dimension name;
    for (vector<TableDimsTxtLangRow>::const_iterator rowIt = i_metaRows.tableDimsTxt.cbegin(); rowIt != i_metaRows.tableDimsTxt.cend(); ++rowIt) {

        for (vector<TableDimsTxtLangRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.tableDimsTxt.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->tableId == otherIt->tableId && rowIt->name == otherIt->name && rowIt->langName == otherIt->langName) 
                throw DbException("in table_dims_txt not unique model id: %d, table id: %d, dimension name: %s and language: %s", rowIt->modelId, rowIt->tableId, rowIt->name.c_str(), rowIt->langName.c_str());
        }

        if (std::none_of(
            i_metaRows.tableDims.cbegin(), 
            i_metaRows.tableDims.cend(),
            [rowIt](const TableDimsRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId && i_row.name == rowIt->name; }
            ))
            throw DbException("in table_dims_txt invalid model id: %d table id: %d and dimension name: %s: not found in table_dims", rowIt->modelId, rowIt->tableId, rowIt->name.c_str());
    }

    // table_acc table
    // unique: model id, table id, accumulator id and name; master key: model id, table id;
    for (vector<TableAccRow>::const_iterator rowIt = i_metaRows.tableAcc.cbegin(); rowIt != i_metaRows.tableAcc.cend(); ++rowIt) {

        for (vector<TableAccRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.tableAcc.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->tableId == otherIt->tableId && rowIt->accId == otherIt->accId) 
                throw DbException("in table_acc not unique model id: %d, table id: %d and accumulator id: %d", rowIt->modelId, rowIt->tableId, rowIt->accId);

            if (rowIt->modelId == otherIt->modelId && rowIt->tableId == otherIt->tableId && rowIt->name == otherIt->name) 
                throw DbException("in table_acc not unique model id: %d, table id: %d and accumulator name: %s", rowIt->modelId, rowIt->tableId, rowIt->name.c_str());
        }

        if (std::none_of(
            i_metaRows.tableDic.cbegin(), 
            i_metaRows.tableDic.cend(),
            [rowIt](const TableDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId; }
            ))
            throw DbException("in table_acc invalid model id: %d and table id: %d: not found in table_dic", rowIt->modelId, rowIt->tableId);
    }

    // table_acc_txt table
    // unique: model id, table id, accumulator id, language; master key: model id, table id, accumulator id;
    for (vector<TableAccTxtLangRow>::const_iterator rowIt = i_metaRows.tableAccTxt.cbegin(); rowIt != i_metaRows.tableAccTxt.cend(); ++rowIt) {

        for (vector<TableAccTxtLangRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.tableAccTxt.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->tableId == otherIt->tableId && rowIt->accId == otherIt->accId && rowIt->langName == otherIt->langName) 
                throw DbException("in table_acc_txt not unique model id: %d, table id: %d, accumulator id: %d and language: %s", rowIt->modelId, rowIt->tableId, rowIt->accId, rowIt->langName.c_str());
        }

        if (std::none_of(
            i_metaRows.tableAcc.cbegin(), 
            i_metaRows.tableAcc.cend(),
            [rowIt](const TableAccRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId && i_row.accId == rowIt->accId; }
            ))
            throw DbException("in table_acc_txt invalid model id: %d table id: %d and accumulator id: %d: not found in table_acc", rowIt->modelId, rowIt->tableId, rowIt->accId);
    }

    // table_unit table
    // unique: model id, table id, unit id and name; master key: model id, table id;
    for (vector<TableUnitRow>::const_iterator rowIt = i_metaRows.tableUnit.cbegin(); rowIt != i_metaRows.tableUnit.cend(); ++rowIt) {

        for (vector<TableUnitRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.tableUnit.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->tableId == otherIt->tableId && rowIt->unitId == otherIt->unitId) 
                throw DbException("in table_unit not unique model id: %d, table id: %d and unit id: %d", rowIt->modelId, rowIt->tableId, rowIt->unitId);

            if (rowIt->modelId == otherIt->modelId && rowIt->tableId == otherIt->tableId && rowIt->name == otherIt->name) 
                throw DbException("in table_unit not unique model id: %d, table id: %d and unit name: %s", rowIt->modelId, rowIt->tableId, rowIt->name.c_str());
        }

        if (std::none_of(
            i_metaRows.tableDic.cbegin(), 
            i_metaRows.tableDic.cend(),
            [rowIt](const TableDicRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId; }
            ))
            throw DbException("in table_unit invalid model id: %d and table id: %d: not found in table_dic", rowIt->modelId, rowIt->tableId);
    }

    // table_unit_txt table
    // unique: model id, table id, unit id, language; master key: model id, table id, unit id;
    for (vector<TableUnitTxtLangRow>::const_iterator rowIt = i_metaRows.tableUnitTxt.cbegin(); rowIt != i_metaRows.tableUnitTxt.cend(); ++rowIt) {

        for (vector<TableUnitTxtLangRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.tableUnitTxt.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->tableId == otherIt->tableId && rowIt->unitId == otherIt->unitId && rowIt->langName == otherIt->langName) 
                throw DbException("in table_unit_txt not unique model id: %d, table id: %d, unit id: %d and language: %s", rowIt->modelId, rowIt->tableId, rowIt->unitId, rowIt->langName.c_str());
        }

        if (std::none_of(
            i_metaRows.tableUnit.cbegin(), 
            i_metaRows.tableUnit.cend(),
            [rowIt](const TableUnitRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId && i_row.unitId == rowIt->unitId; }
            ))
            throw DbException("in table_unit_txt invalid model id: %d table id: %d and unit id: %d: not found in table_unit", rowIt->modelId, rowIt->tableId, rowIt->unitId);
    }

    // group_lst table
    // unique: model id, group id; master key: model id
    for (vector<GroupLstRow>::const_iterator rowIt = i_metaRows.groupLst.cbegin(); rowIt != i_metaRows.groupLst.cend(); ++rowIt) {

        if (rowIt->modelId != mId) 
            throw DbException("in group_lst invalid model id: %d, expected: %d in row with group id: %d, name: %s", rowIt->modelId, mId, rowIt->groupId, rowIt->name.c_str());

        for (vector<GroupLstRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.groupLst.cend(); ++otherIt) {
            
            if (rowIt->modelId == otherIt->modelId && rowIt->groupId == otherIt->groupId) 
                throw DbException("in group_lst not unique model id: %d and group id: %d", rowIt->modelId, rowIt->groupId);
        }
    }

    // group_txt table
    // unique: model id, group id, language; master key: model id, group id
    for (vector<GroupTxtLangRow>::const_iterator rowIt = i_metaRows.groupTxt.cbegin(); rowIt != i_metaRows.groupTxt.cend(); ++rowIt) {

        for (vector<GroupTxtLangRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.groupTxt.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->groupId == otherIt->groupId && rowIt->langName == otherIt->langName) 
                throw DbException("in group_txt not unique model id: %d, group id: %d and language name: %s", rowIt->modelId, rowIt->groupId, rowIt->langName.c_str());
        }

        if (std::none_of(
            i_metaRows.groupLst.cbegin(), 
            i_metaRows.groupLst.cend(),
            [rowIt](const GroupLstRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.groupId == rowIt->groupId; }
            ))
            throw DbException("in group_txt invalid model id: %d and group id: %d: not found in group_lst", rowIt->modelId, rowIt->groupId);
    }

    // group_pc table
    // unique: model id, group id, child position; master key: model id, group id
    for (vector<GroupPcRow>::const_iterator rowIt = i_metaRows.groupPc.cbegin(); rowIt != i_metaRows.groupPc.cend(); ++rowIt) {

        for (vector<GroupPcRow>::const_iterator otherIt = rowIt + 1; otherIt != i_metaRows.groupPc.cend(); ++otherIt) {

            if (rowIt->modelId == otherIt->modelId && rowIt->groupId == otherIt->groupId && rowIt->childPos == otherIt->childPos) 
                throw DbException("in group_pc not unique model id: %d, group id: %d and child position: %d", rowIt->modelId, rowIt->groupId, rowIt->childPos);
        }

        if (std::none_of(
            i_metaRows.groupLst.cbegin(), 
            i_metaRows.groupLst.cend(),
            [rowIt](const GroupLstRow & i_row) -> bool 
                { return i_row.modelId == rowIt->modelId && i_row.groupId == rowIt->groupId; }
            ))
            throw DbException("in group_pc invalid model id: %d and group id: %d: not found in group_lst", rowIt->modelId, rowIt->groupId);
    }
}

// set field values for model_dic table row
void ModelSqlBuilder::setModelDicRow(ModelDicRow & io_mdRow)
{
    // validate model timestamp: must be same as in model source code
    if (io_mdRow.timestamp != modelTs) throw DbException("invalid model timestamp: %s, expected: %s", io_mdRow.timestamp.c_str(), modelTs.c_str());

    // make model prefix from model name and timestamp
    io_mdRow.modelPrefix = ModelInsertSql::makeModelPrefix(io_mdRow.name, io_mdRow.timestamp);

    // validate table prefixes: it must not exceed max size and must be unique
    io_mdRow.paramPrefix = !io_mdRow.paramPrefix.empty() ? io_mdRow.paramPrefix : "p";
    io_mdRow.setPrefix = !io_mdRow.setPrefix.empty() ? io_mdRow.setPrefix : "w";
    io_mdRow.subPrefix = !io_mdRow.subPrefix.empty() ? io_mdRow.subPrefix : "s";
    io_mdRow.valuePrefix = !io_mdRow.valuePrefix.empty() ? io_mdRow.valuePrefix : "v";

    if (io_mdRow.paramPrefix.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) parameter tables prefix: %s", io_mdRow.paramPrefix.c_str());
    if (io_mdRow.setPrefix.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) workset tables prefix: %s", io_mdRow.setPrefix.c_str());
    if (io_mdRow.subPrefix.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) subsample tables prefix: %s", io_mdRow.subPrefix.c_str());
    if (io_mdRow.valuePrefix.length() > OM_DB_TABLE_TYPE_PREFIX_LEN) throw DbException("invalid (too long) value tables prefix: %s", io_mdRow.valuePrefix.c_str());

    if (io_mdRow.paramPrefix == io_mdRow.setPrefix || io_mdRow.paramPrefix == io_mdRow.subPrefix || io_mdRow.paramPrefix == io_mdRow.valuePrefix || 
        io_mdRow.setPrefix == io_mdRow.subPrefix || io_mdRow.setPrefix == io_mdRow.valuePrefix || io_mdRow.subPrefix == io_mdRow.valuePrefix)
        throw DbException(
            "invalid (not unique) table prefixes: %s %s %s %s, model name: %s", 
            io_mdRow.paramPrefix.c_str(), io_mdRow.setPrefix.c_str(), io_mdRow.subPrefix.c_str(), io_mdRow.valuePrefix.c_str(), io_mdRow.name.c_str()
            );
}

// collect info for db parameter tables
void ModelSqlBuilder::setParamTableInfo(const MetaModelHolder & i_metaRows)
{
    // collect table information for all parameters
    for (const ParamDicRow & paramRow : i_metaRows.paramDic) {

        // get id and convert name into db table suffix
        ParamTblInfo tblInf;
        tblInf.id = paramRow.paramId;
        tblInf.name = paramRow.paramName;
        tblInf.suffix = ModelInsertSql::makeDbNameSuffix(tblInf.id, tblInf.name);
        tblInf.paramTableName = i_metaRows.modelDic.modelPrefix + i_metaRows.modelDic.paramPrefix + tblInf.suffix;
        tblInf.setTableName = i_metaRows.modelDic.modelPrefix + i_metaRows.modelDic.setPrefix + tblInf.suffix;

        // collect dimension names
        tblInf.dimNameVec.clear();
        for (const ParamDimsRow & dimRow : i_metaRows.paramDims) {
            if (dimRow.paramId == tblInf.id) tblInf.dimNameVec.push_back(dimRow.name);
        }

        // set value column db type by parameter type
        tblInf.valueTypeName.clear();
        for (const TypeDicRow & typeRow : i_metaRows.typeDic) {

            if (typeRow.typeId == paramRow.typeId) {

                // built-in types (ordered as in omc grammar for clarity)

                // C++ ambiguous integral type
                // (in C/C++, the signedness of char is not specified)
                if (equalNoCase(typeRow.name.c_str(), "char")) tblInf.valueTypeName = "SMALLINT";

                // C++ signed integral types
                if (equalNoCase(typeRow.name.c_str(), "schar")) tblInf.valueTypeName = "SMALLINT";
                if (equalNoCase(typeRow.name.c_str(), "short")) tblInf.valueTypeName = "SMALLINT";
                if (equalNoCase(typeRow.name.c_str(), "int")) tblInf.valueTypeName = "INT";
                if (equalNoCase(typeRow.name.c_str(), "long")) tblInf.valueTypeName = "INT";
                if (equalNoCase(typeRow.name.c_str(), "llong")) tblInf.valueTypeName = "BIGINT";

                // C++ unsigned integral types (including bool)
                if (equalNoCase(typeRow.name.c_str(), "bool")) tblInf.valueTypeName = "TINYINT";
                if (equalNoCase(typeRow.name.c_str(), "uchar")) tblInf.valueTypeName = "TINYINT";
                if (equalNoCase(typeRow.name.c_str(), "ushort")) tblInf.valueTypeName = "INT";
                if (equalNoCase(typeRow.name.c_str(), "uint")) tblInf.valueTypeName = "BIGINT";
                if (equalNoCase(typeRow.name.c_str(), "ulong")) tblInf.valueTypeName = "BIGINT";
                if (equalNoCase(typeRow.name.c_str(), "ullong")) tblInf.valueTypeName = "BIGINT";

                // C++ floating point types
                if (equalNoCase(typeRow.name.c_str(), "float")) tblInf.valueTypeName = "FLOAT";
                if (equalNoCase(typeRow.name.c_str(), "double")) tblInf.valueTypeName = "FLOAT";
                if (equalNoCase(typeRow.name.c_str(), "ldouble")) tblInf.valueTypeName = "FLOAT";

                // Changeable numeric types
                if (equalNoCase(typeRow.name.c_str(), "Time")) tblInf.valueTypeName = "FLOAT";
                if (equalNoCase(typeRow.name.c_str(), "real")) tblInf.valueTypeName = "FLOAT";
                if (equalNoCase(typeRow.name.c_str(), "integer")) tblInf.valueTypeName = "INT";
                if (equalNoCase(typeRow.name.c_str(), "counter")) tblInf.valueTypeName = "INT";

                // Not implemented (a string)
                // if (equalNoCase(typeRow.name.c_str(), "file")) tblInf.valueTypeName = "INT";

                // model specific types: it must be enum
                if (typeRow.typeId > OM_MAX_BUILTIN_TYPE_ID) {
                    tblInf.valueTypeName = "INT";
                }
                break;
            }
        }
        if (tblInf.valueTypeName.empty()) 
            throw DbException("invalid type id: %d for parameter id: %d, name: %s", paramRow.typeId, paramRow.paramId, paramRow.paramName.c_str());
        
        paramInfoVec.push_back(tblInf);     // add to parameters info vector
    }
}    

// collect info for db subsample tables and value views
void ModelSqlBuilder::setOutTableInfo(MetaModelHolder & i_metaRows)
{
    // collect information for all output tables
    for (const TableDicRow & tableRow : i_metaRows.tableDic) {

        // get id and convert name into db table suffix, subsample table name and value view name
        OutTblInfo tblInf;
        tblInf.id = tableRow.tableId;
        tblInf.name = tableRow.tableName;
        tblInf.suffix = ModelInsertSql::makeDbNameSuffix(tblInf.id, tblInf.name);
        tblInf.subTableName = i_metaRows.modelDic.modelPrefix + i_metaRows.modelDic.subPrefix + tblInf.suffix;
        tblInf.valueTableName = i_metaRows.modelDic.modelPrefix + i_metaRows.modelDic.valuePrefix + tblInf.suffix;

        // collect dimension names
        tblInf.dimNameVec.clear();
        for (const TableDimsRow & dimRow : i_metaRows.tableDims) {
            if (dimRow.tableId == tblInf.id) tblInf.dimNameVec.push_back(dimRow.name);
        }

        // collect accumulator names
        tblInf.accNameVec.clear();
        for (const TableAccRow & accRow : i_metaRows.tableAcc) {
            if (accRow.tableId == tblInf.id) tblInf.accNameVec.push_back(accRow.name);
        }
        if (tblInf.accNameVec.empty()) 
            throw DbException("output table accumulators not found for table: %s", tableRow.tableName.c_str());

        // translate expressions into sql
        ModelAggregationSql aggr(tblInf.subTableName, "S", tblInf.accNameVec, tblInf.dimNameVec);

        for (TableUnitRow & unitRow : i_metaRows.tableUnit) {
            if (unitRow.tableId == tblInf.id) {
                unitRow.expr = aggr.translateAggregationExpr(unitRow.src);  // translate expression to sql aggregation
            }
        }
        // if (tblInf.exprVec.empty()) 
        //    throw DbException("output table aggregation expressions not found for table: %s", tableRow.tableName.c_str());

        outInfoVec.push_back(tblInf);   // add to output tables info vector
    }
}
