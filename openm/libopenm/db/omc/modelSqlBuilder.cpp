// OpenM++ data library: class to produce new model sql from metadata
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "modelSqlBuilder.h"

using namespace openm;

// new model builder to create sql script specific to each db-provider
IModelBuilder * IModelBuilder::create(const string & i_providerNames, const string & i_sqlDir, const string & i_outputDir, const string & i_sqliteDir)
{
    return new ModelSqlBuilder(i_providerNames, i_sqlDir, i_outputDir, i_sqliteDir);
}

// release builder resources
IModelBuilder::~IModelBuilder() noexcept { }

// create new model builder and set db table name prefix and suffix rules
ModelSqlBuilder::ModelSqlBuilder(const string & i_providerNames, const string & i_sqlDir, const string & i_outputDir, const string & i_sqliteDir) :
    sqlDir(i_sqlDir),
    outputDir(i_outputDir),
    isSqlite(false)
{
    // parse and validate provider names
    dbProviderLst = IDbExec::parseListOfProviderNames(i_providerNames);
    if (dbProviderLst.empty()) throw DbException(LT("invalid (empty) list db-provider names"));

    isSqlite = dbProviderLst.cend() != std::find(dbProviderLst.cbegin(), dbProviderLst.cend(), SQLITE_DB_PROVIDER);

    sqliteDir = !i_sqliteDir.empty() ? i_sqliteDir : outputDir;
 }

// validate metadata and return sql script to create new model from supplied metadata rows
void ModelSqlBuilder::build(MetaModelHolder & io_metaRows)
{
    try {
        // validate input rows: uniqueness and referential integrity
        prepare(io_metaRows);

        // collect info for db parameter tables, db sub-value tables and value views
        setParamTableInfo(io_metaRows);
        setOutTableInfo(io_metaRows);

        // write sql script to create new model tables
        for (const string & providerName : dbProviderLst) {
            buildCreateModelTables(
                providerName, 
                io_metaRows, 
                makeFilePath(outputDir.c_str(), (io_metaRows.modelDic.name + "_create_" + providerName).c_str(), ".sql")
            );
        }

        // write sql script to drop model tables and views
        buildDropModelTables(
            io_metaRows, 
            makeFilePath(outputDir.c_str(), (io_metaRows.modelDic.name + "_drop").c_str(), ".sql")
        );

        // create SQLite database and publish model: insert model metadata and create model tables
        if (isSqlite) {
            dbExec.reset(IDbExec::create(
                SQLITE_DB_PROVIDER,
                "Database=" + makeFilePath(sqliteDir.c_str(), io_metaRows.modelDic.name.c_str(), ".sqlite") + ";" +
                " Timeout=86400;" +
                " OpenMode=Create;" +
                " DeleteExisting=true;"
            ));

            // run script to create openM++ metadata tables and insert default values
            dbExec->runSqlScript(fileToUtf8(makeFilePath(sqlDir.c_str(), "create_db.sql").c_str()));
            dbExec->runSqlScript(fileToUtf8(makeFilePath(sqlDir.c_str(), "insert_default.sql").c_str()));

            // insert new model metadata into SQLite database
            createModel(dbExec.get(), io_metaRows);

            // run sql script to create new model tables
            dbExec->runSqlScript(
                fileToUtf8(makeFilePath(
                    outputDir.c_str(), (io_metaRows.modelDic.name + "_create_" + SQLITE_DB_PROVIDER).c_str(), ".sql"
                ).c_str()));
        }
    }
    catch (HelperException & ex) {
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

// insert new model metadata into database and update id's with actual db values
void ModelSqlBuilder::createModel(IDbExec * i_dbExec, MetaModelHolder & io_metaRows)
{
    // start transaction and insert new model master row
    i_dbExec->beginTransaction();

    // insert language into lang_lst table, if not exist
    // update language list id's with actual database values
    // make language code to id map
    map<string, int> langMap;

    for (LangLstRow & row : io_metaRows.langLst) {
        ModelInsertSql::insertLangLst(i_dbExec, row);
        langMap[row.code] = row.langId;
    }

    // model master row: model_dic
    ModelInsertSql::insertModelDic(i_dbExec, io_metaRows.modelDic, langMap);

    // for all metadata rows update model id with actual value
    for (ModelDicTxtLangRow & row : io_metaRows.modelTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TypeDicRow & row : io_metaRows.typeDic) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TypeDicTxtLangRow & row : io_metaRows.typeTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TypeEnumLstRow & row : io_metaRows.typeEnum) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TypeEnumTxtLangRow & row : io_metaRows.typeEnumTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (ParamDicRow & row : io_metaRows.paramDic) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (ParamImportRow & row : io_metaRows.paramImport) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (ParamDicTxtLangRow & row : io_metaRows.paramTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (ParamDimsRow & row : io_metaRows.paramDims) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (ParamDimsTxtLangRow & row : io_metaRows.paramDimsTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TableDicRow & row : io_metaRows.tableDic) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TableDicTxtLangRow & row : io_metaRows.tableTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TableDimsRow & row : io_metaRows.tableDims) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TableDimsTxtLangRow & row : io_metaRows.tableDimsTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TableAccRow & row : io_metaRows.tableAcc) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TableAccTxtLangRow & row : io_metaRows.tableAccTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TableExprRow & row : io_metaRows.tableExpr) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (TableExprTxtLangRow & row : io_metaRows.tableExprTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (EntityDicRow & row : io_metaRows.entityDic) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (EntityDicTxtLangRow & row : io_metaRows.entityTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (EntityAttrRow & row : io_metaRows.entityAttr) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (EntityAttrTxtLangRow & row : io_metaRows.entityAttrTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (GroupLstRow & row : io_metaRows.groupLst) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (GroupTxtLangRow & row : io_metaRows.groupTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (GroupPcRow & row : io_metaRows.groupPc) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (EntityGroupLstRow & row : io_metaRows.entityGroupLst) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (EntityGroupTxtLangRow & row : io_metaRows.entityGroupTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (EntityGroupPcRow & row : io_metaRows.entityGroupPc) {
        row.modelId = io_metaRows.modelDic.modelId;
    }

    // model text rows: model_dic_txt
    for (ModelDicTxtLangRow & row : io_metaRows.modelTxt) {
        ModelInsertSql::insertModelDicText(i_dbExec, langMap, row);
    }

    // type rows: type_dic and model_type_dic
    // map model type id to Hid
    map<int, int> typeIdMap;
    for (const TypeDicRow & row : io_metaRows.typeDic) {
        typeIdMap[row.typeId] = ModelInsertSql::insertTypeDic(i_dbExec, row);
    }

    // type text rows: type_dic_txt
    for (TypeDicTxtLangRow & row : io_metaRows.typeTxt) {
        ModelInsertSql::insertTypeText(i_dbExec, langMap, typeIdMap, row);
    }

    // type enum rows: type_enum_lst
    int typeId = -1;
    int typeHid = -1;
    for (const TypeEnumLstRow & row : io_metaRows.typeEnum) {

        if (typeId != row.typeId) {     // on type change find type Hid by model type id
            typeId = row.typeId;
            map<int, int>::const_iterator it = typeIdMap.find(typeId);
            if (it == typeIdMap.cend()) throw DbException(LT("invalid type id: %d"), typeId);
            typeHid = it->second;
        }

        ModelInsertSql::insertTypeEnum(i_dbExec, typeHid, row);
    }

    // type enum text rows: type_enum_txt
    typeId = -1;
    typeHid = -1;
    for (TypeEnumTxtLangRow & row : io_metaRows.typeEnumTxt) {

        if (typeId != row.typeId) {     // on type change find type Hid by model type id
            typeId = row.typeId;
            map<int, int>::const_iterator it = typeIdMap.find(typeId);
            if (it == typeIdMap.cend()) throw DbException(LT("invalid type id: %d"), typeId);
            typeHid = it->second;
        }

        ModelInsertSql::insertTypeEnumText(i_dbExec, langMap, typeHid, row);
    }

    // model input parameter rows: parameter_dic and model_parameter_dic
    for (ParamDicRow & row : io_metaRows.paramDic) {
        ModelInsertSql::insertParamDic(i_dbExec, typeIdMap, row);
    }

    // model parameter import rows: model_parameter_import
    for (ParamImportRow & row : io_metaRows.paramImport) {
        ModelInsertSql::insertParamImport(i_dbExec, row);
    }

    // parameter text rows: parameter_dic_txt
    for (ParamDicTxtLangRow & row : io_metaRows.paramTxt) {
        auto paramRowIt = ParamDicRow::byKey(row.modelId, row.paramId, io_metaRows.paramDic);
        ModelInsertSql::insertParamText(i_dbExec, *paramRowIt, langMap, row);
    }

    // parameter dimension rows: parameter_dims
    for (const ParamDimsRow & row : io_metaRows.paramDims) {
        auto paramRowIt = ParamDicRow::byKey(row.modelId, row.paramId, io_metaRows.paramDic);
        ModelInsertSql::insertParamDims(i_dbExec, *paramRowIt, typeIdMap, row);
    }

    // parameter dimension text rows: parameter_dims_txt
    for (ParamDimsTxtLangRow & row : io_metaRows.paramDimsTxt) {
        auto paramRowIt = ParamDicRow::byKey(row.modelId, row.paramId, io_metaRows.paramDic);
        ModelInsertSql::insertParamDimsText(i_dbExec, *paramRowIt, langMap, row);
    }

    // model output table rows: table_dic and model_table_dic
    for (TableDicRow & row : io_metaRows.tableDic) {
        ModelInsertSql::insertTableDic(i_dbExec, row);
    }

    // output table text rows: table_dic_txt
    for (TableDicTxtLangRow & row : io_metaRows.tableTxt) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, io_metaRows.tableDic);
        ModelInsertSql::insertTableText(i_dbExec, *tableRowIt, langMap, row);
    }

    // output table dimension rows: table_dims
    for (const TableDimsRow & row : io_metaRows.tableDims) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, io_metaRows.tableDic);
        ModelInsertSql::insertTableDims(i_dbExec, *tableRowIt, typeIdMap, row);
    }

    // output table dimension text rows: table_dims_txt
    for (TableDimsTxtLangRow & row : io_metaRows.tableDimsTxt) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, io_metaRows.tableDic);
        ModelInsertSql::insertTableDimsText(i_dbExec, *tableRowIt, langMap, row);
    }

    // output table accumulator rows: table_acc
    for (const TableAccRow & row : io_metaRows.tableAcc) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, io_metaRows.tableDic);
        ModelInsertSql::insertTableAcc(i_dbExec, *tableRowIt, row);
    }

    // output table accumulator text rows: table_acc_txt
    for (TableAccTxtLangRow & row : io_metaRows.tableAccTxt) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, io_metaRows.tableDic);
        ModelInsertSql::insertTableAccText(i_dbExec, *tableRowIt, langMap, row);
    }

    // output table expression rows: table_expr
    for (TableExprRow & row : io_metaRows.tableExpr) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, io_metaRows.tableDic);
        ModelInsertSql::insertTableExpr(i_dbExec, *tableRowIt, row);
    }

    // output table expression text rows: table_expr_txt
    for (TableExprTxtLangRow & row : io_metaRows.tableExprTxt) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, io_metaRows.tableDic);
        ModelInsertSql::insertTableExprText(i_dbExec, *tableRowIt, langMap, row);
    }

    // model entity rows: entity_dic and model_entity_dic
    for (EntityDicRow & row : io_metaRows.entityDic) {
        ModelInsertSql::insertEntityDic(i_dbExec, row);
    }

    // entity text rows: entity_dic_txt
    for (EntityDicTxtLangRow & row : io_metaRows.entityTxt) {
        auto entityRowIt = EntityDicRow::byKey(row.modelId, row.entityId, io_metaRows.entityDic);
        ModelInsertSql::insertEntityText(i_dbExec, *entityRowIt, langMap, row);
    }

    // entity attribute rows: entity_attr
    for (const EntityAttrRow & row : io_metaRows.entityAttr) {
        auto entityRowIt = EntityDicRow::byKey(row.modelId, row.entityId, io_metaRows.entityDic);
        ModelInsertSql::insertEntityAttr(i_dbExec, *entityRowIt, typeIdMap, row);
    }

    // entity attribute text rows: entity_attr_txt
    for (EntityAttrTxtLangRow & row : io_metaRows.entityAttrTxt) {
        auto entityRowIt = EntityDicRow::byKey(row.modelId, row.entityId, io_metaRows.entityDic);
        ModelInsertSql::insertEntityAttrText(i_dbExec, *entityRowIt, langMap, row);
    }

    // group of parameters or output tables: group_lst
    for (const GroupLstRow & row : io_metaRows.groupLst) {
        ModelInsertSql::insertGroupLst(i_dbExec, row);
    }

    // groups text: group_txt
    for (GroupTxtLangRow & row : io_metaRows.groupTxt) {
        ModelInsertSql::insertGroupText(i_dbExec, langMap, row);
    }

    // groups parent-child relationships: group_pc
    for (const GroupPcRow & row : io_metaRows.groupPc) {
        ModelInsertSql::insertGroupPc(i_dbExec, row);
    }

    // group of entity attributes: entity_group_lst
    for (const EntityGroupLstRow & row : io_metaRows.entityGroupLst) {
        ModelInsertSql::insertEntityGroupLst(i_dbExec, row);
    }

    // groups text: entity_group_txt
    for (EntityGroupTxtLangRow & row : io_metaRows.entityGroupTxt) {
        ModelInsertSql::insertEntityGroupText(i_dbExec, langMap, row);
    }

    // groups parent-child relationships: entity_group_pc
    for (const EntityGroupPcRow & row : io_metaRows.entityGroupPc) {
        ModelInsertSql::insertEntityGroupPc(i_dbExec, row);
    }

    // if default model profile not empty then insert rows into profile_lst and profile_option tables
    ModelInsertSql::insertModelProfile(i_dbExec, io_metaRows.profileName, io_metaRows.profileRows);
 
    // insert model metadata compeleted
    i_dbExec->commit();
}

// write sql script to create new model tables and views
void ModelSqlBuilder::buildCreateModelTables(const string & i_sqlProvider, const MetaModelHolder & i_metaRows,  const string & i_filePath) const
{
    // script header
    ModelSqlWriter wr(i_filePath);
    wr.outFs <<
        "--\n" <<
        "-- create model tables: " << i_metaRows.modelDic.name << '\n' <<
        "-- model digest:        " << i_metaRows.modelDic.digest << '\n' <<
        "-- script created:      " << toDateTimeString(theLog->timeStamp()) << '\n' <<
        "--\n\n";
    wr.throwOnFail();

    // create tables for model input parameters
    wr.write(
        "--\n" \
        "-- create model input parameters\n" \
        "--\n"
        );
    for (size_t k = 0; k < i_metaRows.paramDic.size(); k++) {
        paramCreateTable(i_sqlProvider, i_metaRows.paramDic[k].dbRunTable, "run_id", paramInfoVec[k], wr);
        paramCreateTable(i_sqlProvider, i_metaRows.paramDic[k].dbSetTable, "set_id", paramInfoVec[k], wr);
    }
    wr.write("\n");

    // create model output tables and views
    wr.write(
        "--\n" \
        "-- create model output tables and views\n" \
        "--\n"
        );
    for (size_t k = 0; k < i_metaRows.tableDic.size(); k++) {
        accCreateTable(i_sqlProvider, i_metaRows.tableDic[k].dbAccTable, outInfoVec[k], wr);
        valueCreateTable(i_sqlProvider, i_metaRows.tableDic[k].dbExprTable, outInfoVec[k], wr);
        accAllCreateView(
            i_sqlProvider, 
            i_metaRows.tableDic[k].dbAccAll, 
            outInfoVec[k], 
            i_metaRows.tableDic[k].dbAccTable, 
            i_metaRows.tableAcc, 
            wr);
    }
    wr.write("\n");
}

// write sql script to drop model tables and views
void ModelSqlBuilder::buildDropModelTables(const MetaModelHolder & i_metaRows,  const string & i_filePath) const
{
    // script header
    ModelSqlWriter wr(i_filePath);
    wr.outFs <<
        "--\n"
        "-- drop model tables: " << i_metaRows.modelDic.name << '\n' <<
        "-- model digest:      " << i_metaRows.modelDic.digest << '\n' <<
        "-- script created:    " << toDateTimeString(theLog->timeStamp()) << '\n' <<
        "--\n" <<
        "-- DROP MODEL TABLES" << '\n' <<
        "-- DO NOT USE THIS SQL UNLESS YOU HAVE TO" << '\n' <<
        "-- IT WILL DELETE ALL MODEL DATA" << '\n' <<
        "--\n\n";
    wr.throwOnFail();

    // drop model output tables
    wr.write(
        "--\n" \
        "-- drop model output views and tables\n" \
        "--\n"
        );
    for (const TableDicRow & tblRow : i_metaRows.tableDic) {
        wr.writeLine("DROP VIEW " + tblRow.dbAccAll + ";");
        wr.writeLine("DROP TABLE " + tblRow.dbAccTable + ";");
        wr.writeLine("DROP TABLE " + tblRow.dbExprTable + ";");
    }
    wr.write("\n");

    // drop model parameters tables
    wr.write(
        "--\n" \
        "-- drop model input parameters tables\n" \
        "--\n"
        );
    for (const ParamDicRow & paramRow : i_metaRows.paramDic) {
        wr.writeLine("DROP TABLE " + paramRow.dbRunTable + ";");
        wr.writeLine("DROP TABLE " + paramRow.dbSetTable + ";");
    }
    wr.write("\n");

}

// start sql script to create new working set
void ModelSqlBuilder::beginWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet) 
{
    try {
        if (!isSqlite) throw DbException(LT("cannot create workset without model SQLite database."));

        // validate workset metadata: uniqueness and referential integrity
        prepareWorkset(i_metaRows, io_metaSet);

        // calculate workset digest
        //io_metaSet.worksetRow.digest = makeWorksetDigest(io_metaSet.worksetRow, i_metaRows, io_metaSet);

        // clear added flag for all model parameters
        for (ParamTblInfo & tblInfo : paramInfoVec) {
            tblInfo.isAdded = false;
        }

        // start workset transaction and insert new workset metadata
        dbExec->beginTransaction();
        ModelInsertSql::createWorksetMeta(dbExec.get(), i_metaRows, io_metaSet);
    }
    catch (HelperException & ex) {
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

/** finish sql script to create new working set */
void ModelSqlBuilder::endWorkset(const MetaModelHolder & /* i_metaRows */, const MetaSetLangHolder & i_metaSet)
{
    try {
        if (!isSqlite) throw DbException(LT("cannot create workset without model SQLite database."));

        // validate workset parameters: all parameters must be added to workset
        for (const ParamTblInfo & tblInfo : paramInfoVec) {
            if (!tblInfo.isAdded)
                throw DbException(LT("workset %s must include all model parameters, missing: %d: %s"), i_metaSet.worksetRow.name.c_str(), tblInfo.id, tblInfo.name.c_str());
        }

        // mark workset as readonly: ready to use
        dbExec->update("UPDATE workset_lst SET is_readonly = 1 WHERE set_id = " + to_string(i_metaSet.worksetRow.setId));

        // workset compeleted
        dbExec->commit();
    }
    catch (HelperException & ex) {
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

// append scalar parameter value to sql script for new working set creation 
void ModelSqlBuilder::addWorksetParameter(
    const MetaModelHolder & i_metaRows, const MetaSetLangHolder & i_metaSet, const string & i_name, int i_subId, const string & i_value
    )
{
    // check parameters
    if (!isSqlite) throw DbException(LT("cannot create workset without model SQLite database."));
    if (i_name.empty()) throw DbException(LT("invalid (empty) input parameter name"));

    // find parameter info
    auto paramRow = std::find_if(
        i_metaRows.paramDic.cbegin(),
        i_metaRows.paramDic.cend(),
        [i_name](const ParamDicRow & i_row) -> bool { return i_row.paramName == i_name; }
    );
    if (paramRow == i_metaRows.paramDic.cend()) throw DbException(LT("parameter not found in parameters dictionary: %s"), i_name.c_str());

    auto paramInfo = std::find_if(
        paramInfoVec.begin(), 
        paramInfoVec.end(),
        [i_name](const ParamTblInfo & i_info) -> bool { return i_info.name == i_name; }
    );
    if (paramInfo == paramInfoVec.cend()) throw DbException(LT("parameter not found in parameters dictionary: %s"), i_name.c_str());

    // create sql to insert value of scalar paarameter
    doAddScalarWorksetParameter(i_metaSet.worksetRow.setId, i_name, paramRow->dbSetTable, paramInfo, i_subId, i_value);

    paramInfo->isAdded = true;  // done
}

// impelementation of append scalar parameter value to sql script
// it does insert into sub-value id = 0
void ModelSqlBuilder::doAddScalarWorksetParameter(
    int i_setId,
    const string & i_name, 
    const string & i_dbTableName, 
    const vector<ParamTblInfo>::const_iterator & i_paramInfo,
    int i_subId,
    const string & i_value
    )
{
    // scalar parameter expected: check number of dimensions
    if (i_paramInfo->colVec.size() != 0) throw DbException(LT("invalid number of dimensions for scalar parameter: %s"), i_name.c_str());

    // make sql to insert parameter value
    // if parameter value is string type then it must be sql-quoted
    //  INSERT INTO StartingSeed_w20120819 (set_id, sub_id, param_value) VALUES (1234, 0, 0.014)
    dbExec->update(
        "INSERT INTO " + i_dbTableName + " (set_id, sub_id, param_value) VALUES (" + 
        to_string(i_setId) + ", " +
        to_string(i_subId) + ", " +
        (i_paramInfo->valueTypeIt->isString() ? toQuoted(i_value) : i_value) +
        ")");
}

// append parameter values to sql script for new working set creation 
// it does insert into sub-value id = 0
void ModelSqlBuilder::addWorksetParameter(
    const MetaModelHolder & i_metaRows, const MetaSetLangHolder & i_metaSet, const string & i_name, int i_subId, const list<string> & i_valueLst
    )
{
    // check parameters
    if (!isSqlite) throw DbException(LT("cannot create workset without model SQLite database."));
    if (i_name.empty()) throw DbException(LT("invalid (empty) input parameter name"));

    // find parameter info
    auto paramRow = std::find_if(
        i_metaRows.paramDic.cbegin(),
        i_metaRows.paramDic.cend(),
        [i_name](const ParamDicRow & i_row) -> bool { return i_row.paramName == i_name; }
    );
    if (paramRow == i_metaRows.paramDic.cend()) throw DbException(LT("parameter not found in parameters dictionary: %s"), i_name.c_str());

    auto paramInfo = std::find_if(
        paramInfoVec.begin(),
        paramInfoVec.end(),
        [i_name](const ParamTblInfo & i_info) -> bool { return i_info.name == i_name; }
    );
    if (paramInfo == paramInfoVec.cend()) throw DbException(LT("parameter not found in parameters dictionary: %s"), i_name.c_str());

    int colCount = (int)paramInfo->colVec.size();

    // if this is scalar parameter then use simplified version of sql
    if (colCount <= 0) {
        doAddScalarWorksetParameter(i_metaSet.worksetRow.setId, i_name, paramRow->dbSetTable, paramInfo, i_subId, i_valueLst.front());
        paramInfo->isAdded = true;
        return;                     // done with this scalar parameter
    }

    // if parameter value is string type then it must be sql-quoted
    bool isQuote = paramInfo->valueTypeIt->isString();

    // get dimensions list
    int mId = i_metaRows.modelDic.modelId;
    const ParamDimsRow pdRow(mId, paramInfo->id, 0);
    auto dimRange = std::equal_range(
        i_metaRows.paramDims.cbegin(), 
        i_metaRows.paramDims.cend(), 
        pdRow,
        [](const ParamDimsRow & i_left, const ParamDimsRow & i_right) -> bool { 
            return
                i_left.modelId < i_right.modelId || 
                (i_left.modelId == i_right.modelId && i_left.paramId < i_right.paramId);
        }
        );
    if ((colCount > 0 && dimRange.first == i_metaRows.paramDims.cend()) || colCount != (int)(dimRange.second - dimRange.first))
        throw DbException(LT("invalid parameter rank or dimensions not found for parameter: %s"), i_name.c_str());

    // get dimensions type and size, calculate total size
    size_t totalSize = 1;

    struct PDim {
        vector<ParamDimsRow>::const_iterator dimIt;
        int size = 0;
        vector<TypeDicRow>::const_iterator typeIt;
        vector<TypeEnumLstRow>::const_iterator beginEnumIt;
        vector<TypeEnumLstRow>::const_iterator endEnumIt;
    };
    vector<PDim> dims;

    for (vector<ParamDimsRow>::const_iterator dRow = dimRange.first; dRow != dimRange.second; dRow++) {

        PDim d = { dRow, 0, i_metaRows.typeDic.cend(), i_metaRows.typeEnum.cend(), i_metaRows.typeEnum.cbegin() };

        // find dimension type
        TypeDicRow tRow(mId, dRow->typeId);
        d.typeIt = std::lower_bound(
            i_metaRows.typeDic.cbegin(), i_metaRows.typeDic.cend(), tRow, TypeDicRow::isKeyLess
        );
        if (d.typeIt == i_metaRows.typeDic.cend()) throw DbException(LT("type not found for dimension %s, parameter: %s"), dRow->name.c_str(), i_name.c_str());
        if (d.typeIt->isBuiltIn()) throw DbException(LT("invalid (built-in) type of dimension %s, parameter: %s"), dRow->name.c_str(), i_name.c_str());

        // find dimension size as number of enums in type_enum_lst table, if type is not simple type
        TypeEnumLstRow eRow(mId, d.typeIt->typeId, 0);
        auto enumRange = std::equal_range(
            i_metaRows.typeEnum.cbegin(),
            i_metaRows.typeEnum.cend(),
            eRow,
            [](const TypeEnumLstRow & i_left, const TypeEnumLstRow & i_right) -> bool {
                return
                    i_left.modelId < i_right.modelId ||
                    (i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId);
            }
        );
        if (enumRange.first == i_metaRows.typeEnum.cend() || enumRange.first == enumRange.second)
            throw DbException(LT("invalid dimension size (no enums found), dimension: %s of parameter: %s"), dRow->name.c_str(), i_name.c_str());

        d.size = (int)(enumRange.second - enumRange.first);
        d.beginEnumIt = enumRange.first;
        d.endEnumIt = enumRange.second;

        // append to dimension list and calculate total parameter size
        dims.push_back(d);
        if (d.size > 0) totalSize *= d.size;
    }

    if (totalSize <= 0) throw DbException(LT("invalid size of the parameter: %s"), i_name.c_str());
    if (totalSize != i_valueLst.size()) throw DbException(LT("invalid value array size: %zd, expected: %zd for parameter: %s"), i_valueLst.size(), totalSize, i_name.c_str());

    // make sql to insert parameter dimesion enums and parameter value
    // if parameter value is string type then it must be sql-quoted
    // INSERT INTO ageSex_w20120817 (set_id, sub_id, dim0, dim1, param_value) VALUES (1234, 0, 1, 2, 0.014)
    {
        // storage for dimension enums index
        unique_ptr<int[]> cellArrUptr(new int[colCount]);
        int * cellArr = cellArrUptr.get();

        for (int k = 0; k < colCount; k++) {
            cellArr[k] = 0;     // initial enumindex is zero
        }

        // make constant portion of insert
        string insertPrefix = "INSERT INTO " + paramRow->dbSetTable + " (set_id, sub_id, ";

        for (const string & cn : paramInfo->colVec) {
            insertPrefix += cn + ", ";
        }
        insertPrefix += "param_value) VALUES (" + to_string(i_metaSet.worksetRow.setId) + ", " + to_string(i_subId) + ", ";

        // loop through all enums for each dimension and write sql inserts
        list<string>::const_iterator valueIt = i_valueLst.cbegin();
        string sql;
        sql.reserve(OM_STR_DB_MAX);
        char cellBuf[OM_STR_DB_MAX];

        for (size_t cellOffset = 0; cellOffset < totalSize; cellOffset++) {

            sql.clear();
            sql += insertPrefix;

            // write dimension enum_id items
            for (int k = 0; k < colCount; k++) {
                snprintf(cellBuf, OM_STR_DB_MAX, "%d, ", dims[k].beginEnumIt[cellArr[k]].enumId);
                sql += cellBuf;
            }

            // validate and write parameter value
            if (isQuote) {
                sql += toQuoted(*valueIt);
            }
            else {
                if (valueIt->empty()) throw DbException(LT("invalid (empty) parameter value, parameter: %s"), i_name.c_str());
                sql += *valueIt;
            }
            sql += ")";

            // do insert
            dbExec->update(sql);

            // get next cell indices
            for (int nDim = colCount - 1; nDim >= 0; nDim--) {
                if (nDim > 0 && cellArr[nDim] >= dims[nDim].size - 1) {
                    cellArr[nDim] = 0;
                }
                else {
                    cellArr[nDim]++;
                    break;
                }
            }
            if (cellOffset + 1 < totalSize && colCount > 0 && cellArr[0] >= dims[0].size) throw DbException(LT("invalid value array size, parameter: %s"), i_name.c_str());

            valueIt++;     // next value
        }
    }

    paramInfo->isAdded = true;  // done with parameter insert
}

// if type is a range type then return lower bound which should be the first enum id
static int firstRangeEnumId(const MetaModelHolder & i_metaRows, int i_typeId, const char * dName, const char * tName = "")
{
    // find type
    int mId = i_metaRows.modelDic.modelId;

    TypeDicRow tRow(mId, i_typeId);
    vector<TypeDicRow>::const_iterator typeIt = std::lower_bound(
        i_metaRows.typeDic.cbegin(), i_metaRows.typeDic.cend(), tRow, TypeDicRow::isKeyLess
    );
    if (typeIt == i_metaRows.typeDic.cend()) throw DbException(LT("type not found: %s %s"), dName, tName);

    // check if this is a range type
    if (typeIt->dicId != 3) return 0;

    vector<TypeEnumLstRow>::const_iterator eIt = find_if(
        i_metaRows.typeEnum.cbegin(),
        i_metaRows.typeEnum.cend(),
        [mId, i_typeId](const TypeEnumLstRow & i_row) -> bool
        { return i_row.modelId == mId && i_row.typeId == i_typeId; }
    );
    if (eIt == i_metaRows.typeEnum.cend()) throw DbException(LT("type enums not found: %s %s"), dName, tName);

    return eIt->enumId;
}

// write sql script to create backward compatibility views
void ModelSqlBuilder::buildCompatibilityViews(const MetaModelHolder & i_metaRows) const
{
    try {
        // write sql script to create views for each provider
        vector<int> lbVec;

        for (const string & providerName : dbProviderLst) {

            // put descriptive header
            ModelSqlWriter wr(outputDir + i_metaRows.modelDic.name + "_optional_views_" + providerName + ".sql");

            wr.outFs <<
                "--\n" <<
                "-- compatibility views for model: " << i_metaRows.modelDic.name << '\n' <<
                "-- model digest:   " << i_metaRows.modelDic.digest << '\n' <<
                "-- script created: " << toDateTimeString(theLog->timeStamp()) << '\n' <<
                "--\n" <<
                "-- Dear user:\n" <<
                "--   this part of database is optional and NOT used by openM++\n" <<
                "--   if you want it for any reason please enjoy else just ignore it\n" <<
                "-- Or other words:\n" <<
                "--   if you don't know what this is then you don't need it\n" <<
                "--\n\n";
            wr.throwOnFail();

            // input parameters compatibility views
            wr.write(
                "--\n" \
                "-- input parameters compatibility views\n" \
                "--\n"
            );
            for (size_t k = 0; k < i_metaRows.paramDic.size(); k++) {

                // for each dimension type of range find range type lower bound
                lbVec.clear();

                for (size_t j = 0; j < paramInfoVec[k].typeIdVec.size(); j++) {
                    lbVec.push_back(
                        firstRangeEnumId(i_metaRows, paramInfoVec[k].typeIdVec[j], paramInfoVec[k].dimVec[j].c_str(), i_metaRows.paramDic[k].paramName.c_str())
                    );
                }

                // lower bound for parameter value if parameter type of range
                int lbVal = firstRangeEnumId(i_metaRows, paramInfoVec[k].valueTypeIt->typeId, paramInfoVec[k].valueTypeIt->name.c_str());

                paramCompatibilityView(
                    providerName, i_metaRows.modelDic, i_metaRows.paramDic[k].paramName, i_metaRows.paramDic[k].dbRunTable, paramInfoVec[k].colVec, lbVec, lbVal, wr
                );
            }
            wr.write("\n");

            // output tables compatibility views
            wr.write(
                "--\n" \
                "-- output tables compatibility views\n" \
                "--\n"
            );
            for (size_t k = 0; k < i_metaRows.tableDic.size(); k++) {

                // for each dimension type of range find range type lower bound
                lbVec.clear();

                for (size_t j = 0; j < outInfoVec[k].typeIdVec.size(); j++) {
                    lbVec.push_back(
                        firstRangeEnumId(i_metaRows, outInfoVec[k].typeIdVec[j], outInfoVec[k].dimVec[j].c_str(), i_metaRows.tableDic[k].tableName.c_str())
                    );
                }

                outputCompatibilityView(
                    providerName, i_metaRows.modelDic, i_metaRows.tableDic[k].tableName, i_metaRows.tableDic[k].dbExprTable, outInfoVec[k].colVec, lbVec, wr
                );
            }
            wr.write("\n");

        }

        // write sql script to drop compatibility views
        ModelSqlWriter wr(outputDir + i_metaRows.modelDic.name + "_drop_optional_views.sql");
        wr.outFs <<
            "--\n" <<
            "-- drop compatibility views for model: " << i_metaRows.modelDic.name << '\n' <<
            "-- model digest:   " << i_metaRows.modelDic.digest << '\n' <<
            "-- script created: " << toDateTimeString(theLog->timeStamp()) << '\n' <<
            "--\n" <<
            "-- Dear user:\n" <<
            "--   this part of database is optional and NOT used by openM++\n" <<
            "--   if you want it for any reason please enjoy else just ignore it\n" <<
            "-- Or other words:\n" <<
            "--   if you don't know what is this then you don't need it\n" <<
            "--\n\n";
        wr.throwOnFail();

        // drop input parameters compatibility views
        wr.write(
            "--\n" \
            "-- drop input parameters compatibility views\n" \
            "--\n"
            );
        for (const ParamTblInfo & tblInfo : paramInfoVec) {
            wr.writeLine("DROP VIEW " + tblInfo.name + ";");
        }
        wr.write("\n");

        // drop output tables compatibility views
        wr.write(
            "--\n" \
            "-- drop output tables compatibility views\n" \
            "--\n"
            );
        for (const OutTblInfo & tblInfo : outInfoVec) {
            wr.writeLine("DROP VIEW " + tblInfo.name + ";");
        }
        wr.write("\n");
    }
    catch (HelperException & ex) {
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

// create table sql for parameter run or workset:
// CREATE TABLE ageSex_p20120817
// (
//  run_id      INT      NOT NULL,  -- set_id for workset parameter
//  sub_id      SMALLINT NOT NULL,
//  dim0        INT      NOT NULL,
//  dim1        INT      NOT NULL,
//  param_value FLOAT    NOT NULL,  -- it can be nullable for extended parameter
//  PRIMARY KEY (run_id, sub_id, dim0, dim1)  -- set_id for workset parameter
// );
const void ModelSqlBuilder::paramCreateTable(
    const string & i_sqlProvider, const string & i_dbTableName, const string & i_runSetId, const ParamTblInfo & i_tblInfo, ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "(" + 
        i_runSetId + " INT NOT NULL," +
        " sub_id SMALLINT NOT NULL, ";

    for (const string & cn : i_tblInfo.colVec) {
        sqlBody += cn + " INT NOT NULL, ";
    }

    sqlBody +=
        "param_value " + IDbExec::valueDbType(i_sqlProvider, i_tblInfo.valueTypeIt->name, i_tblInfo.valueTypeIt->typeId) + (i_tblInfo.isNullable ? " NULL" : " NOT NULL") + "," +
        " PRIMARY KEY (" + i_runSetId + ", sub_id";

    for (const string & cn : i_tblInfo.colVec) {
        sqlBody += ", " + cn;
    }
    sqlBody += "));";

    io_wr.outFs << IDbExec::makeSqlCreateTableIfNotExist(i_sqlProvider, i_dbTableName, sqlBody) << "\n";
}

// create table sql for accumulator table:
// CREATE TABLE salarySex_a20120820
// (
//  run_id    INT      NOT NULL,
//  acc_id    SMALLINT NOT NULL,
//  sub_id    SMALLINT NOT NULL,
//  dim0      INT      NOT NULL,
//  dim1      INT      NOT NULL,
//  acc_value FLOAT    NULL,
//  PRIMARY KEY (run_id, acc_id, sub_id, dim0, dim1)
// );
const void ModelSqlBuilder::accCreateTable(
    const string & i_sqlProvider, const string & i_dbTableName, const OutTblInfo & i_tblInfo, ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "(run_id INT NOT NULL, " \
        "acc_id SMALLINT NULL, " \
        "sub_id SMALLINT NOT NULL, ";

    for (const string & cn : i_tblInfo.colVec) {
        sqlBody += cn + " INT NOT NULL, ";
    }

    sqlBody += "acc_value FLOAT NULL, " \
        "PRIMARY KEY (run_id, acc_id, sub_id";

    for (const string & cn : i_tblInfo.colVec) {
        sqlBody += ", " + cn;
    }
    sqlBody += "));";

    io_wr.outFs << IDbExec::makeSqlCreateTableIfNotExist(i_sqlProvider, i_dbTableName, sqlBody) << "\n";
}

// create table sql for value table:
// CREATE TABLE salarySex_v20120820
// (
//  run_id     INT      NOT NULL,
//  expr_id    SMALLINT NOT NULL,
//  dim0       INT      NOT NULL,
//  dim1       INT      NOT NULL,
//  expr_value FLOAT    NULL,
//  PRIMARY KEY (run_id, expr_id, dim0, dim1)
// );
const void ModelSqlBuilder::valueCreateTable(
    const string & i_sqlProvider, const string & i_dbTableName, const OutTblInfo & i_tblInfo, ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "(run_id INT NOT NULL, " \
        "expr_id SMALLINT NULL,";

    for (const string & cn : i_tblInfo.colVec) {
        sqlBody += cn + " INT NOT NULL, ";
    }

    sqlBody += " expr_value FLOAT NULL," \
        " PRIMARY KEY (run_id, expr_id";

    for (const string & cn : i_tblInfo.colVec) {
        sqlBody += ", " + cn;
    }
    sqlBody += "));";

    io_wr.outFs << IDbExec::makeSqlCreateTableIfNotExist(i_sqlProvider, i_dbTableName, sqlBody) << "\n";
}

// create sql for all accumulators view:
// 
// CREATE VIEW IF NOT EXISTS T04_FertilityRatesByAgeGroup_d10612268
// AS
// WITH va1 AS
// (
//   SELECT
//     run_id, sub_id, dim0, dim1, acc_value
//   FROM T04_FertilityRatesByAgeGroup_a10612268
//   WHERE acc_id = 1
// )
// SELECT
//   A.run_id, A.sub_id,
//   A.dim0       AS "Year", 
//   A.dim1       AS "Age Group",
//   A.acc_value  AS "acc0",
//   A1.acc_value AS "acc1",
//   (
//     A.acc_value / CASE WHEN ABS(A1.acc_value) > 1.0e-37 THEN A1.acc_value ELSE NULL END
//   ) AS "Expr0"
// FROM T04_FertilityRatesByAgeGroup_a10612268 A
// INNER JOIN va1 A1 ON (A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1)
// WHERE A.acc_id = 0;
// 
const void ModelSqlBuilder::accAllCreateView(
    const string & i_sqlProvider, 
    const string & i_viewName,
    const OutTblInfo & i_tblInfo,
    const string & i_accTablName,
    const vector<TableAccRow> & i_allAcc, 
    ModelSqlWriter & io_wr
    ) const
{
    // find table_acc rows
    vector <vector<TableAccRow>::const_iterator> accVec(i_tblInfo.accIdVec.size());

    for (size_t nAcc = 0; nAcc < i_tblInfo.accIdVec.size(); nAcc++) {

        vector<TableAccRow>::const_iterator accIt = TableAccRow::byKey(i_tblInfo.modelId, i_tblInfo.id, i_tblInfo.accIdVec[nAcc], i_allAcc);
        if (accIt == i_allAcc.cend())
            throw DbException(LT("output table %s accumulator not found, id: %d"), i_tblInfo.name.c_str(), i_tblInfo.accIdVec[nAcc]);

        accVec[nAcc] = accIt;
    }

    // start view body: 
    // WITH clause for each native accumulators, except first
    string sqlBody;

    for (size_t nAcc = 1; nAcc < accVec.size(); nAcc++) {
        if (!accVec[nAcc]->isDerived) {
            sqlBody += ((nAcc == 1) ? "WITH va" : ", va") + to_string(nAcc) + " AS (" + accVec[nAcc]->accSql + ")";
        }
    }

    // start main SELECT run id, sub id, dimensions and first accumulator value: A.acc_value
    sqlBody += " SELECT A.run_id, A.sub_id";

    for (size_t k = 0; k < i_tblInfo.colVec.size(); k++) {
        sqlBody += ", A." + i_tblInfo.colVec[k] + " AS \"" + i_tblInfo.dimVec[k] + "\"";
    }
    sqlBody += ", A.acc_value AS \"" + accVec[0]->name + "\"";

    // append the rest of native accumulators as: , A1.acc_value....
    // append derived accumulators sql expressions
    // first accumulator is always native
    for (size_t nAcc = 1; nAcc < accVec.size(); nAcc++) {
        if (!accVec[nAcc]->isDerived) {
            sqlBody += ", A" + to_string(nAcc) + ".acc_value AS \"" + accVec[nAcc]->name + "\"";
        }
        else {
            sqlBody += ", ( " + accVec[nAcc]->accSql + " ) AS \"" + accVec[nAcc]->name + "\"";
        }
    }

    // FROM main accumulator table WHERE acc_id = first accumulator id
    sqlBody += " FROM " + i_accTablName + " A";

    // all other native accumulators joined to the first by run id, sub id and dimensions
    // INNER JOIN va1 A1 ON (A1.run_id = A.run_id AND A1.sub_id = A.sub_id AND A1.dim0 = A.dim0 AND A1.dim1 = A.dim1)
    for (size_t nAcc = 1; nAcc < accVec.size(); nAcc++) {
        if (accVec[nAcc]->isDerived) continue;

        string aliasDot = "A" + to_string(nAcc) + ".";
        sqlBody += 
            " INNER JOIN va" + to_string(nAcc) + " A" + to_string(nAcc) + " ON (" + aliasDot + "run_id = A.run_id AND " + aliasDot + "sub_id = A.sub_id";
        
        for (const string & cn : i_tblInfo.colVec) {
            sqlBody += " AND " + aliasDot + cn + " = A." + cn;
        }
        sqlBody += ")";
    }

    // WHERE acc_id = first accumulator id
    sqlBody += " WHERE A.acc_id = " + to_string(accVec[0]->accId) + ";";

    // make create view as select
    io_wr.outFs << IDbExec::makeSqlCreateViewReplace(i_sqlProvider, i_viewName, sqlBody) << "\n";
}

// write body of create view sql for parameter compatibility view:
// CREATE VIEW ageSex AS
// SELECT
//  S.dim0        AS "Dim0",
//  S.dim1        AS "Dim1",
//  S.param_value AS "Value"
// FROM ageSex_p20120817 S
// WHERE S.sub_id = 0
// AND S.run_id =
// (
//  SELECT MIN(RL.run_id)
//  FROM run_lst RL
//  INNER JOIN model_dic M ON (M.model_id = RL.model_id)
//  WHERE M.model_digest = '1234abcd'
// );
const void ModelSqlBuilder::paramCompatibilityView(
    const string & i_sqlProvider, 
    const ModelDicRow & i_modelRow, 
    const string & i_viewName, 
    const string & i_srcTableName, 
    const vector<string> & i_colNames,
    const vector<int> & i_dimsLowerBound,
    int i_valueLowerBound,
    ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "SELECT";

    for (size_t k = 0; k < i_colNames.size(); k++) {
        sqlBody +=
            (i_dimsLowerBound[k] == 0 ? " S." + i_colNames[k] : " (S." + i_colNames[k] + " - " + to_string(i_dimsLowerBound[k]) + ")") +
            " AS \"Dim" + to_string(k) + "\",";
    }
    sqlBody += (i_valueLowerBound == 0 ? " S.param_value" : " (S.param_value - " + to_string(i_valueLowerBound) + ")") +
        " AS \"Value\"" ;

    // from sub-value table where run id is first run of that model
    sqlBody +=
        " FROM " + i_srcTableName + " S" +
        " WHERE S.sub_id = 0" \
        " AND S.run_id = (" \
        " SELECT MIN(RL.run_id)" \
        " FROM run_lst RL" \
        " INNER JOIN model_dic M ON (M.model_id = RL.model_id)" \
        " WHERE M.model_digest = " + toQuoted(i_modelRow.digest) + ");";

    io_wr.outFs << IDbExec::makeSqlCreateViewReplace(i_sqlProvider, i_viewName, sqlBody) << "\n";
}

// return body of create view sql for output table compatibility view:
// CREATE VIEW salarySex AS
// SELECT
//   S.dim0       AS "Dim0",
//   S.dim1       AS "Dim1",
//   S.expr_id    AS "Dim2",
//   S.expr_value AS "Value"
// FROM salarySex_v98765432 S
// WHERE S.run_id =
// (
//  SELECT MIN(RL.run_id)
//  FROM run_lst RL
//  INNER JOIN model_dic M ON (M.model_id = RL.model_id)
//  WHERE M.model_digest = '1234abcd'
// );
const void ModelSqlBuilder::outputCompatibilityView(
    const string & i_sqlProvider, 
    const ModelDicRow & i_modelRow, 
    const string & i_viewName, 
    const string & i_srcTableName, 
    const vector<string> & i_colNames, 
    const vector<int> & i_dimsLowerBound,
    ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "SELECT";

    for (size_t k = 0; k < i_colNames.size(); k++) {
        sqlBody +=
            (i_dimsLowerBound[k] == 0 ? " S." + i_colNames[k] : " (S." + i_colNames[k] + " - " + to_string(i_dimsLowerBound[k]) + ")") +
            " AS \"Dim" + to_string(k) + "\",";
    }
    sqlBody +=
        " S.expr_id AS \"Dim" + to_string(i_colNames.size()) + "\"," \
        " S.expr_value AS \"Value\"";

    // from value table where run id is first run of that model
    sqlBody +=
        " FROM " + i_srcTableName + " S" \
        " WHERE S.run_id =" \
        " (" \
        " SELECT MIN(RL.run_id)" \
        " FROM run_lst RL" \
        " INNER JOIN model_dic M ON (M.model_id = RL.model_id)" \
        " WHERE M.model_digest = " + toQuoted(i_modelRow.digest) + ");";

    io_wr.outFs << IDbExec::makeSqlCreateViewReplace(i_sqlProvider, i_viewName, sqlBody) << "\n";
}

