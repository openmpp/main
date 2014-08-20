// OpenM++ data library: class to produce new model sql from metadata
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "modelSqlBuilder.h"

using namespace openm;

// create new model builder
IModelBuilder * IModelBuilder::create(const string & i_outputDir)
{
    return new ModelSqlBuilder(i_outputDir);
}

// release builder resources
IModelBuilder::~IModelBuilder() throw() { }

// create new model builder
ModelSqlBuilder::ModelSqlBuilder(const string & i_outputDir) : outputDir(i_outputDir)
{
    // convert log timestamp to model timestamp: _20120817_160459_0148 => _201208171604590148_
    modelTs = theLog->timeStampSuffix();
    size_t len = modelTs.length();
    if (len < 16) throw DbException("invalid (too short) log timestamp");

    modelTs = "_" + modelTs.substr(1, 8) + modelTs.substr(10, 6) + ((len >= 18) ? modelTs.substr(17) : "") + "_";

// to fix VC++ 2013 upd.2 strange debug behaviour
#if defined(_WIN32) && defined(_DEBUG)
    paramInfoVec = vector<ParamTblInfo>();
    outInfoVec = vector<OutTblInfo>();
#endif  // defined(_WIN32) && defined(_DEBUG)
}

// validate metadata and return sql script to create new model from supplied metadata rows
void ModelSqlBuilder::build(MetaModelHolder & io_metaRows)
{
    try {
        // validate input rows: uniqueness and referential integrity
        prepare(io_metaRows);

        // set model_dic row field values: model prefix and db table prefixes
        setModelDicRow(io_metaRows.modelDic);

        // collect info for db parameter tables, db subsample tables and value views
        setParamTableInfo(io_metaRows);
        setOutTableInfo(io_metaRows);

        // write into file sql script to create new model from supplied metadata rows
        ModelSqlWriter wr(outputDir + io_metaRows.modelDic.name + "_create_model.sql");

        buildCreateModel(io_metaRows, wr);
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

// write sql script to create new model from supplied metadata rows 
void ModelSqlBuilder::buildCreateModel(const MetaModelHolder & i_metaRows, ModelSqlWriter & io_wr) const
{
    // start transaction and get new model id
    io_wr.outFs << 
        "--\n" <<
        "-- create new model: " << i_metaRows.modelDic.name << '\n' <<
        "-- model timestamp:  " << modelTs << '\n' <<
        "-- db names prefix:  " << i_metaRows.modelDic.modelPrefix << '\n' <<
        "-- script created:   " << toDateTimeString(theLog->timeStampSuffix()) << '\n' <<
        "--\n";
    io_wr.throwOnFail();

    io_wr.write(
        "BEGIN TRANSACTION;\n" \
        "UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'model_id';\n\n"
        );

    // model header
    io_wr.write(
        "--\n" \
        "-- model description\n" \
        "--\n"
        );
    ModelInsertSql::insertSql(i_metaRows.modelDic, io_wr);
    io_wr.write("\n");

    for (const ModelDicTxtLangRow & row : i_metaRows.modelTxt) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    // model types
    io_wr.write(
        "--\n" \
        "-- model types\n" \
        "--\n"
        );
    for (const TypeDicRow & row : i_metaRows.typeDic) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const TypeDicTxtLangRow & row : i_metaRows.typeTxt) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const TypeEnumLstRow & row : i_metaRows.typeEnum) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const TypeEnumTxtLangRow & row : i_metaRows.typeEnumTxt) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    // model input parameters
    io_wr.write(
        "--\n" \
        "-- model input parameters\n" \
        "--\n"
        );
    for (const ParamDicRow & row : i_metaRows.paramDic) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const ParamDicTxtLangRow & row : i_metaRows.paramTxt) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const ParamDimsRow & row : i_metaRows.paramDims) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    // model output tables
    io_wr.write(
        "--\n" \
        "-- model output tables\n" \
        "--\n"
        );
    for (const TableDicRow & row : i_metaRows.tableDic) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const TableDicTxtLangRow & row : i_metaRows.tableTxt) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const TableDimsRow & row : i_metaRows.tableDims) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const TableDimsTxtLangRow & row : i_metaRows.tableDimsTxt) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const TableAccRow & row : i_metaRows.tableAcc) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const TableAccTxtLangRow & row : i_metaRows.tableAccTxt) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const TableUnitRow & row : i_metaRows.tableUnit) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    for (const TableUnitTxtLangRow & row : i_metaRows.tableUnitTxt) {
        ModelInsertSql::insertSql(row, io_wr);
    }
    io_wr.write("\n");

    // group of parameters or output tables
    if (i_metaRows.groupLst.size() > 0) {

        io_wr.write(
            "--\n" \
            "-- group of parameters or output tables\n" \
            "--\n"
            );
        for (const GroupLstRow & row : i_metaRows.groupLst) {
            ModelInsertSql::insertSql(row, io_wr);
        }
        io_wr.write("\n");

        for (const GroupTxtLangRow & row : i_metaRows.groupTxt) {
            ModelInsertSql::insertSql(row, io_wr);
        }
        io_wr.write("\n");

        for (const GroupPcRow & row : i_metaRows.groupPc) {
            ModelInsertSql::insertSql(row, io_wr);
        }
        io_wr.write("\n");
    }

    // create tables for model input parameters
    io_wr.write(
        "--\n" \
        "-- model input parameters\n" \
        "--\n"
        );
    for (const ParamTblInfo & tblInfo : paramInfoVec) {
        io_wr.outFs <<
            "CREATE TABLE " << tblInfo.paramTableName << " " << paramCreateTableBody("run_id", tblInfo) << ";\n" <<
            "CREATE TABLE " << tblInfo.setTableName << " " << paramCreateTableBody("set_id", tblInfo) << ";\n";
        io_wr.throwOnFail();
    }
    io_wr.write("\n");

    // create tables for model output tables
    io_wr.write(
        "--\n" \
        "-- model output tables\n" \
        "--\n"
        );
    for (const OutTblInfo & tblInfo : outInfoVec) {
        io_wr.outFs <<
            "CREATE TABLE " << tblInfo.subTableName << " " << subCreateTableBody(tblInfo) << ";\n" <<
            "CREATE TABLE " << tblInfo.valueTableName << " " << valueCreateTableBody(tblInfo) << ";\n";
        io_wr.throwOnFail();
    }
    io_wr.write("\n");

    io_wr.writeLine("COMMIT;\n");  // done
}

// start sql script to create new working set
void ModelSqlBuilder::beginWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet) 
{
    try {
        // validate workset metadata: uniqueness and referential integrity
        prepareWorkset(i_metaRows, io_metaSet);

        // clear added flag for all model parameters
        for (ParamTblInfo & tblInfo : paramInfoVec) {
            tblInfo.isAdded = false;
        }

        // create sql script file
        setWr.reset(new ModelSqlWriter(
            outputDir + toAlphaNumeric(i_metaRows.modelDic.name) + "_" + toAlphaNumeric(io_metaSet.worksetRow.name) + ".sql"
            ));

        // write metadata into sql script file
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

// create new workset and write workset metadata
void ModelSqlBuilder::createWorkset(const MetaModelHolder & i_metaRows, const MetaSetLangHolder & i_metaSet) const
{
    // start transaction and get new workset id
    ModelSqlWriter & wr = *setWr.get();
    wr.outFs <<
        "--\n" <<
        "-- create working set of parameters for model: " << i_metaRows.modelDic.name << '\n' <<
        "-- model timestamp: " << modelTs << '\n' <<
        "-- db names prefix: " << i_metaRows.modelDic.modelPrefix << '\n' <<
        "-- script created:  " << toDateTimeString(theLog->timeStampSuffix()) << '\n' <<
        "--\n";
    wr.throwOnFail();

    wr.write(
        "BEGIN TRANSACTION;\n" \
        "UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_id_set_id';\n\n"
        );

    // workset header
    wr.write(
        "--\n" \
        "-- working set description\n" \
        "--\n"
        );
    ModelInsertSql::insertSetSql(i_metaRows.modelDic, i_metaSet.worksetRow, wr);
    wr.write("\n");

    for (const WorksetTxtLangRow & row : i_metaSet.worksetTxt) {
        ModelInsertSql::insertSetSql(i_metaRows.modelDic, row, wr);
    }
    wr.write("\n");

    // workset parameters
    wr.write(
        "--\n" \
        "-- working set parameters\n" \
        "--\n"
        );
    for (const WorksetParamRow & row : i_metaSet.worksetParam) {
        ModelInsertSql::insertSetSql(i_metaRows.modelDic, row, wr);
    }
    wr.write("\n");

    for (const WorksetParamTxtLangRow & row : i_metaSet.worksetParamTxt) {
        ModelInsertSql::insertSetSql(i_metaRows.modelDic, row, wr);
    }
    wr.write("\n");
}

/** finish sql script to create new working set */
void ModelSqlBuilder::endWorkset(void) const
{
    try {
        // validate workset parameters: all parameters must be added to workset
        for (const ParamTblInfo & tblInfo : paramInfoVec) {
            if (!tblInfo.isAdded)
                throw DbException("workset must include all model parameters, missing: %d: %s", tblInfo.id, tblInfo.name.c_str());
        }

        // mark workset as readonly
        ModelSqlWriter & wr = *setWr.get();

        wr.write(
            "--\n" \
            "-- mark working set as readonly\n" \
            "--\n"
            );
        wr.write(
            "UPDATE workset_lst SET is_readonly = 1" \
            " WHERE set_id =" \
            " (SELECT RSL.id_value FROM id_lst RSL WHERE RSL.id_key = 'run_id_set_id');\n"
            );
        wr.write("\n");

        wr.writeLine("COMMIT;\n");  // done
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

// append scalar parameter value to sql script for new working set  creation 
void ModelSqlBuilder::addWorksetParameter(const MetaModelHolder & /*i_metaRows*/, const string & i_name, const string & i_value)
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

    int dimCount = paramInfo->dimNameVec.size();
    if (dimCount != 0) throw DbException("invalid number of dimensions for scalar parameter: %s", i_name.c_str());

    // if parameter value is string type then it must be sql-quoted
    bool isQuote = 
        equalNoCase(paramInfo->valueTypeName.c_str(), "VARCHAR") ||
        equalNoCase(paramInfo->valueTypeName.c_str(), "CHAR");

    // make sql to insert parameter value
    // INSERT INTO modelone_201208171604590148_w0_ageSex (set_id, value) 
    // SELECT RSL.id_value, 0.014 
    // FROM id_lst RSL WHERE RSL.id_key = 'run_id_set_id';
    ModelSqlWriter & wr = *setWr.get();
    wr.outFs <<
        "INSERT INTO " << paramInfo->setTableName << " (set_id, value)" <<
        " SELECT RSL.id_value, ";
    wr.throwOnFail();

    // validate and write parameter value
    if (isQuote) {
        wr.writeQuoted(i_value);
    }
    else {
        if (i_value.empty()) throw DbException("invalid (empty) parameter value, parameter: %s", i_name.c_str());
        wr.write(i_value.c_str());
    }

    wr.write(" FROM id_lst RSL WHERE RSL.id_key = 'run_id_set_id';\n");
    wr.write("\n");
    paramInfo->isAdded = true;  // done
}

// append scalar parameter value to sql script for new working set  creation 
void ModelSqlBuilder::addWorksetParameter(
    const MetaModelHolder & i_metaRows, const string & i_name, const list<string *> & i_valueLst
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

    int dimCount = paramInfo->dimNameVec.size();
    if (dimCount <= 0) throw DbException("invalid parameter rank for parameter: %s", i_name.c_str());

    // if parameter value is string type then it must be sql-quoted
    bool isQuote =
        equalNoCase(paramInfo->valueTypeName.c_str(), "VARCHAR") ||
        equalNoCase(paramInfo->valueTypeName.c_str(), "CHAR");

    // get dimensions list
    int mId = i_metaRows.modelDic.modelId;
    const ParamDimsRow dRow(mId, paramInfo->id, "");
    auto dimRange = std::equal_range(
        i_metaRows.paramDims.cbegin(), 
        i_metaRows.paramDims.cend(), 
        dRow, 
        [](const ParamDimsRow & i_left, const ParamDimsRow & i_right) -> bool { 
            return
                i_left.modelId < i_right.modelId || 
                i_left.modelId == i_right.modelId && i_left.paramId < i_right.paramId;
        }
        );
    if (dimCount > 0 && dimRange.first == i_metaRows.paramDims.cend() || dimCount != (int)(dimRange.second - dimRange.first))
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
                        i_left.modelId == i_right.modelId && i_left.typeId < i_right.typeId;
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

    // make sql to insert parameter dimasion enums and parameter value
    // INSERT INTO modelone_201208171604590148_w0_ageSex 
    //   (set_id, dim0, dim1, value) 
    // SELECT 
    //   RSL.id_value, 1, 2, 0.014
    // FROM id_lst RSL WHERE RSL.id_key = 'run_id_set_id';
    ModelSqlWriter & wr = *setWr.get();
    {
        // storage for dimension enum_id items
        unique_ptr<int> cellArrUptr(new int[dimCount]);
        int * cellArr = cellArrUptr.get();

        for (int k = 0; k < dimCount; k++) {
            cellArr[k] = 0;     // initial enum_id is zero for all enums
        }

        // loop through all enums for each dimension and write sql inserts
        list<string *>::const_iterator valueIt = i_valueLst.cbegin();

        for (size_t cellOffset = 0; cellOffset < totalSize; cellOffset++) {

            wr.outFs <<
                "INSERT INTO " << paramInfo->setTableName << " (set_id, ";
            wr.throwOnFail();

            // add dimension column names
            for (const string & dimName : paramInfo->dimNameVec) {
                wr.outFs << dimName << ", ";
                wr.throwOnFail();
            }

            wr.write("value) SELECT RSL.id_value, ");

            // write dimension enum_id items
            for (int k = 0; k < dimCount; k++) {
                wr.outFs << cellArr[k] << ", ";
                wr.throwOnFail();
            }

            // validate and write parameter value
            const string * valPtr = *valueIt;
            if (valPtr == nullptr) throw DbException("invalid (empty) parameter value, parameter: %s", i_name.c_str());
            if (isQuote) {
                wr.writeQuoted(*valPtr);
            }
            else {
                if (valPtr->empty()) throw DbException("invalid (empty) parameter value, parameter: %s", i_name.c_str());
                wr.write(valPtr->c_str());
            }

            // end of insert statement
            wr.write(" FROM id_lst RSL WHERE RSL.id_key = 'run_id_set_id';\n");

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
        // put descriptive header
        ModelSqlWriter wr(outputDir + i_metaRows.modelDic.name + "_optional_views.sql");

        wr.outFs <<
            "--\n" <<
            "-- compatibility views for model: " << i_metaRows.modelDic.name << '\n' <<
            "-- model timestamp:  " << modelTs << '\n' <<
            "-- db names prefix:  " << i_metaRows.modelDic.modelPrefix << '\n' <<
            "-- script created:   " << toDateTimeString(theLog->timeStampSuffix()) << '\n' <<
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
            wr.outFs << "CREATE VIEW " << tblInfo.name << " AS " << paramCompatibilityViewBody(i_metaRows, tblInfo) << ";\n";
            wr.throwOnFail();
        }
        wr.write("\n");

        // output tables compatibility views
        wr.write(
            "--\n" \
            "-- output tables compatibility views\n" \
            "--\n"
            );
        for (const OutTblInfo & tblInfo : outInfoVec) {
            wr.outFs << "CREATE VIEW " << tblInfo.name << " AS " << outputCompatibilityViewBody(i_metaRows, tblInfo) << ";\n";
            wr.throwOnFail();
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

// sort and validate metadata rows for uniqueness and referential integrity
void ModelSqlBuilder::prepare(MetaModelHolder & io_metaRows) const
{
    // validate model timestamp: must be same as in model source code
    if (io_metaRows.modelDic.timestamp != modelTs) 
        throw DbException("invalid model timestamp: %s, expected: %s", io_metaRows.modelDic.timestamp.c_str(), modelTs.c_str());

    int mId = io_metaRows.modelDic.modelId;
        
    // model_dic_txt table
    // unique: model id and language name; master key: model id
    sort(io_metaRows.modelTxt.begin(), io_metaRows.modelTxt.end(), ModelDicTxtLangRow::uniqueLangKeyLess);

    for (vector<ModelDicTxtLangRow>::const_iterator rowIt = io_metaRows.modelTxt.cbegin(); rowIt != io_metaRows.modelTxt.cend(); ++rowIt) {

        if (rowIt->modelId != mId)
            throw DbException("in model_dic_txt invalid model id: %d, expected: %d in row with language id: %d and name: %s", rowIt->modelId, mId, rowIt->langId, rowIt->langName.c_str());

        vector<ModelDicTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.modelTxt.cend() && ModelDicTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in model_dic_txt not unique model id: %d and language name: %s", rowIt->modelId, rowIt->langName.c_str());
    }

    // type_dic table
    // unique: model id and type id; unique: model id and type name; master key: model id
    sort(io_metaRows.typeDic.begin(), io_metaRows.typeDic.end(), TypeDicRow::isKeyLess);

    for (vector<TypeDicRow>::const_iterator rowIt = io_metaRows.typeDic.cbegin(); rowIt != io_metaRows.typeDic.cend(); ++rowIt) {

        if (rowIt->modelId != mId) 
            throw DbException("in type_dic invalid model id: %d, expected: %d in row with type id: %d", rowIt->modelId, mId, rowIt->typeId);

        vector<TypeDicRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.typeDic.cend() && TypeDicRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in type_dic not unique model id: %d and type id: %d", rowIt->modelId, rowIt->typeId);

        if (std::any_of(
            io_metaRows.typeDic.cbegin(), 
            io_metaRows.typeDic.cend(),
            [rowIt](const TypeDicRow & i_row) -> bool { 
                return 
                    i_row.modelId == rowIt->modelId && i_row.typeId != rowIt->typeId && i_row.name == rowIt->name;
            }
            ))
            throw DbException("in type_dic not unique model id: %d and type name: %s", rowIt->modelId, rowIt->name.c_str());
    }

    // type_dic_txt table
    // unique: model id, type id, language name; master key: model id, type id
    sort(io_metaRows.typeTxt.begin(), io_metaRows.typeTxt.end(), TypeDicTxtLangRow::uniqueLangKeyLess);

    for (vector<TypeDicTxtLangRow>::const_iterator rowIt = io_metaRows.typeTxt.cbegin(); rowIt != io_metaRows.typeTxt.cend(); ++rowIt) {

        TypeDicRow mkRow(rowIt->modelId, rowIt->typeId);
        if (!std::binary_search(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            mkRow,
            TypeDicRow::isKeyLess
            ))
            throw DbException("in type_dic_txt invalid model id: %d and type id: %d: not found in type_dic", rowIt->modelId, rowIt->typeId);

        vector<TypeDicTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.typeTxt.cend() && TypeDicTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in type_dic_txt not unique model id: %d, type id: %d and language name: %s", rowIt->modelId, rowIt->typeId, rowIt->langName.c_str());
    }

    // type_enum_lst table
    // unique: model id, type id, enum id; master key: model id, type id
    sort(io_metaRows.typeEnum.begin(), io_metaRows.typeEnum.end(), TypeEnumLstRow::isKeyLess);

    for (vector<TypeEnumLstRow>::const_iterator rowIt = io_metaRows.typeEnum.cbegin(); rowIt != io_metaRows.typeEnum.cend(); ++rowIt) {

        TypeDicRow mkRow(rowIt->modelId, rowIt->typeId);
        if (!std::binary_search(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            mkRow,
            TypeDicRow::isKeyLess
            ))
            throw DbException("in type_enum_lst invalid model id: %d and type id: %d: not found in type_dic", rowIt->modelId, rowIt->typeId);

        vector<TypeEnumLstRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.typeEnum.cend() && TypeEnumLstRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in type_enum_lst not unique model id: %d, type id: %d and enum id: %d", rowIt->modelId, rowIt->typeId, rowIt->enumId);
    }

    // type_enum_txt table
    // unique: model id, type id, enum id, language; master key: model id, type id, enum id
    sort(io_metaRows.typeEnumTxt.begin(), io_metaRows.typeEnumTxt.end(), TypeEnumTxtLangRow::uniqueLangKeyLess);

    for (vector<TypeEnumTxtLangRow>::const_iterator rowIt = io_metaRows.typeEnumTxt.cbegin(); rowIt != io_metaRows.typeEnumTxt.cend(); ++rowIt) {

        TypeEnumLstRow mkRow(rowIt->modelId, rowIt->typeId, rowIt->enumId);
        if (!std::binary_search(
            io_metaRows.typeEnum.cbegin(),
            io_metaRows.typeEnum.cend(),
            mkRow,
            TypeEnumLstRow::isKeyLess
            ))
            throw DbException("in type_enum_txt invalid model id: %d, type id: %d and enum id: %d not found in type_enum_lst", rowIt->modelId, rowIt->typeId, rowIt->enumId);

        vector<TypeEnumTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.typeEnumTxt.cend() && TypeEnumTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in type_enum_txt not unique model id: %d, type id: %d enum id: %d and language name: %s", rowIt->modelId, rowIt->typeId, rowIt->enumId, rowIt->langName.c_str());
    }

    // parameter_dic table
    // unique: model id, parameter id; unique: model id, parameter name; 
    // master key: model id; foreign key: model id, type id;
    sort(io_metaRows.paramDic.begin(), io_metaRows.paramDic.end(), ParamDicRow::isKeyLess);

    for (vector<ParamDicRow>::const_iterator rowIt = io_metaRows.paramDic.cbegin(); rowIt != io_metaRows.paramDic.cend(); ++rowIt) {

        if (rowIt->modelId != mId) 
            throw DbException("in parameter_dic invalid model id: %d, expected: %d in row with type id: %d", rowIt->modelId, mId, rowIt->typeId);

        TypeDicRow fkRow(rowIt->modelId, rowIt->typeId);
        if (!std::binary_search(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            fkRow,
            TypeDicRow::isKeyLess
            ))
            throw DbException("in parameter_dic invalid model id: %d and type id: %d: not found in type_dic", rowIt->modelId, rowIt->typeId);

        vector<ParamDicRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.paramDic.cend() && ParamDicRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in parameter_dic not unique model id: %d and parameter id: %d", rowIt->modelId, rowIt->paramId);

        if (std::any_of(
            io_metaRows.paramDic.cbegin(),
            io_metaRows.paramDic.cend(),
            [rowIt](const ParamDicRow & i_row) -> bool {
                return 
                    i_row.modelId == rowIt->modelId && i_row.paramId != rowIt->paramId && i_row.paramName == rowIt->paramName;
            }
            ))
            throw DbException("in parameter_dic not unique model id: %d and parameter name: %s", rowIt->modelId, rowIt->paramName.c_str());
    }

    // parameter_dic_txt table
    // unique: model id, parameter id, language; master key: model id, parameter id
    sort(io_metaRows.paramTxt.begin(), io_metaRows.paramTxt.end(), ParamDicTxtLangRow::uniqueLangKeyLess);

    for (vector<ParamDicTxtLangRow>::const_iterator rowIt = io_metaRows.paramTxt.cbegin(); rowIt != io_metaRows.paramTxt.cend(); ++rowIt) {

        ParamDicRow mkRow(rowIt->modelId, rowIt->paramId);
        if (!std::binary_search(
            io_metaRows.paramDic.cbegin(),
            io_metaRows.paramDic.cend(),
            mkRow,
            ParamDicRow::isKeyLess
            ))
            throw DbException("in parameter_dic_txt invalid model id: %d and parameter id: %d: not found in parameter_dic", rowIt->modelId, rowIt->paramId);

        vector<ParamDicTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.paramTxt.cend() && ParamDicTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in parameter_dic_txt not unique model id: %d, parameter id: %d and language name: %s", rowIt->modelId, rowIt->paramId, rowIt->langName.c_str());
    }

    // parameter_dims table
    // unique: model id, parameter id, dimension name; master key: model id, parameter id; foreign key: model id, type id;
    sort(io_metaRows.paramDims.begin(), io_metaRows.paramDims.end(), ParamDimsRow::isKeyLess);

    for (vector<ParamDimsRow>::const_iterator rowIt = io_metaRows.paramDims.cbegin(); rowIt != io_metaRows.paramDims.cend(); ++rowIt) {

        ParamDicRow mkRow(rowIt->modelId, rowIt->paramId);
        if (!std::binary_search(
            io_metaRows.paramDic.cbegin(),
            io_metaRows.paramDic.cend(),
            mkRow,
            ParamDicRow::isKeyLess
            ))
            throw DbException("in parameter_dims invalid model id: %d and parameter id: %d: not found in parameter_dic", rowIt->modelId, rowIt->paramId);

        TypeDicRow fkRow(rowIt->modelId, rowIt->typeId);
        if (!std::binary_search(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            fkRow,
            TypeDicRow::isKeyLess
            ))
            throw DbException("in parameter_dims invalid model id: %d and type id: %d: not found in type_dic", rowIt->modelId, rowIt->typeId);

        vector<ParamDimsRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.paramDims.cend() && ParamDimsRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in parameter_dims not unique model id: %d, parameter id: %d and dimension name: %s", rowIt->modelId, rowIt->paramId, rowIt->name.c_str());
    }

    // table_dic table
    // unique: model id, table id; unique: model id, table name; master key: model id
    sort(io_metaRows.tableDic.begin(), io_metaRows.tableDic.end(), TableDicRow::isKeyLess);

    for (vector<TableDicRow>::const_iterator rowIt = io_metaRows.tableDic.cbegin(); rowIt != io_metaRows.tableDic.cend(); ++rowIt) {

        if (rowIt->modelId != mId) 
            throw DbException("in table_dic invalid model id: %d, expected: %d in row with table id: %d, name: %s", rowIt->modelId, mId, rowIt->tableId, rowIt->tableName.c_str());

        vector<TableDicRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableDic.cend() && TableDicRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in table_dic not unique model id: %d and table id: %d", rowIt->modelId, rowIt->tableId);

        if (std::any_of(
            io_metaRows.tableDic.cbegin(),
            io_metaRows.tableDic.cend(),
            [rowIt](const TableDicRow & i_row) -> bool {
                return
                    i_row.modelId == rowIt->modelId && i_row.tableId != rowIt->tableId && i_row.tableName == rowIt->tableName;
            }
            ))
            throw DbException("in table_dic not unique model id: %d and table name: %s", rowIt->modelId, rowIt->tableName.c_str());
    }

    // table_dic_txt table
    // unique: model id, table id, language; master key: model id, table id
    sort(io_metaRows.tableTxt.begin(), io_metaRows.tableTxt.end(), TableDicTxtLangRow::uniqueLangKeyLess);

    for (vector<TableDicTxtLangRow>::const_iterator rowIt = io_metaRows.tableTxt.cbegin(); rowIt != io_metaRows.tableTxt.cend(); ++rowIt) {

        TableDicRow mkRow(rowIt->modelId, rowIt->tableId);
        if (!std::binary_search(
            io_metaRows.tableDic.cbegin(),
            io_metaRows.tableDic.cend(),
            mkRow,
            TableDicRow::isKeyLess
            ))
            throw DbException("in table_dic_txt invalid model id: %d and table id: %d: not found in table_dic", rowIt->modelId, rowIt->tableId);

        vector<TableDicTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableTxt.cend() && TableDicTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in table_dic_txt not unique model id: %d, table id: %d and language name: %s", rowIt->modelId, rowIt->tableId, rowIt->langName.c_str());
    }

    // table_dims table
    // unique: model id, table id, dimension name; master key: model id, table id; foreign key: model id, type id;
    sort(io_metaRows.tableDims.begin(), io_metaRows.tableDims.end(), TableDimsRow::isKeyLess);

    for (vector<TableDimsRow>::const_iterator rowIt = io_metaRows.tableDims.cbegin(); rowIt != io_metaRows.tableDims.cend(); ++rowIt) {

        TableDicRow mkRow(rowIt->modelId, rowIt->tableId);
        if (!std::binary_search(
            io_metaRows.tableDic.cbegin(),
            io_metaRows.tableDic.cend(),
            mkRow,
            TableDicRow::isKeyLess
            ))
            throw DbException("in table_dims invalid model id: %d and table id: %d: not found in table_dic", rowIt->modelId, rowIt->tableId);

        TypeDicRow fkRow(rowIt->modelId, rowIt->typeId);
        if (!std::binary_search(
            io_metaRows.typeDic.cbegin(),
            io_metaRows.typeDic.cend(),
            fkRow,
            TypeDicRow::isKeyLess
            ))
            throw DbException("in table_dims invalid model id: %d and type id: %d: not found in type_dic", rowIt->modelId, rowIt->typeId);

        vector<TableDimsRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableDims.cend() && TableDimsRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in table_dims not unique model id: %d, table id: %d and dimension name: %s", rowIt->modelId, rowIt->tableId, rowIt->name.c_str());
    }

    // table_dims_txt table
    // unique: model id, table id, dimension name, language; master key: model id, table id, dimension name;
    sort(io_metaRows.tableDimsTxt.begin(), io_metaRows.tableDimsTxt.end(), TableDimsTxtLangRow::uniqueLangKeyLess);

    for (vector<TableDimsTxtLangRow>::const_iterator rowIt = io_metaRows.tableDimsTxt.cbegin(); rowIt != io_metaRows.tableDimsTxt.cend(); ++rowIt) {

        TableDimsRow mkRow(rowIt->modelId, rowIt->tableId, rowIt->name);
        if (!std::binary_search(
            io_metaRows.tableDims.cbegin(),
            io_metaRows.tableDims.cend(),
            mkRow,
            TableDimsRow::isKeyLess
            ))
            throw DbException("in table_dims_txt invalid model id: %d table id: %d and dimension name: %s: not found in table_dims", rowIt->modelId, rowIt->tableId, rowIt->name.c_str());

        vector<TableDimsTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableDimsTxt.cend() && TableDimsTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in table_dims_txt not unique model id: %d, table id: %d, dimension name: %s and language: %s", rowIt->modelId, rowIt->tableId, rowIt->name.c_str(), rowIt->langName.c_str());
    }

    // table_acc table
    // master key: model id, table id;
    // unique: model id, table id, accumulator id; unique: model id, table id, accumulator name; 
    sort(io_metaRows.tableAcc.begin(), io_metaRows.tableAcc.end(), TableAccRow::isKeyLess);

    for (vector<TableAccRow>::const_iterator rowIt = io_metaRows.tableAcc.cbegin(); rowIt != io_metaRows.tableAcc.cend(); ++rowIt) {

        TableDicRow mkRow(rowIt->modelId, rowIt->tableId);
        if (!std::binary_search(
            io_metaRows.tableDic.cbegin(),
            io_metaRows.tableDic.cend(),
            mkRow,
            TableDicRow::isKeyLess
            ))
            throw DbException("in table_acc invalid model id: %d and table id: %d: not found in table_dic", rowIt->modelId, rowIt->tableId);

        vector<TableAccRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableAcc.cend() && TableAccRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in table_acc not unique model id: %d, table id: %d and accumulator id: %d", rowIt->modelId, rowIt->tableId, rowIt->accId);

        if (std::any_of(
            io_metaRows.tableAcc.cbegin(),
            io_metaRows.tableAcc.cend(),
            [rowIt](const TableAccRow & i_row) -> bool {
                return
                    i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId && i_row.accId != rowIt->accId &&
                    i_row.name == rowIt->name;
            }
            ))
            throw DbException("in table_acc not unique model id: %d, table id: %d and accumulator name: %s", rowIt->modelId, rowIt->tableId, rowIt->name.c_str());
    }

    // table_acc_txt table
    // unique: model id, table id, accumulator id, language; master key: model id, table id, accumulator id;
    sort(io_metaRows.tableAccTxt.begin(), io_metaRows.tableAccTxt.end(), TableAccTxtLangRow::uniqueLangKeyLess);

    for (vector<TableAccTxtLangRow>::const_iterator rowIt = io_metaRows.tableAccTxt.cbegin(); rowIt != io_metaRows.tableAccTxt.cend(); ++rowIt) {

        TableAccRow mkRow(rowIt->modelId, rowIt->tableId, rowIt->accId);
        if (!std::binary_search(
            io_metaRows.tableAcc.cbegin(),
            io_metaRows.tableAcc.cend(),
            mkRow,
            TableAccRow::isKeyLess
            ))
            throw DbException("in table_acc_txt invalid model id: %d table id: %d and accumulator id: %d: not found in table_acc", rowIt->modelId, rowIt->tableId, rowIt->accId);

        vector<TableAccTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableAccTxt.cend() && TableAccTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in table_acc_txt not unique model id: %d, table id: %d, accumulator id: %d and language: %s", rowIt->modelId, rowIt->tableId, rowIt->accId, rowIt->langName.c_str());
    }

    // table_unit table
    // unique: model id, table id, unit id; unique: model id, table id, unit name;
    // master key: model id, table id;
    sort(io_metaRows.tableUnit.begin(), io_metaRows.tableUnit.end(), TableUnitRow::isKeyLess);

    for (vector<TableUnitRow>::const_iterator rowIt = io_metaRows.tableUnit.cbegin(); rowIt != io_metaRows.tableUnit.cend(); ++rowIt) {

        TableDicRow mkRow(rowIt->modelId, rowIt->tableId);
        if (!std::binary_search(
            io_metaRows.tableDic.cbegin(),
            io_metaRows.tableDic.cend(),
            mkRow,
            TableDicRow::isKeyLess
            ))
            throw DbException("in table_unit invalid model id: %d and table id: %d: not found in table_dic", rowIt->modelId, rowIt->tableId);

        vector<TableUnitRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableUnit.cend() && TableUnitRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in table_unit not unique model id: %d, table id: %d and unit id: %d", rowIt->modelId, rowIt->tableId, rowIt->unitId);

        if (std::any_of(
            io_metaRows.tableUnit.cbegin(),
            io_metaRows.tableUnit.cend(),
            [rowIt](const TableUnitRow & i_row) -> bool {
                return
                    i_row.modelId == rowIt->modelId && i_row.tableId == rowIt->tableId && i_row.unitId != rowIt->unitId &&
                    i_row.name == rowIt->name;
            }
            ))
            throw DbException("in table_unit not unique model id: %d, table id: %d and unit name: %s", rowIt->modelId, rowIt->tableId, rowIt->name.c_str());
    }

    // table_unit_txt table
    // unique: model id, table id, unit id, language; master key: model id, table id, unit id;
    sort(io_metaRows.tableUnitTxt.begin(), io_metaRows.tableUnitTxt.end(), TableUnitTxtLangRow::uniqueLangKeyLess);

    for (vector<TableUnitTxtLangRow>::const_iterator rowIt = io_metaRows.tableUnitTxt.cbegin(); rowIt != io_metaRows.tableUnitTxt.cend(); ++rowIt) {

        TableUnitRow mkRow(rowIt->modelId, rowIt->tableId, rowIt->unitId);
        if (!std::binary_search(
            io_metaRows.tableUnit.cbegin(),
            io_metaRows.tableUnit.cend(),
            mkRow,
            TableUnitRow::isKeyLess
            ))
            throw DbException("in table_unit_txt invalid model id: %d table id: %d and unit id: %d: not found in table_unit", rowIt->modelId, rowIt->tableId, rowIt->unitId);

        vector<TableUnitTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.tableUnitTxt.cend() && TableUnitTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in table_unit_txt not unique model id: %d, table id: %d, unit id: %d and language: %s", rowIt->modelId, rowIt->tableId, rowIt->unitId, rowIt->langName.c_str());
    }

    // group_lst table
    // unique: model id, group id; master key: model id
    sort(io_metaRows.groupLst.begin(), io_metaRows.groupLst.end(), GroupLstRow::isKeyLess);

    for (vector<GroupLstRow>::const_iterator rowIt = io_metaRows.groupLst.cbegin(); rowIt != io_metaRows.groupLst.cend(); ++rowIt) {

        if (rowIt->modelId != mId) 
            throw DbException("in group_lst invalid model id: %d, expected: %d in row with group id: %d, name: %s", rowIt->modelId, mId, rowIt->groupId, rowIt->name.c_str());

        vector<GroupLstRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.groupLst.cend() && GroupLstRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in group_lst not unique model id: %d and group id: %d", rowIt->modelId, rowIt->groupId);
    }

    // group_txt table
    // unique: model id, group id, language; master key: model id, group id
    sort(io_metaRows.groupTxt.begin(), io_metaRows.groupTxt.end(), GroupTxtLangRow::uniqueLangKeyLess);

    for (vector<GroupTxtLangRow>::const_iterator rowIt = io_metaRows.groupTxt.cbegin(); rowIt != io_metaRows.groupTxt.cend(); ++rowIt) {

        GroupLstRow mkRow(rowIt->modelId, rowIt->groupId);
        if (!std::binary_search(
            io_metaRows.groupLst.cbegin(),
            io_metaRows.groupLst.cend(),
            mkRow,
            GroupLstRow::isKeyLess
            ))
            throw DbException("in group_txt invalid model id: %d and group id: %d: not found in group_lst", rowIt->modelId, rowIt->groupId);

        vector<GroupTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.groupTxt.cend() && GroupTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in group_txt not unique model id: %d, group id: %d and language name: %s", rowIt->modelId, rowIt->groupId, rowIt->langName.c_str());
    }

    // group_pc table
    // unique: model id, group id, child position; master key: model id, group id
    sort(io_metaRows.groupPc.begin(), io_metaRows.groupPc.end(), GroupPcRow::isKeyLess);

    for (vector<GroupPcRow>::const_iterator rowIt = io_metaRows.groupPc.cbegin(); rowIt != io_metaRows.groupPc.cend(); ++rowIt) {

        GroupLstRow mkRow(rowIt->modelId, rowIt->groupId);
        if (!std::binary_search(
            io_metaRows.groupLst.cbegin(),
            io_metaRows.groupLst.cend(),
            mkRow,
            GroupLstRow::isKeyLess
            ))
            throw DbException("in group_pc invalid model id: %d and group id: %d: not found in group_lst", rowIt->modelId, rowIt->groupId);

        vector<GroupPcRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaRows.groupPc.cend() && GroupPcRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in group_pc not unique model id: %d, group id: %d and child position: %d", rowIt->modelId, rowIt->groupId, rowIt->childPos);
    }
}

// validate workset metadata: uniqueness and referential integrity
void ModelSqlBuilder::prepareWorkset(const MetaModelHolder & i_metaRows, MetaSetLangHolder & io_metaSet) const
{
    // set workset_lst row field values: readonly status and creation date-time
    // validate model_dic field values: model prefix and timestamp
    setWorksetRow(i_metaRows.modelDic, io_metaSet.worksetRow);

    int mId = i_metaRows.modelDic.modelId;
    int setId = io_metaSet.worksetRow.setId;

    // workset_lst table
    // master key: model id
    if (io_metaSet.worksetRow.modelId != mId)
        throw DbException("in workset_lst invalid model id: %d, expected: %d", io_metaSet.worksetRow.modelId, mId);

    // workset_txt table
    // unique: set id and language name; master key: set id; 
    // foreign key: model id
    sort(io_metaSet.worksetTxt.begin(), io_metaSet.worksetTxt.end(), WorksetTxtLangRow::uniqueLangKeyLess);

    for (vector<WorksetTxtLangRow>::const_iterator rowIt = io_metaSet.worksetTxt.cbegin(); rowIt != io_metaSet.worksetTxt.cend(); ++rowIt) {

        if (rowIt->setId != setId)
            throw DbException("in workset_txt invalid set id: %d, expected: %d in row with language id: %d and name: %s", rowIt-setId, setId, rowIt->langId, rowIt->langName.c_str());

        if (rowIt->modelId != mId)
            throw DbException("in workset_txt invalid model id: %d, expected: %d in row with language id: %d and name: %s", rowIt->modelId, mId, rowIt->langId, rowIt->langName.c_str());

        vector<WorksetTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaSet.worksetTxt.cend() && WorksetTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in workset_txt not unique set id: %d and language name: %s", rowIt->setId, rowIt->langName.c_str());
    }

    // workset_parameter table
    // unique: set id, parameter id; master key: set id; 
    // foreign key: model id, parameter id;
    sort(io_metaSet.worksetParam.begin(), io_metaSet.worksetParam.end(), WorksetParamRow::isKeyLess);

    for (vector<WorksetParamRow>::const_iterator rowIt = io_metaSet.worksetParam.cbegin(); rowIt != io_metaSet.worksetParam.cend(); ++rowIt) {

        if (rowIt->setId != setId)
            throw DbException("in workset_parameter invalid set id: %d, expected: %d in row with parameter id: %d", rowIt->setId, setId, rowIt->paramId);

        ParamDicRow fkRow(rowIt->modelId, rowIt->paramId);
        if (!std::binary_search(
            i_metaRows.paramDic.cbegin(),
            i_metaRows.paramDic.cend(),
            fkRow,
            ParamDicRow::isKeyLess
            ))
            throw DbException("in workset_parameter invalid model id: %d and parameter id: %d: not found in parameter_dic", rowIt->modelId, rowIt->paramId);

        vector<WorksetParamRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaSet.worksetParam.cend() && WorksetParamRow::isKeyEqual(*rowIt, *nextIt))
            throw DbException("in workset_parameter not unique set id: %d and parameter id: %d", rowIt->setId, rowIt->paramId);
    }

    // workset_parameter_txt table
    // unique: set id, parameter id, language; master key: set id, parameter id
    sort(io_metaSet.worksetParamTxt.begin(), io_metaSet.worksetParamTxt.end(), WorksetParamTxtLangRow::uniqueLangKeyLess);

    for (vector<WorksetParamTxtLangRow>::const_iterator rowIt = io_metaSet.worksetParamTxt.cbegin(); rowIt != io_metaSet.worksetParamTxt.cend(); ++rowIt) {

        WorksetParamRow mkRow(rowIt->setId, rowIt->paramId);
        if (!std::binary_search(
            io_metaSet.worksetParam.cbegin(),
            io_metaSet.worksetParam.cend(),
            mkRow,
            WorksetParamRow::isKeyLess
            ))
            throw DbException("in workset_parameter_txt invalid set id: %d and parameter id: %d: not found in workset_parameter", rowIt->setId, rowIt->paramId);

        vector<WorksetParamTxtLangRow>::const_iterator nextIt = rowIt + 1;

        if (nextIt != io_metaSet.worksetParamTxt.cend() && WorksetParamTxtLangRow::uniqueLangKeyEqual(*rowIt, *nextIt))
            throw DbException("in workset_parameter_txt not unique set id: %d, parameter id: %d and language name: %s", rowIt->setId, rowIt->paramId, rowIt->langName.c_str());
    }
}

// set field values for model_dic table row
void ModelSqlBuilder::setModelDicRow(ModelDicRow & io_mdRow) const
{
    // validate model timestamp: must be same as in model source code
    if (io_mdRow.timestamp != modelTs) throw DbException("invalid model timestamp: %s, expected: %s", io_mdRow.timestamp.c_str(), modelTs.c_str());

    // trim model name and make model prefix from model name and timestamp
    io_mdRow.modelPrefix = makeModelPrefix(io_mdRow.name, io_mdRow.timestamp);

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

// set workset_lst row field values: readonly status and creation date-time
// validate model_dic field values: model prefix and timestamp
void ModelSqlBuilder::setWorksetRow(const ModelDicRow & i_mdRow, WorksetLstRow & io_wsRow) const
{ 
    // validate model timestamp
    if (i_mdRow.timestamp.empty() || trim(i_mdRow.timestamp).length() < 16) throw DbException("invalid (or empty) model timestamp");
    if (i_mdRow.timestamp.length() > 32) throw DbException("invalid (too long) model timestamp: %s", i_mdRow.timestamp.c_str());

    // validate model timestamp: must be same as in model source code
    if (i_mdRow.timestamp != modelTs) throw DbException("invalid model timestamp: %s, expected: %s", i_mdRow.timestamp.c_str(), modelTs.c_str());

    // validate model prefix
    if (i_mdRow.modelPrefix.empty() || trim(i_mdRow.modelPrefix).length() < 16) throw DbException("invalid (or empty) model prefix");
    if (i_mdRow.modelPrefix.length() > 32) throw DbException("invalid (too long) model prefix: %s", i_mdRow.modelPrefix.c_str());

    // mark workset as read-write and set workset creation date-time
    io_wsRow.isReadonly = false;
    io_wsRow.updateDateTime = toDateTimeString(theLog->timeStampSuffix());  // get log date-time as string
}

// collect info for db parameter tables
void ModelSqlBuilder::setParamTableInfo(MetaModelHolder & io_metaRows)
{
    // collect table information for all parameters
    for (ParamDicRow & paramRow : io_metaRows.paramDic) {

        // convert name into db table suffix
        paramRow.dbNameSuffix = makeDbNameSuffix(paramRow.paramId, paramRow.paramName);

        // make db table names
        ParamTblInfo tblInf;
        tblInf.id = paramRow.paramId;
        tblInf.name = paramRow.paramName;
        tblInf.paramTableName = io_metaRows.modelDic.modelPrefix + io_metaRows.modelDic.paramPrefix + paramRow.dbNameSuffix;
        tblInf.setTableName = io_metaRows.modelDic.modelPrefix + io_metaRows.modelDic.setPrefix + paramRow.dbNameSuffix;

        // collect dimension names
        tblInf.dimNameVec.clear();
        for (const ParamDimsRow & dimRow : io_metaRows.paramDims) {
            if (dimRow.paramId == tblInf.id) tblInf.dimNameVec.push_back(dimRow.name);
        }

        // set value column db type by parameter type
        tblInf.valueTypeName.clear();
        for (const TypeDicRow & typeRow : io_metaRows.typeDic) {

            if (typeRow.typeId == paramRow.typeId) {

                // built-in types (ordered as in omc grammar for clarity)

                // C++ ambiguous integral type
                // (in C/C++, the signedness of char is not specified)
                if (equalNoCase(typeRow.name.c_str(), "char")) tblInf.valueTypeName = "SMALLINT";

                // C++ signed integral types
                if (equalNoCase(typeRow.name.c_str(), "schar")) tblInf.valueTypeName = "SMALLINT";
                if (equalNoCase(typeRow.name.c_str(), "short")) tblInf.valueTypeName = "SMALLINT";
                if (equalNoCase(typeRow.name.c_str(), "int")) tblInf.valueTypeName = "INT";
                if (equalNoCase(typeRow.name.c_str(), "long")) tblInf.valueTypeName = "BIGINT";
                if (equalNoCase(typeRow.name.c_str(), "llong")) tblInf.valueTypeName = "BIGINT";

                // C++ unsigned integral types (including bool)
                if (equalNoCase(typeRow.name.c_str(), "bool")) tblInf.valueTypeName = "SMALLINT";
                if (equalNoCase(typeRow.name.c_str(), "uchar")) tblInf.valueTypeName = "SMALLINT";
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

                // path to a file (a string)
                if (equalNoCase(typeRow.name.c_str(), "file")) {
                    tblInf.valueTypeName = "VARCHAR(" + to_string(OM_PATH_MAX) + ")";
                }

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
void ModelSqlBuilder::setOutTableInfo(MetaModelHolder & io_metaRows)
{
    // collect information for all output tables
    for (TableDicRow & tableRow : io_metaRows.tableDic) {

        // convert name into db table suffix
        tableRow.dbNameSuffix = makeDbNameSuffix(tableRow.tableId, tableRow.tableName);

        // make db table names
        OutTblInfo tblInf;
        tblInf.id = tableRow.tableId;
        tblInf.name = tableRow.tableName;
        tblInf.subTableName = io_metaRows.modelDic.modelPrefix + io_metaRows.modelDic.subPrefix + tableRow.dbNameSuffix;
        tblInf.valueTableName = io_metaRows.modelDic.modelPrefix + io_metaRows.modelDic.valuePrefix + tableRow.dbNameSuffix;

        // collect dimension names
        tblInf.dimNameVec.clear();
        for (const TableDimsRow & dimRow : io_metaRows.tableDims) {
            if (dimRow.tableId == tblInf.id) tblInf.dimNameVec.push_back(dimRow.name);
        }

        // collect accumulator names
        tblInf.accNameVec.clear();
        for (const TableAccRow & accRow : io_metaRows.tableAcc) {
            if (accRow.tableId == tblInf.id) tblInf.accNameVec.push_back(accRow.name);
        }
        if (tblInf.accNameVec.empty()) 
            throw DbException("output table accumulators not found for table: %s", tableRow.tableName.c_str());

        // translate expressions into sql
        ModelAggregationSql aggr(tblInf.subTableName, "S", tblInf.accNameVec, tblInf.dimNameVec);

        for (TableUnitRow & unitRow : io_metaRows.tableUnit) {
            if (unitRow.tableId == tblInf.id) {
                unitRow.expr = aggr.translateAggregationExpr(unitRow.src);  // translate expression to sql aggregation
            }
        }
        // if (tblInf.exprVec.empty()) 
        //    throw DbException("output table aggregation expressions not found for table: %s", tableRow.tableName.c_str());

        outInfoVec.push_back(tblInf);   // add to output tables info vector
    }
}

// make model prefix from model name and timestamp
const string ModelSqlBuilder::makeModelPrefix(const string & i_name, const string & i_timestamp)
{
    if (i_name.empty() || trim(i_name).length() < 1) throw DbException("invalid (empty) model name");
    if (i_timestamp.empty() || trim(i_timestamp).length() < 16) throw DbException("invalid (too short) model timestamp");

    // validate model name and timestamp max size
    if (i_name.length() > 255 || i_timestamp.length() > 32)
        throw DbException("invalid (too long) model name: %s or timestamp: %s", i_name.c_str(), i_timestamp.c_str());

    // in model name use only [a-z,0-9] and _ underscore
    // make sure prefix size not longer than 32 
    string sPrefix = toAlphaNumeric(i_name, 32);

    // make model prefix from model name and timestamp
    return
        sPrefix.substr(0, 32 - i_timestamp.length()) + i_timestamp;
}

// make unique part of db table name for parameter or output table, ie: 1234_ageSex
const string ModelSqlBuilder::makeDbNameSuffix(int i_id, const string & i_src)
{
    if (i_src.empty() || trim(i_src).length() < 1) throw DbException("invalid (empty) source name, id: %d", i_id);

    // in db table name use only [A-Z,a-z,0-9] and _ underscore
    // make sure name size not longer than (32 - max prefix size)
    string sId = to_string(i_id) + "_";
    string sName = toAlphaNumeric(i_src, 32);

    return sId + sName.substr(0, 32 - (sId.length() + OM_DB_TABLE_TYPE_PREFIX_LEN));
}

