/**
* @file
* OpenM++ modeling library: run controller class to create new model run(s) and support data exchange (create run parameters)
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
#include "modelHelper.h"
#include "runControllerImpl.h"

using namespace std;
using namespace openm;

// copy input parameters from working set and "base" run into new run id
// search for input parameter value in following order:
//   use value of parameter specified in run options: (command-line, ini-file, profile_option table)
//   use parameter.csv file if SubFrom.Parameter == csv or by default if parameters csv directory specified
//   import parameter from upstream model run if any of Import.* specified
//   use value of parameter from working set of model parameters
//   else search workset or base run:
//     if base run id not specified or workset is not default then search workset to get parameter value
//     else or if not found in workset then search base run:
//       if base run explicitly specified then search this base run else serach workset base run
//   if parameter value not found then raise an exception
void RunController::createRunParameters(int i_runId, int i_setId, bool i_isWsDefault, int i_baseRunId, IDbExec * i_dbExec) const
{
    // find input parameters workset
    if (i_setId <= 0) throw DbException("invalid set id or model working sets not found in database");

    // increase read only flag to "lock" workset until parameters copy is done
    string sSetId = to_string(i_setId);
    i_dbExec->update(
        "UPDATE workset_lst SET is_readonly = is_readonly + 1 WHERE set_id = " + sSetId
        );
    int nReadonly = i_dbExec->selectToInt(
        "SELECT is_readonly FROM workset_lst WHERE set_id = " + sSetId,
        0);

    // workset must exist and must be read-only
    vector<WorksetLstRow> wsVec = IWorksetLstTable::byKey(i_dbExec, i_setId);
    if (wsVec.empty()) throw DbException("workset must exist (set id: %d)", i_setId);
    if (nReadonly <= 1) throw DbException("workset must be read-only (set id: %d)", i_setId);

    const WorksetLstRow & wsRow = wsVec[0];

    // validate workset: it must be read-only and must be from the same model
    if (wsRow.modelId != modelId) throw DbException("invalid workset model id: %d, expected: %d (set id: %d)", wsRow.modelId, modelId, i_setId);

    // get list of model parameters and list of parameters included into workset
    vector<ParamDicRow> paramVec = metaStore->paramDic->rows();
    vector<WorksetParamRow> wsParamVec = IWorksetParamTable::select(i_dbExec, i_setId);

    // check if parameters csv directory specified and accessible
    string paramDir = argOpts().strOption(RunOptionsKey::paramDir);
    bool isParamDir = !paramDir.empty() && isFileExists(paramDir.c_str());

    // is csv contains enum id's or enum codes
    bool isIdCsv = argOpts().boolOption(RunOptionsKey::useIdCsv);
    bool isIdValue = argOpts().boolOption(RunOptionsKey::useIdParamValue);

    // find import run of upstream model and verify it is completed successfully
    map<string, RunImportOpts> riOptsMap;

    bool isImport = importOptsMap.size() > 0;
    if (isImport) {
        unique_ptr<IModelDicTable> mainModelTable(IModelDicTable::create(i_dbExec));  // list of the models in "main" database

        for (const auto & m : importOptsMap) {
            riOptsMap[m.first] = std::move(findImportRun(m.first, m.second, mainModelTable.get(), i_dbExec));
        }
    }

    // copy parameters into destination run, searching it by following order:
    //   from run options (command-line, ini-file, profile_option table)
    //   from parameter.csv file if exist
    //   if base run speified use base run parameter table
    //   else
    //   workset parameter value table
    //   if workset based on run then base run of workset
    // calculate parameter values digest and store only single copy of parameter values
    string sRunId = to_string(i_runId);
    string sModelId = to_string(modelId);
    int nBaseRunId = (i_baseRunId > 0) ? i_baseRunId : wsRow.baseRunId;
    string paramDot = string(RunOptionsKey::parameterPrefix) + ".";

    for (vector<ParamDicRow>::const_iterator paramIt = paramVec.cbegin(); paramIt != paramVec.cend(); ++paramIt) {

        // calculate parameter source: command line (or ini-file), generate "iota", workset, based run, run options
        bool isInserted = false;
        bool isCheckSubCount = false;
        bool isBaseRunFullCopy = false;
        bool isArgOption = argOpts().isOptionExist((paramDot + paramIt->paramName).c_str());
        int nParamSubCount = parameterSubCount(paramIt->paramId);   // if >1 then multiple sub-values expected
        int nRank = paramIt->rank;

        // check if parameter exist in workset
        auto wsParamRowIt = WorksetParamRow::byKey(i_setId, paramIt->paramId, wsParamVec);
        bool isExistInWs = wsParamRowIt != wsParamVec.cend();

        // default sub-value id for parameter, use 0 if parameter exist in base run
        int defaultSubId = (isExistInWs && (i_baseRunId <= 0 || !i_isWsDefault)) ? wsParamRowIt->defaultSubId : 0;

        // find sub-value rule to get parameter sub-values
        // if parameter does not have sub-values then it would be "default" rule
        ParamSubOpts subOpts = subOptsById(*paramIt, nParamSubCount, defaultSubId);
        bool isFromDb = subOpts.from == RunOptionsKey::dbSubValue;
        bool isFromCsv = subOpts.from == RunOptionsKey::csvSubValue;
        bool isFromIota = subOpts.from == RunOptionsKey::iotaSubValue;
        bool isFromDefault = subOpts.from == RunOptionsKey::defaultValue;

        if (!isParamDir && isFromCsv) throw DbException("invalid (empty) parameter.csv file path for parameter: %s", paramIt->paramName.c_str());

        // get parameter type
        const TypeDicRow * typeRow = metaStore->typeDic->byKey(modelId, paramIt->typeId);
        if (typeRow == nullptr)
            throw DbException("invalid (not found) type of parameter: %s", paramIt->paramName.c_str());

        // find parameter type in model parameters array to get typeid and size
        const ParameterNameSizeItem * paramNameSizeItem = find_if(
            parameterNameSizeArr,
            parameterNameSizeArr + PARAMETER_NAME_ARR_LEN,
            [&paramIt](const ParameterNameSizeItem & i_item) -> bool { return paramIt->paramName == i_item.name; }
        );
        if (paramNameSizeItem == parameterNameSizeArr + PARAMETER_NAME_ARR_LEN)
            throw ModelException("parameter %d: %s not found in model parameters", paramIt->paramId, paramIt->paramName.c_str());

        // insert parameter from run options
        if (isArgOption) {

            // only one sub-value can be supplied on command-line (ini-file, profile table)
            if (nParamSubCount > 1)
                throw DbException("only one sub-value can be provided for parameter: %s", paramIt->paramName.c_str());

            // get parameter value as sql string 
            // do special parameter value handling if required: 
            // bool conversion to 0/1 or quoted for string or enum code to id
            string sVal = argOpts().strOption((paramDot + paramIt->paramName).c_str());
            if (typeRow->isString()) sVal = toQuoted(sVal);  // "file" type is VARCHAR

            if ((typeRow->isInt() || typeRow->isBigInt()) && !TypeDicRow::isIntValid(sVal.c_str()))
                throw DbException("invalid value '%s' of parameter: %s", sVal.c_str(), paramIt->paramName.c_str());
            
            if ((typeRow->isFloat() || typeRow->isTime()) && !TypeDicRow::isFloatValid(sVal.c_str()))
                throw DbException("invalid value '%s' of parameter: %s", sVal.c_str(), paramIt->paramName.c_str());

            if (typeRow->isBool()) {
                if (!TypeDicRow::isBoolValid(sVal.c_str())) 
                    throw DbException("invalid value '%s' of parameter: %s", sVal.c_str(), paramIt->paramName.c_str());

                sVal = TypeDicRow::isBoolTrue(sVal.c_str()) ? "1" : "0";
            }

            if (!typeRow->isBuiltIn() && !isIdValue) {
                const TypeEnumLstRow * enumRow = metaStore->typeEnumLst->byName(modelId, paramIt->typeId, sVal.c_str());
                if (enumRow == nullptr) 
                    throw DbException("invalid value '%s' of parameter: %s", sVal.c_str(), paramIt->paramName.c_str());

                sVal = to_string(enumRow->enumId);
            }

            // insert the value
            i_dbExec->update(
                "INSERT INTO " + paramIt->dbRunTable + " (run_id, sub_id, param_value) VALUES (" + sRunId + ", 0, " + sVal + ")"
                );
            isInserted = true;
        }

        // generate "iota" parameter values: numeric from 0 to sub-value count-1
        if (!isInserted && isFromIota) {

            if (nRank > 0) throw DbException("parameter %s is not a scalar", paramIt->paramName.c_str());

            // parameter type must be numeric
            if (!typeRow->isBigInt() && !typeRow->isInt() && !typeRow->isFloat()) throw DbException("invalid (not a numeric) type of parameter: %s", paramIt->paramName.c_str());

            // insert the value
            for (int k = 0; k < nParamSubCount; k++) {
                string sVal = to_string(k);
                i_dbExec->update(
                    "INSERT INTO " + paramIt->dbRunTable + " (run_id, sub_id, param_value) VALUES (" + sRunId + ", " + sVal + ", " + sVal + ")"
                );
            }
            isInserted = true;
        }

        // insert from parameter.csv file if sub-value from option either "csv" or "default" and csv directory specified
        if (!isInserted && (isFromCsv || (isFromDefault && isParamDir))) {

            // if parameter.csv exist then copy it into parameter value table
            string csvPath = makeFilePath(paramDir.c_str(), paramIt->paramName.c_str(), ".csv");
            
            if (isFileExists(csvPath.c_str())) {
                theLog->logMsg("Read", csvPath.c_str());

                // read from csv file, parse csv lines and insert values into parameter run table
                unique_ptr<IParameterRunWriter> writer(IParameterRunWriter::create(
                    i_runId,
                    paramIt->paramName.c_str(),
                    i_dbExec,
                    meta()
                ));
                writer->loadCsvParameter(i_dbExec, subOpts.subIds, csvPath.c_str(), isIdCsv);

                isInserted = true;
            }
            else {
                if (isFromCsv) throw DbException("parameter csv file not found: %s", csvPath.c_str());
            }
        }

        // import paramter from upstream model output table or parameter
        if (!isInserted && isImport) {
            if (auto impRow = metaStore->paramImport->byKey(modelId, paramIt->paramId); impRow != nullptr) {

                const RunImportOpts & riOpts = riOptsMap[impRow->fromModel];

                // find and import output table from upsteram model
                // if not found as output table then import parameter from upsteram model
                isInserted = importOutputTable(i_runId, *paramIt, paramNameSizeItem, riOpts, impRow, i_dbExec);
                bool isFp = false;
                if (!isInserted) {
                    isInserted = importParameter(i_runId, *paramIt, paramNameSizeItem, riOpts, impRow, subOpts, i_dbExec);
                    isFp = true;
                }
                if (!isInserted)
                    throw DbException("parameter %d: %s not found as imported: %s from model %d: %s",
                        paramIt->paramId, paramIt->paramName.c_str(), impRow->fromName.c_str(), riOpts.modelId, impRow->fromModel.c_str());
            
                // parameter imported from other model run
                i_dbExec->update("INSERT INTO run_parameter_import (" \
                    " run_id, parameter_hid, is_from_parameter, from_model_id, from_model_digest, from_run_id, from_run_digest" \
                    " ) VALUES ( " +
                    to_string(i_runId) + ", " +
                    to_string(paramIt->paramHid) + ", " +
                    (isFp ? "1, " : "0, ") +
                    to_string(riOpts.modelId) + ", " +
                    toQuoted(riOpts.modelDigest) + ", " +
                    to_string(riOpts.runId) + ", " +
                    toQuoted(riOpts.runDigest) + ")"
                );
                isCheckSubCount = true;
            }
        }

        // if parameter exist in workset and
        //   if base run not specified or workset is not default then copy from workset:
        //   copy parameter from workset parameter value table
        //   copy parameter value notes from workset parameter text table
        if (!isInserted && (isFromDb || isFromDefault) && isExistInWs && (i_baseRunId <= 0 || !i_isWsDefault)) {

            // validate: parameter must exist in workset and must have enough sub-values
            int nSub = i_dbExec->selectToInt(
                "SELECT sub_count FROM workset_parameter" \
                " WHERE set_id = " + sSetId + " AND parameter_hid = " + to_string(paramIt->paramHid),
                0);
            if (nSub <= 0)
                throw DbException("parameter %d: %s must be included in workset (id: %d)", paramIt->paramId, paramIt->paramName.c_str(), i_setId);
            if (nSub < nParamSubCount)
                throw DbException("parameter %d: %s must have %d sub-values in workset (id: %d)", paramIt->paramId, paramIt->paramName.c_str(), nParamSubCount, i_setId);

            // use sub id options to make where sub_id filter and map source sub id's to parameter run sub id's
            string flt = makeWhereSubId(subOpts, defaultSubId);
            string subFlds = mapSelectedSubId(subOpts);

            vector<ParamDimsRow> paramDimVec = metaStore->paramDims->byModelIdParamId(modelId, paramIt->paramId);
            if (nRank < 0 || nRank != (int)paramDimVec.size())
                throw DbException("invalid parameter rank or dimensions not found for parameter: %s", paramIt->paramName.c_str());

            string sDimLst = "";
            for (int nDim = 0; nDim < nRank; nDim++) {
                sDimLst += paramDimVec[nDim].name + ", ";
            }

            // copy parameter from workset parameter
            i_dbExec->update(
                "INSERT INTO " + paramIt->dbRunTable + " (run_id, sub_id, " + sDimLst + " param_value)" +
                " SELECT " + sRunId + "," + subFlds + ", " + sDimLst + " param_value" +
                " FROM " + paramIt->dbSetTable +
                " WHERE set_id = " + sSetId +
                (!flt.empty() ? " AND " + flt : "")
            );
            i_dbExec->update(
                "INSERT INTO run_parameter_txt (run_id, parameter_hid, lang_id, note)" \
                " SELECT " + sRunId + ", parameter_hid, lang_id, note" +
                " FROM workset_parameter_txt" +
                " WHERE set_id = " + sSetId +
                " AND parameter_hid = " + to_string(paramIt->paramHid)
                );
            isInserted = true;
            isCheckSubCount = true;
        }

        // insert parameter values from base run:
        // if base run has same number of sub-values 
        // then insert link to parameter values from base run
        // else copy only part of source sub-values as new parameter values
        if (!isInserted && (isFromDb || isFromDefault) && nBaseRunId > 0) {

            // validate: parameter must exist in base run and must have enough sub-values
            int nSub = i_dbExec->selectToInt(
                "SELECT sub_count FROM run_parameter" \
                " WHERE run_id = " + to_string(nBaseRunId) + " AND parameter_hid = " + to_string(paramIt->paramHid),
                0);
            if (nSub <= 0)
                throw DbException("parameter %d: %s must be included in base run (id: %d)", paramIt->paramId, paramIt->paramName.c_str(), nBaseRunId);
            if (nSub < nParamSubCount)
                throw DbException("parameter %d: %s must have %d sub-values in base run (id: %d)", paramIt->paramId, paramIt->paramName.c_str(), nParamSubCount, nBaseRunId);

            // if base run has same number of sub-values and it is in range [0, sub count - 1] then copy all else only part of source sub-values
            isBaseRunFullCopy = nParamSubCount == nSub &&
                ((nParamSubCount == 1 && ((subOpts.kind == KindSubIds::single && subOpts.subIds[0] == 0) || subOpts.kind == KindSubIds::defaultId) && subOpts.subIds[0] == defaultSubId) ||
                (nParamSubCount > 1 && subOpts.kind == KindSubIds::range && subOpts.subIds[0] == 0 && subOpts.subIds.back() == nParamSubCount - 1));

            if (isBaseRunFullCopy) {            // copy parameter from base run
                i_dbExec->update(
                    "INSERT INTO run_parameter (run_id, parameter_hid, base_run_id, sub_count, value_digest)" \
                    " SELECT " + sRunId + ", parameter_hid, base_run_id, sub_count, value_digest" \
                    " FROM run_parameter"
                    " WHERE run_id = " + to_string(nBaseRunId) +
                    " AND parameter_hid = " + to_string(paramIt->paramHid)
                );
            }
            else {      // copy only required number of sub-values from base run

                // use sub id options to make where sub_id filter and map source sub id's to parameter run sub id's
                // default sub-value id =0 for parameters from base run
                string flt = makeWhereSubId(subOpts, 0);
                string subFlds = mapSelectedSubId(subOpts);

                vector<ParamDimsRow> paramDimVec = metaStore->paramDims->byModelIdParamId(modelId, paramIt->paramId);
                if (nRank < 0 || nRank != (int)paramDimVec.size())
                    throw DbException("invalid parameter rank or dimensions not found for parameter: %s", paramIt->paramName.c_str());

                string sDimLst = "";
                for (int nDim = 0; nDim < nRank; nDim++) {
                    sDimLst += paramDimVec[nDim].name + ", ";
                }

                i_dbExec->update(
                    "INSERT INTO " + paramIt->dbRunTable + " (run_id, sub_id, " + sDimLst + " param_value)" +
                    " SELECT " + sRunId + ", " + subFlds + ", " + sDimLst + " param_value" +
                    " FROM " + paramIt->dbRunTable +
                    " WHERE run_id = " + to_string(nBaseRunId) +
                    (!flt.empty() ? " AND " + flt : "")
                );
                isCheckSubCount = true;
            }
            isInserted = true;
        }

        // parameter values must be inserted from one of above sources
        if (!isInserted) {
            if (nRank <= 0)
                throw DbException("parameter %d not found: %s, it must specified as model run option or be included in workset (set id: %d), ", paramIt->paramId, paramIt->paramName.c_str(), i_setId);
            else
                throw DbException("parameter %d not found: %s, workset (set id: %d) must include this parameter", paramIt->paramId, paramIt->paramName.c_str(), i_setId);
        }

        // check sub-values count: select count of sub-values for each for each sub_id, it must be equal to parameter size
        if (isCheckSubCount) {
            checkParamSubCounts(i_runId, nParamSubCount, *paramIt, i_dbExec);
        }

        // if new value of parameter inserted then calculte parameter values digest
        if (!isBaseRunFullCopy) {

            unique_ptr<IParameterRunWriter> writer(IParameterRunWriter::create(
                i_runId,
                paramIt->paramName.c_str(),
                i_dbExec,
                meta(),
                argOpts().strOption(RunOptionsKey::doubleFormat).c_str()
            ));
            writer->digestParameter(i_dbExec, nParamSubCount, paramNameSizeItem->typeOf);
        }
    }

    // close all parameter import database connections and cleanup metadata
    for (auto & ri : riOptsMap) {
        ri.second.dbExec.reset(nullptr);
        ri.second.meta.reset(nullptr);
    }

    // "unlock" workset: parameters copy completed.
    i_dbExec->update("UPDATE workset_lst SET is_readonly = 1 WHERE set_id = " + sSetId);
}

// make part of where clause to select sub_id's
const string RunController::makeWhereSubId(const MetaLoader::ParamSubOpts & i_subOpts, int i_defaultSubId) const
{
    switch (i_subOpts.kind) {
    case KindSubIds::single:
        return "sub_id = " + to_string(i_subOpts.subIds[0]);

    case KindSubIds::defaultId:
        return "sub_id = " + to_string(i_defaultSubId);

    case KindSubIds::range:
        return "sub_id BETWEEN " + to_string(i_subOpts.subIds[0]) + " AND " + to_string(i_subOpts.subIds[i_subOpts.subIds.size() - 1]);

    case KindSubIds::list:
        {
            string flt = "sub_id IN (";
            for (size_t k = 0; k < i_subOpts.subIds.size(); k++) {
                flt += ((k > 0) ? ", " : "") + to_string(i_subOpts.subIds[k]);
            }
            return flt += ")";
        }

    default: break;     // only to suppress compiler warning
    }
    return "";  // by default use all sub id's from source
}

// make part of select columns list to map source sub id's to run parameter sub_id's
const string RunController::mapSelectedSubId(const MetaLoader::ParamSubOpts & i_subOpts) const
{
    switch (i_subOpts.kind) {
    case KindSubIds::single:
    case KindSubIds::defaultId:
        return "0";

    case KindSubIds::range:
        return "sub_id - " + to_string(i_subOpts.subIds[0]);

    case KindSubIds::list:
        {
            string sql = "CASE";
            for (size_t k = 0; k < i_subOpts.subIds.size(); k++) {
                sql += " WHEN sub_id = " + to_string(i_subOpts.subIds[k]) + " THEN " + to_string(k);
            }
            return sql += " END";
        }

    default: break;     // only to suppress compiler warning
    }
    return "sub_id";    // default result is no mapping: return sub_id as is
}

// return parameter sub-value options by parameter id
const MetaLoader::ParamSubOpts RunController::subOptsById(const ParamDicRow & i_paramRow, int i_subCount, int i_defaultSubId) const
{
    // check if sub-value options specified
    ParamSubOpts ps;

    if (const auto psIt = subOptsMap.find(i_paramRow.paramId); psIt != subOptsMap.cend()) { // options found, use copy of it
        ps = psIt->second;
        if (ps.from == RunOptionsKey::iotaSubValue) return ps;  // iota sub-values, return options as is
    }
    else {  // not found: create default options
        ps.from = RunOptionsKey::defaultValue;
        ps.subCount = i_subCount;
    }

    // if no SubValue. options then set sub id's
    if (ps.kind == KindSubIds::none) {
        if (ps.subCount <= 1) {
            ps.kind = KindSubIds::defaultId;
        }
        else {
            ps.subIds.clear();
            for (int n = 0; n < i_subCount; n++) {
                ps.subIds.push_back(n);
            }
            ps.subCount = i_subCount;
            ps.kind = KindSubIds::range;
        }
    }

    // set actual default id value
    if (ps.kind == KindSubIds::defaultId) {
        ps.subIds = { i_defaultSubId };
        ps.subCount = 1;
    }
    return ps;
}

// check sub-value id's and count: sub id must be zero in range [0, sub count -1] and count must be equal to parameter size
void RunController::checkParamSubCounts(int i_runId, int i_subCount, const ParamDicRow & i_paramRow, IDbExec * i_dbExec) const
{
    // calculate size of parameter value: all sub values expected to be that size
    int64_t nSize = metaStore->parameterSize(i_paramRow);

    // select row count for each sub-value
    const auto rowLst = i_dbExec->selectRowList(
        "SELECT sub_id, COUNT(*) FROM " + i_paramRow.dbRunTable + " WHERE run_id = " + to_string(i_runId) + " GROUP BY sub_id",
        ValueRowAdapter(1, typeid(nSize))
    );

    // expected all row counts to be equal to parameter size
    // sub-value id's must be in the list of sub id's for that parameter
    int64_t rowCount = 0;
    for (const auto & rowUptr : rowLst) {

        ValueRow * row = dynamic_cast<ValueRow *>(rowUptr.get());
        int nSub = row->idArr[0];
        int64_t nCount = row->dbVal.llVal;
        rowCount += nCount;

        if (i_subCount == 1 && nSub != 0)
            throw DbException("invalid sub-value id %d in parameter: %s, expected: 0", nSub, i_paramRow.paramName.c_str());
        if (i_subCount != 1 && (nSub < 0 || nSub > i_subCount - 1))
            throw DbException("invalid sub-value id %d in parameter: %s, expected: [0, %d]", nSub, i_paramRow.paramName.c_str(), i_subCount - 1);
        if (nCount != nSize)
            throw DbException("invalid number of rows in parameter: %s sub-value: %d, expected: %lld, found: %lld", i_paramRow.paramName.c_str(), nSub, nSize, nCount);
    }

    // all sub-values must exist in run parameter
    if (rowCount != nSize * i_subCount)
        throw DbException("invalid number of rows in parameter: %s, expected: %lld, found: %lld", i_paramRow.paramName.c_str(), nSize * i_subCount, rowCount);
}

/** find import run of upstream model and verify it is completed successfully */
MetaLoader::RunImportOpts RunController::findImportRun(
    const string & i_modelName, const ImportOpts & i_importOpts, const IModelDicTable * i_mainModelTable, IDbExec * i_dbExec
) const
{
    // if database connection string specified then open new database connection
    // else:
    //   if model exist in "main" database then select from main database
    //   else try to open default model.sqlite database
    RunImportOpts riOpts;
    bool isMainDb = false;
    IDbExec * dbEx = i_dbExec;

    if (i_importOpts.connectionStr.empty()) {
        const ModelDicRow * mr = i_mainModelTable->findFirst(
            [i_modelName](ModelDicRow i_row) -> bool { return i_row.name == i_modelName; }
        );
        isMainDb = mr != nullptr;
    }
    if (!isMainDb) {    // if connection string specified or model not found in "main" database
        try {
            riOpts.dbExec.reset(IDbExec::create(
                SQLITE_DB_PROVIDER,
                (!i_importOpts.connectionStr.empty() ? i_importOpts.connectionStr : "Database=" + i_modelName + ".sqlite; Timeout=86400; OpenMode=ReadOnly;")
            ));
            dbEx = riOpts.dbExec.get();
        }
        catch (exception & ex) {
            throw DbException("Unable to open model database: %s. %s", i_modelName.c_str(), ex.what());
        }
    }

    // where conditions
    // model name must be the same as model_parameter_import.from_model
    string andModelName = " AND M.model_name = " + toQuoted(i_modelName);
    string andModelDigest = !i_importOpts.modelDigest.empty() ? " AND M.model_digest = " + toQuoted(i_importOpts.modelDigest) : "";
    string andModelId = i_importOpts.modelId > 0 ? " AND M.model_id = " + to_string(i_importOpts.modelId) : "";
    string andRunDigest = !i_importOpts.runDigest.empty() ? " AND M.model_digest = " + toQuoted(i_importOpts.runDigest) : "";
    string andRunId = i_importOpts.runId > 0 ? " AND R.run_id = " + to_string(i_importOpts.modelId) : "";
    string andRunName = !i_importOpts.runName.empty() ? " AND R.run_name = " + toQuoted(i_importOpts.runName) : "";

    // if run digest specified then select run_id from run_lst by unique key
    if (i_importOpts.kind == ImportKind::runDigest) {
        riOpts.runId = dbEx->selectToInt(
            "SELECT R.run_id " \
            " FROM run_lst R" \
            " INNER JOIN model_dic M ON (M.model_id = R.model_id)" \
            " WHERE run_digest = " + toQuoted(i_importOpts.runDigest) +
            andModelName +
            andModelDigest + andModelId + andRunId + andRunName,
            0);
        if (riOpts.runId <= 0) throw DbException("error: import run digest: %s not found", i_importOpts.runDigest.c_str());
    }

    // if run id specified then check is it exists
    if (i_importOpts.kind == ImportKind::runId) {
        riOpts.runId = dbEx->selectToInt(
            "SELECT R.run_id " \
            " FROM run_lst R" \
            " INNER JOIN model_dic M ON (M.model_id = R.model_id)" \
            " WHERE R.run_id = " + to_string(i_importOpts.runId) +
            andModelName +
            andModelDigest + andModelId + andRunDigest + andRunName,
            0);
        if (riOpts.runId <= 0) throw DbException("error: import run id: %d not found", i_importOpts.runId);
    }

    // if run name specified then select last sucessful run from run_lst by run name
    // and use model id, model digest, model name if any specified
    if (i_importOpts.kind == ImportKind::runName) {
        
        string sql = "SELECT MAX(R.run_id)" \
            " FROM run_lst R" \
            " INNER JOIN model_dic M ON (M.model_id = R.model_id)" \
            " WHERE R.run_name = " + toQuoted(i_importOpts.runName) +
            " AND R.status = " + toQuoted(RunStatus::done) +
            andModelName +
            andModelDigest + andModelId + andRunDigest + andRunId;

        riOpts.runId = dbEx->selectToInt(sql, 0);
        if (riOpts.runId <= 0) throw DbException("error: import run name: %s not found", i_importOpts.runName.c_str());
    }

    // if model digest specified then select last sucessful run from run_lst by model digest
    // and use model name if specified
    if (i_importOpts.kind == ImportKind::modelDigest) {

        string sql = "SELECT MAX(R.run_id)" \
            " FROM run_lst R" \
            " INNER JOIN model_dic M ON (M.model_id = R.model_id)" \
            " WHERE M.model_digest = " + toQuoted(i_importOpts.modelDigest) +
            " AND R.status = " + toQuoted(RunStatus::done) +
            andModelName +
            andModelId + andRunDigest + andRunId + andRunName;

        riOpts.runId = dbEx->selectToInt(sql, 0);
        if (riOpts.runId <= 0) throw DbException("error: import model digest: %s not found", i_importOpts.modelDigest.c_str());
    }

    // if model id specified then select last sucessful run from run_lst by model id
    // and use model digest, model name if any specified
    if (i_importOpts.kind == ImportKind::modelId) {

        string sql = "SELECT MAX(R.run_id)" \
            " FROM run_lst R" \
            " INNER JOIN model_dic M ON (M.model_id = R.model_id)" \
            " WHERE M.model_id = " + to_string(i_importOpts.modelId) +
            " AND R.status = " + toQuoted(RunStatus::done) +
            andModelName +
            andModelDigest + andRunDigest + andRunId + andRunName;

        riOpts.runId = dbEx->selectToInt(sql, 0);
        if (riOpts.runId <= 0) throw DbException("error: import model id: %d not found", i_importOpts.modelId);
    }

    // if model name specified then select last sucessful run from run_lst by model name
    if (i_importOpts.kind == ImportKind::modelName) {

        string sql = "SELECT MAX(R.run_id)" \
            " FROM run_lst R" \
            " INNER JOIN model_dic M ON (M.model_id = R.model_id)" \
            " WHERE M.model_name = " + toQuoted(i_modelName) +
            " AND R.status = " + toQuoted(RunStatus::done) +
            andModelId + andModelDigest + andRunDigest + andRunId + andRunName;

        riOpts.runId = dbEx->selectToInt(sql, 0);
        if (riOpts.runId <= 0) throw DbException("error: import model name: %s not found", i_modelName.c_str());
    }

    // import run must be completed successfully
    riOpts.runDigest = dbEx->selectToStr(
        "SELECT run_digest FROM run_lst WHERE run_id = " + to_string(riOpts.runId) + " AND status = " + toQuoted(RunStatus::done)
    );
    if (riOpts.runDigest.empty()) throw DbException("error: import run id: %d not completed successfully", riOpts.runId);

    // find model by run id
    riOpts.modelId = dbEx->selectToInt(
        "SELECT model_id FROM run_lst WHERE run_id = " + to_string(riOpts.runId),
        0);
    if (riOpts.modelId <= 0) throw DbException("error: import model not found by import run id: %d", riOpts.runId);

    riOpts.modelDigest = dbEx->selectToStr(
        "SELECT model_digest FROM model_dic WHERE model_id = " + to_string(riOpts.modelId)
    );
    if (riOpts.modelDigest.empty()) throw DbException("error: import model not found by import model id: %d", riOpts.modelId);

    if ((i_importOpts.modelId > 0 && i_importOpts.modelId != riOpts.modelId) ||
        (!i_importOpts.modelDigest.empty() && i_importOpts.modelDigest != riOpts.modelDigest)) {
        throw DbException("error: import model not found by import model id: %d or digest: %s", i_importOpts.modelId, i_importOpts.modelDigest.c_str());
    }

    // load source metadata to import from parameters or output tables
    riOpts.meta.reset(new MetaHolder);
    readMetaTables(dbEx, riOpts.meta.get(), i_modelName.c_str(), riOpts.modelDigest.c_str());

    return riOpts;
}

/** import parameter value from output table expression of upstream model, return true if imported or false if not found */
bool RunController::importOutputTable(
    int i_runId,
    const ParamDicRow & i_paramRow,
    const ParameterNameSizeItem * i_nameSizeItem,
    const RunImportOpts & i_riOpts,
    const ParamImportRow * i_importRow,
    IDbExec * i_dbExec
) const {

    // if model is in different database then use model db connection else use "main" db
    IDbExec * dbEx = i_riOpts.dbExec.get() != nullptr ? i_riOpts.dbExec.get() : i_dbExec;

    // check if table with imported name exist in upstream model
    auto tRow = i_riOpts.meta->tableDic->byModelIdName(i_riOpts.modelId, i_importRow->fromName);
    if (tRow == nullptr) return false;  // not found

    // import digest must be identical for source output table and destination parameter
    if (tRow->importDigest != i_paramRow.importDigest)
        throw DbException("parameter %d: %s not compatible with imported output table: %s from model %d: %s",
            i_paramRow.paramId, i_paramRow.paramName.c_str(), i_importRow->fromName.c_str(), i_riOpts.modelId, i_importRow->fromModel.c_str());

    // parameter type must bs double
    if (i_nameSizeItem->typeOf != typeid(double))
        throw DbException("parameter %d: %s must be double type in order to be imported from output table: %s from model %d: %s",
            i_paramRow.paramId, i_paramRow.paramName.c_str(), i_importRow->fromName.c_str(), i_riOpts.modelId, i_importRow->fromModel.c_str());

    // import output table expression as pararamter value
    // by default import first expression from output table 
    // if other expression name specified as source for that parameter then check is it exist in output table
    string exprName;
    if (const auto piOpts = paramImportOptsMap.find(i_paramRow.paramId); piOpts == paramImportOptsMap.cend()) {
        exprName = i_riOpts.meta->tableExpr->byModelIdTableId(tRow->modelId, tRow->tableId)[0].name;    // default: expression 0
    }
    else {  // not a default expression name
        exprName = piOpts->second.expr;

        auto eRow = i_riOpts.meta->tableExpr->findFirst(
            [&exprName](const TableExprRow & i_row) -> bool { return i_row.name == exprName; }
        );
        if (eRow == nullptr)
            throw DbException("output table: %s does not have expression %s", i_importRow->fromName.c_str(), i_importRow->fromName.c_str());
    }

    // parameter must have only one sub value for that model run
    if (parameterSubCount(i_paramRow.paramId) != 1)
        throw DbException("parameter %d: %s must have only one sub value in order to be imported from output table expression: %s %s from model %d: %s",
            i_paramRow.paramId, i_paramRow.paramName.c_str(), i_importRow->fromName.c_str(), exprName.c_str(), i_riOpts.modelId, i_importRow->fromModel.c_str());

    // select values from output table expression
    unique_ptr<IOutputTableExprReader> rd(IOutputTableExprReader::create(
        i_riOpts.runId, i_importRow->fromName.c_str(), dbEx, i_riOpts.meta.get(), exprName.c_str()
    ));
    size_t nSize = rd->sizeOf();
    unique_ptr<double> vUptr(new double[nSize]);    // parameter type must be double
    rd->readTable(
        dbEx,
        false,      //  if row missing in source db table then use zero filling, which is default for output tables
        nSize,
        vUptr.get()
    );

    // write expression values into current model run parameter
    unique_ptr<IParameterRunWriter> wr(IParameterRunWriter::create(
        i_runId, i_paramRow.paramName.c_str(), i_dbExec, meta()
    ));
    wr->writeParameter(i_dbExec, typeid(double), 1, nSize, vUptr.get());
    return true;
}

/** import parameter from parameter of upstream model, return true if imported or false if not found */
bool RunController::importParameter(
    int i_runId,
    const ParamDicRow & i_paramRow,
    const ParameterNameSizeItem * i_nameSizeItem,
    const RunImportOpts & i_riOpts,
    const ParamImportRow * i_importRow,
    const ParamSubOpts & i_subOpts,
    IDbExec * i_dbExec
) const {

    // if model is in different database then use model db connection else use "main" db
    bool isMainDb = i_riOpts.dbExec.get() == nullptr;
    IDbExec * dbEx = !isMainDb ? i_riOpts.dbExec.get() : i_dbExec;

    // import digest must be identical for source parameter and destination parameter
    auto pRow = i_riOpts.meta->paramDic->byModelIdName(i_riOpts.modelId, i_importRow->fromName);
    if (pRow->importDigest != i_paramRow.importDigest)
        throw DbException("parameter %d: %s not compatible with imported parameter: %s from model %d: %s",
            i_paramRow.paramId, i_paramRow.paramName.c_str(), i_importRow->fromName.c_str(), i_riOpts.modelId, i_importRow->fromModel.c_str());

    // get dimension columns list for source parameter
    vector<ParamDimsRow> srcDimArr = i_riOpts.meta->paramDims->byModelIdParamId(i_riOpts.modelId, pRow->paramId);
    if (pRow->rank < 0 || pRow->rank != (int)srcDimArr.size())
        throw DbException("invalid parameter rank or dimensions not found for parameter: %s", pRow->paramName.c_str());

    // check sub value count: it must be same for source and destination parameter
    int nSubCount = parameterSubCount(i_paramRow.paramId);
    if (i_subOpts.subCount != nSubCount)
        throw ModelException("invalid number of sub-values: %d specified for parameter: %s, expected: %d",
            i_subOpts.subCount, i_paramRow.paramName.c_str(), nSubCount);

    // if parameter is in "main" database then copy between models using sql query
    if (isMainDb) {

        vector<ParamDimsRow> dstDimArr = metaStore->paramDims->byModelIdParamId(modelId, i_paramRow.paramId);
        if (i_paramRow.rank < 0 || i_paramRow.rank != (int)dstDimArr.size())
            throw DbException("invalid parameter rank or dimensions not found for parameter: %s", i_paramRow.paramName.c_str());

        string dstDimLst = "";
        for (size_t k = 0; k < dstDimArr.size(); k++) {
            dstDimLst += dstDimArr[k].name + ", ";
        }

        string srcDimLst = "";
        for (size_t k = 0; k < srcDimArr.size(); k++) {
            srcDimLst += srcDimArr[k].name + ", ";
        }

        // use sub id options to make where sub_id filter and map source sub id's to parameter run sub id's
        // default sub-value id =0 for parameters from base run
        string flt = makeWhereSubId(i_subOpts, 0);
        string subFlds = mapSelectedSubId(i_subOpts);

        // find actual run id (base run id) for source parameter values
        int srcRunId = i_dbExec->selectToInt(
            "SELECT base_run_id FROM run_parameter R" \
            " INNER JOIN model_parameter_dic M ON (M.parameter_hid = R.parameter_hid)" \
            " WHERE R.run_id = " + to_string(i_riOpts.runId) +
            " AND R.parameter_hid = " + to_string(i_paramRow.paramHid),
            0);
        if (srcRunId <= 0)
            throw DbException("imported parameter run id: %d not found for parameter: %s", i_riOpts.runId, pRow->paramName.c_str());

        // copy in the same database
        i_dbExec->update(
            "INSERT INTO " + i_paramRow.dbRunTable + " (run_id, sub_id, " + dstDimLst + " param_value)" +
            " SELECT " + to_string(i_runId) + ", " + subFlds + ", " + srcDimLst + " param_value" +
            " FROM " + pRow->dbRunTable +
            " WHERE run_id = " + to_string(srcRunId) +
            (!flt.empty() ? " AND " + flt : "")
        );
    }
    else {  // copy parameter from other database

        unique_ptr<IParameterReader> rd(IParameterReader::create(i_riOpts.runId, i_importRow->fromName.c_str(), dbEx, i_riOpts.meta.get()));
        size_t nSize = rd->sizeOf();
        auto vArr = ValueArray(i_nameSizeItem->typeOf, nSize * nSubCount);
        rd->readParameter(
            dbEx,
            i_subOpts.subIds,
            i_nameSizeItem->typeOf,
            nSize,
            vArr.ptr()
        );

        unique_ptr<IParameterRunWriter> wr(IParameterRunWriter::create(i_runId, i_paramRow.paramName.c_str(), i_dbExec, meta()));
        wr->writeParameter(
            i_dbExec,
            i_nameSizeItem->typeOf,
            nSubCount,
            nSize,
            vArr.ptr()
        );
    }

    return true;
}
