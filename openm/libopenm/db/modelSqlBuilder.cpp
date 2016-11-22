// OpenM++ data library: class to produce new model sql from metadata
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "modelSqlBuilder.h"

using namespace openm;

// new model builder to create sql script specific to each db-provider
IModelBuilder * IModelBuilder::create(const string & i_providerNames, const string & i_sqlDir, const string & i_outputDir)
{
    return new ModelSqlBuilder(i_providerNames, i_sqlDir, i_outputDir);
}

// release builder resources
IModelBuilder::~IModelBuilder() throw() { }

// create new model builder and set db table name prefix and suffix rules
ModelSqlBuilder::ModelSqlBuilder(const string & i_providerNames, const string & i_sqlDir, const string & i_outputDir) :
    isCrc32Name(false),
    dbPrefixSize(0),
    dbSuffixSize(0),
    sqlDir(i_sqlDir),
    outputDir(i_outputDir),
    isSqlite(false)
{
    // parse and validate provider names
    dbProviderLst = IDbExec::parseListOfProviderNames(i_providerNames);
    if (dbProviderLst.empty()) throw DbException("invalid (empty) list db-provider names");

    isSqlite = dbProviderLst.cend() != std::find(dbProviderLst.cbegin(), dbProviderLst.cend(), SQLITE_DB_PROVIDER);

    // find smallest of max size of db table name
    int minSize = 256;
    for (const string providerName : dbProviderLst) {
        int nSize = IDbExec::maxDbTableNameSize(providerName);
        if (nSize < minSize) minSize = nSize;
    }

    // if max size of db table name is too short then use crc32(md5) digest
    // table name is: paramNameAsPrefix + _p + md5Suffix, for example: ageSex_p12345678
    // prefix based on parameter name or output table name
    // suffix is 32 chars of md5 or 8 chars of crc32
    // there is extra 2 chars: _p, _w, _v, _a in table name between prefix and suffix
    //
    // 2016-08-17: always use short crc32 name suffix (due to Oracle 30 limit)
    // isCrc32Name = minSize < 50;
    isCrc32Name = true;
    dbSuffixSize = isCrc32Name ? 8 : 32;
    dbPrefixSize = minSize - (2 + dbSuffixSize);

    if (dbPrefixSize < 2 || dbSuffixSize < 8)
        throw DbException("invalid db table name prefix size: %d or suffix size: %d", dbPrefixSize, dbSuffixSize);
 }

// validate metadata and return sql script to create new model from supplied metadata rows
void ModelSqlBuilder::build(MetaModelHolder & io_metaRows)
{
    try {
        // validate input rows: uniqueness and referential integrity
        prepare(io_metaRows);

        // collect info for db parameter tables, db subsample tables and value views
        setParamTableInfo(io_metaRows);
        setOutTableInfo(io_metaRows);

        // write sql script to create new model tables
        for (const string providerName : dbProviderLst) {
            buildCreateModelTables(
                providerName, 
                io_metaRows, 
                makeFilePath(outputDir.c_str(), (io_metaRows.modelDic.name + "_create_tables_" + providerName).c_str(), ".sql")
            );
        }

        // write sql script to drop model tables
        buildDropModelTables(
            io_metaRows, 
            makeFilePath(outputDir.c_str(), (io_metaRows.modelDic.name + "_drop_tables").c_str(), ".sql")
        );

        // create SQLite database and publish model: insert model metadata and create model tables
        if (isSqlite) {
            dbExec.reset(IDbExec::create(
                SQLITE_DB_PROVIDER,
                "Database=" + makeFilePath(outputDir.c_str(), io_metaRows.modelDic.name.c_str(), ".sqlite") + ";" +
                " Timeout=86400;" +
                " OpenMode=Create;" +
                " DeleteExisting=true;"
            ));

            // run script to create openM++ metadata tables and insert default values
            dbExec->runSqlScript(fileToUtf8(makeFilePath(sqlDir.c_str(), "create_db.sql").c_str()));

            // insert new model metadata into SQLite database
            createModel(dbExec.get(), io_metaRows);

            // run sql script to create new model tables
            dbExec->runSqlScript(
                fileToUtf8(makeFilePath(
                    outputDir.c_str(), (io_metaRows.modelDic.name + "_create_tables_" + SQLITE_DB_PROVIDER).c_str(), ".sql"
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

    // model master row: model_dic
    ModelInsertSql::insertModelDic(i_dbExec, io_metaRows.modelDic);

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
    for (GroupLstRow & row : io_metaRows.groupLst) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (GroupTxtLangRow & row : io_metaRows.groupTxt) {
        row.modelId = io_metaRows.modelDic.modelId;
    }
    for (GroupPcRow & row : io_metaRows.groupPc) {
        row.modelId = io_metaRows.modelDic.modelId;
    }

    // TODO: 
    // when lanuage list and word list implemeneted in omc then insert languages instead of code below
    //
    // make language code to id map
    unique_ptr<ILangLstTable> langTbl(std::move(ILangLstTable::create(i_dbExec)));
    map<string, int> langMap;

    for (int k = 0; k < langTbl->rowCount(); k++) {
        const LangLstRow * langRow = langTbl->byIndex(k);
        langMap[langRow->code] = langRow->langId;
    }

    // update language list id's with actual database values
    for (LangLstRow & row : io_metaRows.langLst) {
        map<string, int>::const_iterator it = langMap.find(row.code);
        if (it == langMap.cend()) throw DbException("invalid language code: %s", row.code.c_str());
        row.langId = it->second;
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
            if (it == typeIdMap.cend()) throw DbException("invalid type id: %d", typeId);
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
            if (it == typeIdMap.cend()) throw DbException("invalid type id: %d", typeId);
            typeHid = it->second;
        }

        ModelInsertSql::insertTypeEnumText(i_dbExec, langMap, typeHid, row);
    }

    // model input parameter rows: parameter_dic and model_parameter_dic
    for (ParamDicRow & row : io_metaRows.paramDic) {
        ModelInsertSql::insertParamDic(i_dbExec, typeIdMap, row);
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
 
    // insert model metadata compeleted
    i_dbExec->commit();
}

// write sql script to create new model tables
void ModelSqlBuilder::buildCreateModelTables(const string & i_sqlProvider, const MetaModelHolder & i_metaRows,  const string & i_filePath) const
{
    // script header
    ModelSqlWriter wr(i_filePath);
    wr.outFs <<
        "--\n" <<
        "-- create model tables: " << i_metaRows.modelDic.name << '\n' <<
        "-- model digest:        " << i_metaRows.modelDic.digest << '\n' <<
        "-- script created:      " << toDateTimeString(theLog->timeStampSuffix()) << '\n' <<
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

    // create tables for model output tables
    wr.write(
        "--\n" \
        "-- create model output tables\n" \
        "--\n"
        );
    for (size_t k = 0; k < i_metaRows.tableDic.size(); k++) {
        accCreateTable(i_sqlProvider, i_metaRows.tableDic[k].dbAccTable, outInfoVec[k], wr);
        valueCreateTable(i_sqlProvider, i_metaRows.tableDic[k].dbExprTable, outInfoVec[k], wr);
    }
    wr.write("\n");
}

// write sql script to drop model tables
void ModelSqlBuilder::buildDropModelTables(const MetaModelHolder & i_metaRows,  const string & i_filePath) const
{
    // script header
    ModelSqlWriter wr(i_filePath);
    wr.outFs <<
        "--\n"
        "-- drop model tables: " << i_metaRows.modelDic.name << '\n' <<
        "-- model digest:      " << i_metaRows.modelDic.digest << '\n' <<
        "-- script created:    " << toDateTimeString(theLog->timeStampSuffix()) << '\n' <<
        "--\n" <<
        "-- DROP MODEL TABLES" << '\n' <<
        "-- DO NOT USE THIS SQL UNLESS YOU HAVE TO" << '\n' <<
        "-- IT WILL DELETE ALL MODEL DATA" << '\n' <<
        "--\n\n";
    wr.throwOnFail();

    // drop model output tables
    wr.write(
        "--\n" \
        "-- drop model output tables\n" \
        "--\n"
        );
    for (const TableDicRow & tblRow : i_metaRows.tableDic) {
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
        if (!isSqlite) throw DbException("cannot create workset without model SQLite database.");

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
        if (!isSqlite) throw DbException("cannot create workset without model SQLite database.");

        // validate workset parameters: all parameters must be added to workset
        for (const ParamTblInfo & tblInfo : paramInfoVec) {
            if (!tblInfo.isAdded)
                throw DbException("workset must include all model parameters, missing: %d: %s", tblInfo.id, tblInfo.name.c_str());
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
    const MetaModelHolder & i_metaRows, const MetaSetLangHolder & i_metaSet, const string & i_name, const string & i_value
    )
{
    // check parameters
    if (!isSqlite) throw DbException("cannot create workset without model SQLite database.");
    if (i_name.empty()) throw DbException("invalid (empty) input parameter name");

    // find parameter info
    auto paramRow = std::find_if(
        i_metaRows.paramDic.cbegin(),
        i_metaRows.paramDic.cend(),
        [i_name](const ParamDicRow & i_row) -> bool { return i_row.paramName == i_name; }
    );
    if (paramRow == i_metaRows.paramDic.cend()) throw DbException("parameter not found in parameters dictionary: %s", i_name.c_str());

    auto paramInfo = std::find_if(
        paramInfoVec.begin(), 
        paramInfoVec.end(),
        [i_name](const ParamTblInfo & i_info) -> bool { return i_info.name == i_name; }
    );
    if (paramInfo == paramInfoVec.cend()) throw DbException("parameter not found in parameters dictionary: %s", i_name.c_str());

    // create sql to insert value of scalar paarameter
    doAddScalarWorksetParameter(i_metaSet.worksetRow.setId, i_name, paramRow->dbSetTable, paramInfo, i_value);

    paramInfo->isAdded = true;  // done
}

// impelementation of append scalar parameter value to sql script
void ModelSqlBuilder::doAddScalarWorksetParameter(
    int i_setId,
    const string & i_name, 
    const string & i_dbTableName, 
    const vector<ParamTblInfo>::const_iterator & i_paramInfo, 
    const string & i_value
    )
{
    // scalar parameter expected: check number of dimensions
    if (i_paramInfo->dimNameVec.size() != 0) throw DbException("invalid number of dimensions for scalar parameter: %s", i_name.c_str());

    // make sql to insert parameter value
    // if parameter value is string type then it must be sql-quoted
    //  INSERT INTO StartingSeed_w20120819 (set_id, param_value) VALUES (1234, 0.014)
    dbExec->update(
        "INSERT INTO " + i_dbTableName + " (set_id, param_value) VALUES (" + 
        to_string(i_setId) + ", " +
        (i_paramInfo->valueTypeIt->isString() ? toQuoted(i_value) : i_value) +
        ")");
}

// append parameter values to sql script for new working set creation 
void ModelSqlBuilder::addWorksetParameter(
    const MetaModelHolder & i_metaRows, const MetaSetLangHolder & i_metaSet, const string & i_name, const list<string> & i_valueLst
    )
{
    // check parameters
    if (!isSqlite) throw DbException("cannot create workset without model SQLite database.");
    if (i_name.empty()) throw DbException("invalid (empty) input parameter name");

    // find parameter info
    auto paramRow = std::find_if(
        i_metaRows.paramDic.cbegin(),
        i_metaRows.paramDic.cend(),
        [i_name](const ParamDicRow & i_row) -> bool { return i_row.paramName == i_name; }
    );
    if (paramRow == i_metaRows.paramDic.cend()) throw DbException("parameter not found in parameters dictionary: %s", i_name.c_str());

    auto paramInfo = std::find_if(
        paramInfoVec.begin(),
        paramInfoVec.end(),
        [i_name](const ParamTblInfo & i_info) -> bool { return i_info.name == i_name; }
    );
    if (paramInfo == paramInfoVec.cend()) throw DbException("parameter not found in parameters dictionary: %s", i_name.c_str());

    int dimCount = (int)paramInfo->dimNameVec.size();

    // if this is scalar parameter then use simplified version of sql
    if (dimCount <= 0) {
        doAddScalarWorksetParameter(i_metaSet.worksetRow.setId, i_name, paramRow->dbSetTable, paramInfo, i_valueLst.front());
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
    if ((dimCount > 0 && dimRange.first == i_metaRows.paramDims.cend()) || dimCount != (int)(dimRange.second - dimRange.first))
        throw DbException("invalid parameter rank or dimensions not found for parameter: %s", i_name.c_str());

    // get dimensions type and size, calculate total size
    size_t totalSize = 1;
    vector<int> dimSizeVec;

    for (vector<ParamDimsRow>::const_iterator dRow = dimRange.first; dRow != dimRange.second; dRow++) {

        // find dimension type
        TypeDicRow tRow(mId, dRow->typeId);
        auto dimTypeRow = std::lower_bound(
            i_metaRows.typeDic.cbegin(), i_metaRows.typeDic.cend(), tRow, TypeDicRow::isKeyLess
            );
        if (dimTypeRow == i_metaRows.typeDic.cend()) throw DbException("type not found for dimension %s of parameter: %s", dRow->name.c_str(), i_name.c_str());

        // find dimension size as number of enums in type_enum_lst table, if type is not simple type
        int dimSize = 0;
        if (!dimTypeRow->isBuiltIn()) {

            TypeEnumLstRow eRow(mId, dimTypeRow->typeId, 0);
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
            if (enumRange.first == i_metaRows.typeEnum.cend())
                throw DbException("invalid dimension size (no enums found), dimension: %s of parameter: %s", dRow->name.c_str(), i_name.c_str());

            dimSize = (int)(enumRange.second - enumRange.first);
        }

        // store dimension name, size and calculate total parameter size
        dimSizeVec.push_back(dimSize);
        if (dimSize > 0) totalSize *= dimSize;
    }

    if (totalSize <= 0) throw DbException("invalid size of the parameter: %s", i_name.c_str());
    if (totalSize != i_valueLst.size()) throw DbException("invalid value array size: %ld, expected: %ld for parameter: %s", i_valueLst.size(), totalSize, i_name.c_str());

    // make sql to insert parameter dimesion enums and parameter value
    // if parameter value is string type then it must be sql-quoted
    // INSERT INTO ageSex_w20120817 (set_id, dim0, dim1, param_value) VALUES (1234, 1, 2, 0.014)
    {
        // storage for dimension enum_id items
        unique_ptr<int> cellArrUptr(new int[dimCount]);
        int * cellArr = cellArrUptr.get();

        for (int k = 0; k < dimCount; k++) {
            cellArr[k] = 0;     // initial enum_id is zero for all enums
        }

        // make constant portion of insert
        string insertPrefix = "INSERT INTO " + paramRow->dbSetTable + " (set_id, ";

        for (const string & dimName : paramInfo->dimNameVec) {
            insertPrefix += dimName + ", ";
        }
        insertPrefix += "param_value) VALUES (" + to_string(i_metaSet.worksetRow.setId) + ", ";

        // loop through all enums for each dimension and write sql inserts
        list<string>::const_iterator valueIt = i_valueLst.cbegin();
        string sql;
        sql.reserve(OM_STR_DB_MAX);
        char cellBuf[OM_STR_DB_MAX];

        for (size_t cellOffset = 0; cellOffset < totalSize; cellOffset++) {

            sql.clear();
            sql += insertPrefix;

            // write dimension enum_id items
            for (int k = 0; k < dimCount; k++) {
                snprintf(cellBuf, OM_STR_DB_MAX, "%d, ", cellArr[k]);
                sql += cellBuf;
            }

            // validate and write parameter value
            if (isQuote) {
                 sql += toQuoted(*valueIt);
            }
            else {
                if (valueIt->empty()) throw DbException("invalid (empty) parameter value, parameter: %s", i_name.c_str());
                sql += *valueIt;
            }
            sql += ")";

            // do insert
            dbExec->update(sql);

            // get next cell indices
            for (int nDim = dimCount - 1; nDim >= 0; nDim--) {
                if (nDim > 0 && cellArr[nDim] >= dimSizeVec[nDim] - 1) {
                    cellArr[nDim] = 0;
                }
                else {
                    cellArr[nDim]++;
                    break;
                }
            }
            if (cellOffset + 1 < totalSize && dimCount > 0 && cellArr[0] >= dimSizeVec[0]) throw DbException("invalid value array size, parameter: %s", i_name.c_str());

            valueIt++;     // next value
        }
    }

    paramInfo->isAdded = true;  // done with parameter insert
}

// write sql script to create backward compatibility views
void ModelSqlBuilder::buildCompatibilityViews(const MetaModelHolder & i_metaRows) const
{
    try {
        // write sql script to create views for each provider
        for (const string providerName : dbProviderLst) {

            // put descriptive header
            ModelSqlWriter wr(outputDir + i_metaRows.modelDic.name + "_optional_views_" + providerName + ".sql");

            wr.outFs <<
                "--\n" <<
                "-- compatibility views for model: " << i_metaRows.modelDic.name << '\n' <<
                "-- model digest:   " << i_metaRows.modelDic.digest << '\n' <<
                "-- script created: " << toDateTimeString(theLog->timeStampSuffix()) << '\n' <<
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
                paramCompatibilityView(
                    providerName, i_metaRows.modelDic, i_metaRows.paramDic[k].paramName, i_metaRows.paramDic[k].dbRunTable, paramInfoVec[k].dimNameVec, wr
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
                outputCompatibilityView(
                    providerName, i_metaRows.modelDic, i_metaRows.tableDic[k].tableName, i_metaRows.tableDic[k].dbExprTable, outInfoVec[k].dimNameVec, wr
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
            "-- script created: " << toDateTimeString(theLog->timeStampSuffix()) << '\n' <<
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

// create table sql for parameter run or workset:
// CREATE TABLE ageSex_p20120817
// (
//  run_id      INT   NOT NULL,  -- set_id for workset parameter
//  dim0        INT   NOT NULL,
//  dim1        INT   NOT NULL,
//  param_value FLOAT NOT NULL,
//  PRIMARY KEY (run_id, dim0, dim1)  -- set_id for workset parameter
// );
const void ModelSqlBuilder::paramCreateTable(
    const string & i_sqlProvider, const string & i_dbTableName, const string & i_runSetId, const ParamTblInfo & i_tblInfo, ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "(" + 
        i_runSetId + " INT NOT NULL, ";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += dimName + " INT NOT NULL, ";
    }

    sqlBody +=
        "param_value " + valueDbType(i_sqlProvider, i_tblInfo) + " NOT NULL, " +
        "PRIMARY KEY (" + i_runSetId;

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += ", " + dimName;
    }
    sqlBody += "));";

    io_wr.outFs << IDbExec::makeSqlCreateTableIfNotExist(i_sqlProvider, i_dbTableName, sqlBody) << "\n";
}

// return db type name by model type for specific db provider
string ModelSqlBuilder::valueDbType(const string & i_sqlProvider, const ParamTblInfo & i_tblInfo)
{
    // C++ ambiguous integral type
    // (in C/C++, the signedness of char is not specified)
    if (equalNoCase(i_tblInfo.valueTypeIt->name.c_str(), "char")) return "SMALLINT";

    // C++ signed integral types
    if (equalNoCase(i_tblInfo.valueTypeIt->name.c_str(), "schar")) return "SMALLINT";
    if (equalNoCase(i_tblInfo.valueTypeIt->name.c_str(), "short")) return "SMALLINT";
    if (equalNoCase(i_tblInfo.valueTypeIt->name.c_str(), "int")) return "INT";

    // C++ unsigned integral types
    if (equalNoCase(i_tblInfo.valueTypeIt->name.c_str(), "uchar")) return "SMALLINT";
    if (equalNoCase(i_tblInfo.valueTypeIt->name.c_str(), "ushort")) return "INT";

    // Changeable numeric types
    if (equalNoCase(i_tblInfo.valueTypeIt->name.c_str(), "integer")) return "INT";
    if (equalNoCase(i_tblInfo.valueTypeIt->name.c_str(), "counter")) return "INT";

    // C++ bool type
    if (i_tblInfo.valueTypeIt->isBool()) return "SMALLINT";

    // C++ int64 and uint64 types
    if (i_tblInfo.valueTypeIt->isBigInt()) return IDbExec::bigIntTypeName(i_sqlProvider);

    // C++ floating point types
    if (i_tblInfo.valueTypeIt->isFloat()) return IDbExec::floatTypeName(i_sqlProvider);

    // path to a file (a string)
    if (i_tblInfo.valueTypeIt->isString()) return IDbExec::textTypeName(i_sqlProvider, OM_PATH_MAX);

    // model specific types: it must be enum
    if (!i_tblInfo.valueTypeIt->isBuiltIn()) return "INT";

    throw DbException("invalid value type for parameter id: %d, db table name: %s", i_tblInfo.id, i_tblInfo.name.c_str());
}

// create table sql for accumulator table:
// CREATE TABLE salarySex_a20120820
// (
//  run_id    INT   NOT NULL,
//  acc_id    INT   NOT NULL,
//  sub_id    INT   NOT NULL,
//  dim0      INT   NOT NULL,
//  dim1      INT   NOT NULL,
//  acc_value FLOAT NULL,
//  PRIMARY KEY (run_id, acc_id, sub_id, dim0, dim1)
// );
const void ModelSqlBuilder::accCreateTable(
    const string & i_sqlProvider, const string & i_dbTableName, const OutTblInfo & i_tblInfo, ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "(run_id INT NOT NULL, " \
        "acc_id INT NOT NULL, " \
        "sub_id INT NOT NULL, ";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += dimName + " INT NOT NULL, ";
    }

    sqlBody += "acc_value FLOAT NULL, " \
        "PRIMARY KEY (run_id, acc_id, sub_id";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += ", " + dimName;
    }
    sqlBody += "));";

    io_wr.outFs << IDbExec::makeSqlCreateTableIfNotExist(i_sqlProvider, i_dbTableName, sqlBody) << "\n";
}

// create table sql for value table:
// CREATE TABLE salarySex_v20120820
// (
//  run_id     INT   NOT NULL,
//  expr_id    INT   NOT NULL,
//  dim0       INT   NOT NULL,
//  dim1       INT   NOT NULL,
//  expr_value FLOAT NULL,
//  PRIMARY KEY (run_id, expr_id, dim0, dim1)
// );
const void ModelSqlBuilder::valueCreateTable(
    const string & i_sqlProvider, const string & i_dbTableName, const OutTblInfo & i_tblInfo, ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "(run_id INT NOT NULL, " \
        "expr_id INT NOT NULL,";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += dimName + " INT NOT NULL, ";
    }

    sqlBody += " expr_value FLOAT NULL," \
        " PRIMARY KEY (run_id, expr_id";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += ", " + dimName;
    }
    sqlBody += "));";

    io_wr.outFs << IDbExec::makeSqlCreateTableIfNotExist(i_sqlProvider, i_dbTableName, sqlBody) << "\n";
}

// write body of create view sql for parameter compatibility view:
// CREATE VIEW ageSex AS
// SELECT
//  S.dim0        AS "Dim0",
//  S.dim1        AS "Dim1",
//  S.param_value AS "Value"
// FROM ageSex_p20120817 S
// WHERE S.run_id =
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
    const vector<string> & i_dimNames, 
    ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "SELECT";

    for (size_t k = 0; k < i_dimNames.size(); k++) {
        sqlBody += " S." + i_dimNames[k] + " AS \"Dim" + to_string(k) + "\",";
    }
    sqlBody += " S.param_value AS \"Value\"";

    // from subsample table where run id is first run of that model
    sqlBody +=
        " FROM " + i_srcTableName + " S" +
        " WHERE S.run_id = (" \
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
    const vector<string> & i_dimNames, 
    ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "SELECT";

    for (size_t k = 0; k < i_dimNames.size(); k++) {
        sqlBody += " S." + i_dimNames[k] + " AS \"Dim" + to_string(k) + "\",";
    }
    sqlBody +=
        " S.expr_id AS \"Dim" + to_string(i_dimNames.size()) + "\"," \
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

