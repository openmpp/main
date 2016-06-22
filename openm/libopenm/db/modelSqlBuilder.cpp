// OpenM++ data library: class to produce new model sql from metadata
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "modelSqlBuilder.h"

using namespace openm;

// new model builder to create sql script specific to each db-provider
IModelBuilder * IModelBuilder::create(const string & i_providerNames, const string & i_outputDir)
{
    return new ModelSqlBuilder(i_providerNames, i_outputDir);
}

// release builder resources
IModelBuilder::~IModelBuilder() throw() { }

// create new model builder and set db table name prefix and suffix rules
ModelSqlBuilder::ModelSqlBuilder(const string & i_providerNames, const string & i_outputDir) : 
    isCrc32Name(false),
    dbPrefixSize(0),
    dbSuffixSize(0),
    outputDir(i_outputDir),
    worksetFileIndex(0)
{
    // parse and validate provider names
    dbProviderLst = IDbExec::parseListOfProviderNames(i_providerNames);

    // find smallest of max size of db table name
    int minSize = 256;
    for (const string providerName : dbProviderLst) {
        int nSize = IDbExec::maxDbTableNameSize(providerName);
        if (nSize < minSize) minSize = nSize;
    }

    // if max size of db table name is too short then use crc32(md5) digest
    isCrc32Name = minSize < 50;
    dbSuffixSize = isCrc32Name ? 8 : 32;
    dbPrefixSize = minSize - (OM_DB_TABLE_TYPE_PREFIX_LEN + dbSuffixSize);

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

        // write sql script to insert new model metadata 
        // write sql script to create new model tables
        for (const string providerName : dbProviderLst) {
            buildCreateModel(providerName, io_metaRows, outputDir + io_metaRows.modelDic.name + "_1_create_model_" + providerName + ".sql");
            buildCreateModelTables(providerName, io_metaRows, outputDir + io_metaRows.modelDic.name + "_2_create_tables_" + providerName + ".sql");
        }

        // write sql script to drop model tables
        buildDropModelTables(io_metaRows, outputDir + io_metaRows.modelDic.name + "_drop_tables.sql");
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

// write sql script to insert new model metadata
void ModelSqlBuilder::buildCreateModel(const string & i_sqlProvider, const MetaModelHolder & i_metaRows, const string & i_filePath) const
{
    // start transaction and get new model id
    ModelSqlWriter wr(i_filePath);
    wr.outFs << 
        "--\n" <<
        "-- create new model: " << i_metaRows.modelDic.name << '\n' <<
        "-- model digest:     " << i_metaRows.modelDic.digest << '\n' <<
        "-- script created:   " << toDateTimeString(theLog->timeStampSuffix()) << '\n' <<
        "--\n\n";
    wr.throwOnFail();

    string sqlBeginTrx = IDbExec::makeSqlBeginTransaction(i_sqlProvider);
    wr.writeLine("--");
    wr.writeLine("-- make sure all below is done inside of TRANSACTION");
    wr.writeLine("--");
    if (!sqlBeginTrx.empty()) wr.writeLine(sqlBeginTrx + ";\n");

    // model header
    wr.write(
        "--\n" \
        "-- model description\n" \
        "--\n"
        );
    ModelInsertSql::insertTopSql(i_metaRows.modelDic, wr);
    wr.write("\n");

    for (const ModelDicTxtLangRow & row : i_metaRows.modelTxt) {
        ModelInsertSql::insertDetailSql(i_metaRows.modelDic, row, wr);
    }
    wr.write("\n");

    // model types
    wr.write(
        "--\n" \
        "-- model types\n" \
        "--\n"
        );
    for (const TypeDicRow & row : i_metaRows.typeDic) {
        ModelInsertSql::insertDetailSql(i_metaRows.modelDic, row, wr);
    }
    wr.write("\n");

    for (const TypeDicTxtLangRow & row : i_metaRows.typeTxt) {
        auto typeRowIt = TypeDicRow::byKey(row.modelId, row.typeId, i_metaRows.typeDic);
        ModelInsertSql::insertDetailSql(*typeRowIt, row, wr);
    }
    wr.write("\n");

    for (const TypeEnumLstRow & row : i_metaRows.typeEnum) {
        auto typeRowIt = TypeDicRow::byKey(row.modelId, row.typeId, i_metaRows.typeDic);
        ModelInsertSql::insertDetailSql(*typeRowIt, row, wr);
    }
    wr.write("\n");

    for (const TypeEnumTxtLangRow & row : i_metaRows.typeEnumTxt) {
        auto typeRowIt = TypeDicRow::byKey(row.modelId, row.typeId, i_metaRows.typeDic);
        ModelInsertSql::insertDetailSql(*typeRowIt, row, wr);
    }
    wr.write("\n");

    // model input parameters
    wr.write(
        "--\n" \
        "-- model input parameters\n" \
        "--\n"
        );
    for (const ParamDicRow & row : i_metaRows.paramDic) {
        ModelInsertSql::insertDetailSql(i_metaRows.modelDic, row, wr);
    }
    wr.write("\n");

    for (const ParamDicTxtLangRow & row : i_metaRows.paramTxt) {
        auto paramRowIt = ParamDicRow::byKey(row.modelId, row.paramId, i_metaRows.paramDic);
        ModelInsertSql::insertDetailSql(*paramRowIt, row, wr);
    }
    wr.write("\n");

    for (const ParamDimsRow & row : i_metaRows.paramDims) {
        auto paramRowIt = ParamDicRow::byKey(row.modelId, row.paramId, i_metaRows.paramDic);
        ModelInsertSql::insertDetailSql(*paramRowIt, row, wr);
    }
    wr.write("\n");

    for (const ParamDimsTxtLangRow & row : i_metaRows.paramDimsTxt) {
        auto paramRowIt = ParamDicRow::byKey(row.modelId, row.paramId, i_metaRows.paramDic);
        ModelInsertSql::insertDetailSql(*paramRowIt, row, wr);
    }
    wr.write("\n");

    // model output tables
    wr.write(
        "--\n" \
        "-- model output tables\n" \
        "--\n"
        );
    for (const TableDicRow & row : i_metaRows.tableDic) {
        ModelInsertSql::insertDetailSql(i_metaRows.modelDic, row, wr);
    }
    wr.write("\n");

    for (const TableDicTxtLangRow & row : i_metaRows.tableTxt) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, i_metaRows.tableDic);
        ModelInsertSql::insertDetailSql(*tableRowIt, row, wr);
    }
    wr.write("\n");

    for (const TableDimsRow & row : i_metaRows.tableDims) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, i_metaRows.tableDic);
        ModelInsertSql::insertDetailSql(*tableRowIt, row, wr);
    }
    wr.write("\n");

    for (const TableDimsTxtLangRow & row : i_metaRows.tableDimsTxt) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, i_metaRows.tableDic);
        ModelInsertSql::insertDetailSql(*tableRowIt, row, wr);
    }
    wr.write("\n");

    for (const TableAccRow & row : i_metaRows.tableAcc) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, i_metaRows.tableDic);
        ModelInsertSql::insertDetailSql(*tableRowIt, row, wr);
    }
    wr.write("\n");

    for (const TableAccTxtLangRow & row : i_metaRows.tableAccTxt) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, i_metaRows.tableDic);
        ModelInsertSql::insertDetailSql(*tableRowIt, row, wr);
    }
    wr.write("\n");

    for (const TableExprRow & row : i_metaRows.tableExpr) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, i_metaRows.tableDic);
        ModelInsertSql::insertDetailSql(*tableRowIt, row, wr);
    }
    wr.write("\n");

    for (const TableExprTxtLangRow & row : i_metaRows.tableExprTxt) {
        auto tableRowIt = TableDicRow::byKey(row.modelId, row.tableId, i_metaRows.tableDic);
        ModelInsertSql::insertDetailSql(*tableRowIt, row, wr);
    }
    wr.write("\n");

    // group of parameters or output tables
    if (i_metaRows.groupLst.size() > 0) {

        wr.write(
            "--\n" \
            "-- group of parameters or output tables\n" \
            "--\n"
            );
        for (const GroupLstRow & row : i_metaRows.groupLst) {
            ModelInsertSql::insertDetailSql(i_metaRows.modelDic, row, wr);
        }
        wr.write("\n");

        for (const GroupTxtLangRow & row : i_metaRows.groupTxt) {
            ModelInsertSql::insertDetailSql(i_metaRows.modelDic, row, wr);
        }
        wr.write("\n");

        for (const GroupPcRow & row : i_metaRows.groupPc) {
            ModelInsertSql::insertDetailSql(i_metaRows.modelDic, row, wr);
        }
        wr.write("\n");
    }

    string sqlCommitTrx = IDbExec::makeSqlCommitTransaction(i_sqlProvider);
    wr.writeLine("--");
    wr.writeLine("-- make sure all above done inside of TRANSACTION");
    wr.writeLine("--");
    if (!sqlCommitTrx.empty()) wr.writeLine(sqlCommitTrx + ";");
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
    for (const ParamTblInfo & tblInfo : paramInfoVec) {
        paramCreateTable(i_sqlProvider, tblInfo.paramTableName, "run_id", tblInfo, wr);
        paramCreateTable(i_sqlProvider, tblInfo.setTableName, "set_id", tblInfo, wr);
    }
    wr.write("\n");

    // create tables for model output tables
    wr.write(
        "--\n" \
        "-- create model output tables\n" \
        "--\n"
        );
    for (const OutTblInfo & tblInfo : outInfoVec) {
        accCreateTable(i_sqlProvider, tblInfo, wr);
        valueCreateTable(i_sqlProvider, tblInfo, wr);
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
    for (const OutTblInfo & tblInfo : outInfoVec) {
        wr.writeLine("DROP TABLE " + tblInfo.accTableName + ";");
        wr.writeLine("DROP TABLE " + tblInfo.valueTableName + ";");
    }
    wr.write("\n");

    // drop model parameters tables
    wr.write(
        "--\n" \
        "-- drop model input parameters tables\n" \
        "--\n"
        );
    for (const ParamTblInfo & tblInfo : paramInfoVec) {
        wr.writeLine("DROP TABLE " + tblInfo.paramTableName + ";");
        wr.writeLine("DROP TABLE " + tblInfo.setTableName + ";");
    }
    wr.write("\n");

}

// start sql script to create new working set
void ModelSqlBuilder::beginWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet) 
{
    try {
        // validate workset metadata: uniqueness and referential integrity
        prepareWorkset(i_metaRows, io_metaSet);

        // calculate workset digest
        //io_metaSet.worksetRow.digest = makeWorksetDigest(io_metaSet.worksetRow, i_metaRows, io_metaSet);

        // clear added flag for all model parameters
        for (ParamTblInfo & tblInfo : paramInfoVec) {
            tblInfo.isAdded = false;
        }

        // create sql script to insert workset metadata and parameter values
        bodySqlPath = outputDir + i_metaRows.modelDic.name + "_" + io_metaSet.worksetRow.name + theLog->timeStampSuffix() + ".sql";

        setWr.reset(new ModelSqlWriter(bodySqlPath));

        // delete existing workset values and metadata
        modelDigestQuoted = toQuoted(i_metaRows.modelDic.digest);
        worksetNameQuoted = toQuoted(io_metaSet.worksetRow.name);

        deleteWorkset(i_metaRows, io_metaSet);

        // insert new workset metadata
        createWorkset(i_metaRows, io_metaSet);
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

// delete existing workset parameter values and workset metadata
void ModelSqlBuilder::deleteWorkset(const MetaModelHolder & i_metaRows, const MetaSetLangHolder & /*i_metaSet*/) const
{
    // make workset id where clause
    string whereSetId = " WHERE set_id =" \
        " (" \
        " SELECT W.set_id" \
        " FROM workset_lst W" \
        " INNER JOIN model_dic M ON (M.model_id = W.model_id)" \
        " WHERE W.set_name = " + worksetNameQuoted +
        " AND M.model_digest = " + modelDigestQuoted +
        " )";

    // delete existing workset: workset parameter values
    ModelSqlWriter & wr = *setWr.get();
    wr.write(
        "--\n" \
        "-- delete existing workset: workset parameter values\n" \
        "--\n"
    );

    // DELETE FROM StartingSeed_w20120819 WHERE set_id = ... ;
    for (const ParamDicRow & row : i_metaRows.paramDic) {
        wr.outFs << "DELETE FROM " << row.dbPrefix + i_metaRows.modelDic.setPrefix + row.dbSuffix << whereSetId << ";\n";
    }

    // delete workset metadata except of workset_lst master row
    wr.write(
        "--\n" \
        "-- delete existing working set metadata\n" \
        "--\n"
    );
    wr.outFs << "DELETE FROM workset_parameter_txt " << whereSetId << ";\n";
    wr.outFs << "DELETE FROM workset_parameter " << whereSetId << ";\n";
    wr.outFs << "DELETE FROM workset_txt " << whereSetId << ";\n";
    wr.write("\n");
}

// create new workset and write workset metadata
void ModelSqlBuilder::createWorkset(const MetaModelHolder & /*i_metaRows*/, const MetaSetLangHolder & i_metaSet) const
{
    // delete existing workset: workset parameters
    ModelSqlWriter & wr = *setWr.get();

    // insert new workset: workset header
    wr.write(
        "--\n" \
        "-- insert new working set description\n" \
        "--\n"
        );
    ModelInsertSql::insertSetSql(modelDigestQuoted, worksetNameQuoted, i_metaSet.worksetRow, wr);
    wr.write("\n");

    for (const WorksetTxtLangRow & row : i_metaSet.worksetTxt) {
        ModelInsertSql::insertSetSql(modelDigestQuoted, worksetNameQuoted, row, wr);
    }
    wr.write("\n");

    // insert new workset: workset parameters
    wr.write(
        "--\n" \
        "-- insert new working set parameters\n" \
        "--\n"
        );
    for (const WorksetParamRow & row : i_metaSet.worksetParam) {
        ModelInsertSql::insertSetSql(modelDigestQuoted, worksetNameQuoted, row, wr);
    }
    wr.write("\n");

    for (const WorksetParamTxtLangRow & row : i_metaSet.worksetParamTxt) {
        ModelInsertSql::insertSetSql(modelDigestQuoted, worksetNameQuoted, row, wr);
    }
    wr.write("\n");

    // header comment for parameter values
    wr.write(
        "--\n" \
        "-- insert values of parameters\n" \
        "--\n"
        );
}

/** finish sql script to create new working set */
void ModelSqlBuilder::endWorkset(const MetaModelHolder & i_metaRows, const MetaSetLangHolder & i_metaSet)
{
    try {
        // validate workset parameters: all parameters must be added to workset
        for (const ParamTblInfo & tblInfo : paramInfoVec) {
            if (!tblInfo.isAdded)
                throw DbException("workset must include all model parameters, missing: %d: %s", tblInfo.id, tblInfo.name.c_str());
        }

        // close workset sql body file
        setWr.reset();

        // for each provider create sql script file and copy sql body into transaction scope
        for (const string & providerName : dbProviderLst) {

            // create provider sql file and put header
            ModelSqlWriter wr(outputDir + i_metaRows.modelDic.name + "_" + to_string(worksetFileIndex + 3) + "_" + i_metaSet.worksetRow.name + "_" + providerName + ".sql");
            wr.outFs <<
                "--\n" <<
                "-- create working set of parameters: " << i_metaSet.worksetRow.name << '\n' <<
                "-- model name:     " << i_metaRows.modelDic.name << '\n' <<
                "-- model digest:   " << i_metaRows.modelDic.digest << '\n' <<
                "-- script created: " << toDateTimeString(theLog->timeStampSuffix()) << '\n' <<
                "--\n\n";
            wr.throwOnFail();

            // start transaction
            string sqlBeginTrx = IDbExec::makeSqlBeginTransaction(providerName);
            wr.writeLine("--");
            wr.writeLine("-- make sure all below is done inside of TRANSACTION");
            wr.writeLine("--");
            if (!sqlBeginTrx.empty()) wr.writeLine(sqlBeginTrx + ";\n");

            // copy sql body
            {
                ifstream bodyFs(bodySqlPath.c_str(), ios::in);
                if (bodyFs.fail()) throw HelperException("Failed to create file: %s", bodySqlPath.c_str());

                wr.outFs << bodyFs.rdbuf();
                wr.throwOnFail();
            }
            wr.write("\n");

            // mark workset as readonly
            wr.write(
                "--\n" \
                "-- mark working set as readonly\n" \
                "--\n"
            );
            wr.writeLine(
                "UPDATE workset_lst SET is_readonly = 1" \
                " WHERE set_name = " + worksetNameQuoted +
                " AND model_id = (SELECT M.model_id FROM model_dic M WHERE M.model_digest = " + modelDigestQuoted + ");"
            );
            wr.write("\n");

            string sqlCommitTrx = IDbExec::makeSqlCommitTransaction(providerName);
            wr.writeLine("--");
            wr.writeLine("-- make sure all above done inside of TRANSACTION");
            wr.writeLine("--");
            if (!sqlCommitTrx.empty()) wr.writeLine(sqlCommitTrx + ";");
            wr.write("\n");
        }
        worksetFileIndex++;     // next workset sql file number

        // delete sql body file, treat file delete error as warning
        if (std::remove(bodySqlPath.c_str())) theLog->logFormatted("File delete error: %s", bodySqlPath.c_str());
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
    const MetaModelHolder & /*i_metaRows*/, const MetaSetLangHolder & /*i_metaSet*/, const string & i_name, const string & i_value
    )
{
    // check parameter name
    if (i_name.empty()) throw DbException("invalid (empty) input parameter name");

    // find parameter table info
    auto paramInfo = std::find_if(
        paramInfoVec.begin(), 
        paramInfoVec.end(),
        [i_name](const ParamTblInfo & i_info) -> bool { return i_info.name == i_name; }
    );
    if (paramInfo == paramInfoVec.cend()) throw DbException("parameter not found in parameters dictionary: %s", i_name.c_str());

    // create sql to insert value of scalar paarameter
    doAddScalarWorksetParameter(paramInfo, i_name, i_value);

    paramInfo->isAdded = true;  // done
}

// impelementation of append scalar parameter value to sql script
void ModelSqlBuilder::doAddScalarWorksetParameter(
    const vector<ParamTblInfo>::const_iterator & i_paramInfo, const string & i_name, const string & i_value
    )
{
    // scalar parameter expected: check number of dimensions
    if (i_paramInfo->dimNameVec.size() != 0) throw DbException("invalid number of dimensions for scalar parameter: %s", i_name.c_str());

    // if parameter value is string type then it must be sql-quoted
    bool isQuote =
        equalNoCase(i_paramInfo->valueTypeName.c_str(), "VARCHAR") ||
        equalNoCase(i_paramInfo->valueTypeName.c_str(), "CHAR") ||
        equalNoCase(i_paramInfo->valueTypeName.c_str(), "TEXT");

    // make sql to insert parameter value
    // INSERT INTO StartingSeed_w20120819 (set_id, param_value) 
    // SELECT W.set_id, 0.014
    // FROM workset_lst W
    // INNER JOIN model_dic M ON (M.model_id = W.model_id)
    // WHERE W.set_name = 'modelOne'
    // AND M.model_digest = '1234abcd';
    ModelSqlWriter & wr = *setWr.get();
    wr.outFs <<
        "INSERT INTO " << i_paramInfo->setTableName << " (set_id, param_value)" <<
        " SELECT W.set_id, ";
    wr.throwOnFail();

    // validate and write parameter value
    if (isQuote) {
        wr.writeQuoted(i_value);
    }
    else {
        if (i_value.empty()) throw DbException("invalid (empty) parameter value, parameter: %s", i_name.c_str());
        wr.write(i_value.c_str());
    }

    wr.outFs <<
        " FROM workset_lst W" \
        " INNER JOIN model_dic M ON (M.model_id = W.model_id)" \
        " WHERE W.set_name = " << worksetNameQuoted <<
        " AND M.model_digest = " << modelDigestQuoted << ";\n";
    wr.throwOnFail();
}

// append parameter values to sql script for new working set creation 
void ModelSqlBuilder::addWorksetParameter(
    const MetaModelHolder & i_metaRows, const MetaSetLangHolder & /*i_metaSet*/, const string & i_name, const list<string> & i_valueLst
    )
{
    // check parameters
    if (i_name.empty()) throw DbException("invalid (empty) input parameter name");

    // find parameter table info
    auto paramInfo = std::find_if(
        paramInfoVec.begin(),
        paramInfoVec.end(),
        [i_name](const ParamTblInfo & i_info) -> bool { return i_info.name == i_name; }
    );
    if (paramInfo == paramInfoVec.cend()) throw DbException("parameter not found in parameters dictionary: %s", i_name.c_str());

    int dimCount = (int)paramInfo->dimNameVec.size();

    // if this is scalar parameter then use simplified version of sql
    if (dimCount <= 0) {
        doAddScalarWorksetParameter(paramInfo, i_name, i_valueLst.front());
        paramInfo->isAdded = true;
        return;                     // done with this scalar parameter
    }

    // if parameter value is string type then it must be sql-quoted
    bool isQuote =
        equalNoCase(paramInfo->valueTypeName.c_str(), "VARCHAR") ||
        equalNoCase(paramInfo->valueTypeName.c_str(), "CHAR") ||
        equalNoCase(paramInfo->valueTypeName.c_str(), "TEXT");

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
        if (dimTypeRow->typeId > OM_MAX_BUILTIN_TYPE_ID) {

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
    // INSERT INTO ageSex_w20120817 
    //   (set_id, dim0, dim1, param_value) 
    // SELECT 
    //   W.set_id, 1, 2, 0.014
    // FROM workset_lst W
    // INNER JOIN model_dic M ON (M.model_id = W.model_id)
    // WHERE W.set_name = 'modelOne'
    // AND M.model_digest = '1234abcd';
    ModelSqlWriter & wr = *setWr.get();
    {
        // storage for dimension enum_id items
        unique_ptr<int> cellArrUptr(new int[dimCount]);
        int * cellArr = cellArrUptr.get();

        for (int k = 0; k < dimCount; k++) {
            cellArr[k] = 0;     // initial enum_id is zero for all enums
        }

        // make constant portion of insert
        string insertPrefix = "INSERT INTO " + paramInfo->setTableName + " (set_id, ";

        for (const string & dimName : paramInfo->dimNameVec) {
            insertPrefix += dimName + ", ";
        }
        insertPrefix += "param_value) SELECT W.set_id, ";

        // loop through all enums for each dimension and write sql inserts
        list<string>::const_iterator valueIt = i_valueLst.cbegin();

        for (size_t cellOffset = 0; cellOffset < totalSize; cellOffset++) {

            wr.write(insertPrefix.c_str());

            // write dimension enum_id items
            for (int k = 0; k < dimCount; k++) {
                wr.outFs << cellArr[k] << ", ";
                wr.throwOnFail();
            }

            // validate and write parameter value
            if (isQuote) {
                wr.writeQuoted(*valueIt);
            }
            else {
                if (valueIt->empty()) throw DbException("invalid (empty) parameter value, parameter: %s", i_name.c_str());
                wr.write(valueIt->c_str());
            }

            // end of insert statement
            wr.outFs <<
                " FROM workset_lst W" \
                " INNER JOIN model_dic M ON (M.model_id = W.model_id)" \
                " WHERE W.set_name = " << worksetNameQuoted <<
                " AND M.model_digest = " << modelDigestQuoted << ";";
            wr.throwOnFail();

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
    wr.write("\n");             // done with parameter insert
    paramInfo->isAdded = true;
}

// write sql script to create backward compatibility views
void ModelSqlBuilder::buildCompatibilityViews(const MetaModelHolder & i_metaRows) const
{
    try {
        // write sql script to cerate views for each provider
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
                "--   if you don't know what is this then you don't need it\n" <<
                "--\n\n";
            wr.throwOnFail();

            // input parameters compatibility views
            wr.write(
                "--\n" \
                "-- input parameters compatibility views\n" \
                "--\n"
            );
            for (const ParamTblInfo & tblInfo : paramInfoVec) {
                paramCompatibilityView(providerName, i_metaRows.modelDic, tblInfo.name, tblInfo.paramTableName, tblInfo.dimNameVec, wr);
            }
            wr.write("\n");

            // output tables compatibility views
            wr.write(
                "--\n" \
                "-- output tables compatibility views\n" \
                "--\n"
            );
            for (const OutTblInfo & tblInfo : outInfoVec) {
                outputCompatibilityView(providerName, i_metaRows.modelDic, tblInfo.name, tblInfo.valueTableName, tblInfo.dimNameVec, wr);
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
//  run_id      INT   NOT NULL,  -- set_id for worset parameter
//  dim0        INT   NOT NULL,
//  dim1        INT   NOT NULL,
//  param_value FLOAT NOT NULL,
//  PRIMARY KEY (run_id, dim0, dim1)  -- set_id for worset parameter
// );
const void ModelSqlBuilder::paramCreateTable(
    const string & i_sqlProvider, const string i_dbTableName, const string & i_runSetId, const ParamTblInfo & i_tblInfo, ModelSqlWriter & io_wr
    ) const
{
    string sqlBody = "(" + 
        i_runSetId + " INT NOT NULL, ";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += dimName + " INT NOT NULL, ";
    }

    string tname = 
        (i_sqlProvider == ORACLE_DB_PROVIDER && equalNoCase(i_tblInfo.valueTypeName.c_str(), "BIGINT")) ? 
        "NUMBER(19)" : 
        i_tblInfo.valueTypeName;

    sqlBody +=
        "param_value " + tname + " NOT NULL, " +
        "PRIMARY KEY (" + i_runSetId;

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += ", " + dimName;
    }
    sqlBody += "));";

    io_wr.outFs << IDbExec::makeSqlCreateTableIfNotExist(i_sqlProvider, i_dbTableName, sqlBody) << "\n";
}

// create table sql for accumulator table:
// CREATE TABLE salarySex_a20120820
// (
//  run_id    INT   NOT NULL,
//  dim0      INT   NOT NULL,
//  dim1      INT   NOT NULL,
//  acc_id    INT   NOT NULL,
//  sub_id    INT   NOT NULL,
//  acc_value FLOAT NULL,
//  PRIMARY KEY (run_id, dim0, dim1, acc_id, sub_id)
// );
const void ModelSqlBuilder::accCreateTable(const string & i_sqlProvider, const OutTblInfo & i_tblInfo, ModelSqlWriter & io_wr) const
{
    string sqlBody = "(run_id INT NOT NULL, ";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += dimName + " INT NOT NULL, ";
    }

    sqlBody +=
        "acc_id INT NOT NULL, " \
        "sub_id INT NOT NULL, " \
        "acc_value FLOAT NULL, " \
        "PRIMARY KEY (run_id";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += ", " + dimName;
    }
    sqlBody += ", acc_id, sub_id));";

    io_wr.outFs << IDbExec::makeSqlCreateTableIfNotExist(i_sqlProvider, i_tblInfo.accTableName, sqlBody) << "\n";
}

// create table sql for value table:
// CREATE TABLE salarySex_v20120820
// (
//  run_id     INT   NOT NULL,
//  dim0       INT   NOT NULL,
//  dim1       INT   NOT NULL,
//  expr_id    INT   NOT NULL,
//  expr_value FLOAT NULL,
//  PRIMARY KEY (run_id, dim0, dim1, expr_id)
// );
const void ModelSqlBuilder::valueCreateTable(const string & i_sqlProvider, const OutTblInfo & i_tblInfo, ModelSqlWriter & io_wr) const
{
    string sqlBody = "(run_id INT NOT NULL, ";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += dimName + " INT NOT NULL, ";
    }

    sqlBody += "expr_id INT NOT NULL," \
        " expr_value FLOAT NULL," \
        " PRIMARY KEY (run_id";

    for (const string & dimName : i_tblInfo.dimNameVec) {
        sqlBody += ", " + dimName;
    }
    sqlBody += ", expr_id));";

    io_wr.outFs << IDbExec::makeSqlCreateTableIfNotExist(i_sqlProvider, i_tblInfo.valueTableName, sqlBody) << "\n";
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

