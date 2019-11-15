/**
* @file
* OpenM++ modeling library: run controller class to create new model run(s) and support data exchange
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
#include "modelHelper.h"
#include "runControllerImpl.h"

using namespace std;
using namespace openm;

/** create task run entry in database */
int RunController::createTaskRun(int i_taskId, IDbExec * i_dbExec)
{
    // update in transaction scope
    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    // get next task run id
    i_dbExec->update("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'task_run_id'");

    int taskRunId = i_dbExec->selectToInt("SELECT id_value FROM id_lst WHERE id_key = 'task_run_id'", 0);
    if (taskRunId <= 0)
        throw DbException("invalid task run id: %d", taskRunId);

    string dtStr = toDateTimeString(theLog->timeStamp()); // get log date-time as string

    // make new task run name or use name specified by model run options
    string rn = argOpts().strOption(RunOptionsKey::taskRunName);
    if (rn.empty()) {
        rn = toAlphaNumeric(OM_MODEL_NAME + string("_") + argOpts().strOption(RunOptionsKey::taskName), OM_NAME_DB_MAX);
    }

    // create new task run
    i_dbExec->update(
        "INSERT INTO task_run_lst (task_run_id, task_id, run_name, sub_count, create_dt, status, update_dt, run_stamp)" \
        " VALUES (" +
        to_string(taskRunId) + ", " +
        to_string(i_taskId) + ", " +
        toQuoted(rn) + ", " +
        to_string(subValueCount) + ", " +
        toQuoted(dtStr) + ", " +
        toQuoted(RunStatus::init) + ", " +
        toQuoted(dtStr) + ", " +
        toQuoted(argOpts().strOption(ArgKey::runStamp)) + ")"
    );

    // completed: commit the changes
    i_dbExec->commit();

    setArgOpt(RunOptionsKey::taskRunId, to_string(taskRunId));
    return taskRunId;
}

/** find modeling task, if specified */
int RunController::findTask(IDbExec * i_dbExec)
{
    // find task id or name if specified as run options
    int taskId = argOpts().intOption(RunOptionsKey::taskId, 0);
    string taskName = argOpts().strOption(RunOptionsKey::taskName);

    if (taskId > 0) {
        int cnt = i_dbExec->selectToInt(
            "SELECT COUNT(*) FROM task_lst WHERE task_id = " + to_string(taskId), 0
        );
        if (cnt <= 0) throw DbException("task id not found in database: %d", taskId);
    }
    else {  // find task id by name, if taskName option specified 

        if (!taskName.empty()) {
            taskId = i_dbExec->selectToInt(
                "SELECT MIN(task_id) FROM task_lst WHERE model_id = " + to_string(modelId) +
                " AND task_name = " + toQuoted(taskName),
                0);
            if (taskId <= 0)
                throw DbException("model %s, id: %d does not contain task with name: %s", metaStore->modelRow->name.c_str(), modelId, taskName.c_str());
        }
    }

    // if task found then add it to run options
    if (taskId > 0) {

        if (taskName.empty()) {
            taskName = i_dbExec->selectToStr(
                "SELECT task_name FROM task_lst WHERE task_id = " + to_string(taskId)
            );
        }
        setArgOpt(RunOptionsKey::taskId, to_string(taskId));
        setArgOpt(RunOptionsKey::taskName, taskName);
    }

    return taskId;
}

// create new run, create input parameters and run options for input working sets
// find working set to run the model, it can be:
//  next set of current modeling task
//  set id or set name specified by run options (i.e. command line)
//  default working set for that model
tuple<int, int, ModelStatus> RunController::createNewRun(int i_taskRunId, bool i_isWaitTaskRun, IDbExec * i_dbExec)
{
    // if this is not a part of task then and model status not "initial"
    // then exit with "no more" data to signal all input completed because it was single input set
    ModelStatus mStatus = theModelRunState->status();
    if (i_taskRunId <= 0 && mStatus != ModelStatus::init) return { 0, 0, ModelStatus::shutdown };

    // update in transaction scope
    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    // get next run id
    i_dbExec->update("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_id_set_id'");

    int nRunId = i_dbExec->selectToInt("SELECT id_value FROM id_lst WHERE id_key = 'run_id_set_id'", 0);
    if (nRunId <= 0)
        throw DbException("invalid run id: %d", nRunId);

    // if this is a part of modeling task then find next input set from that task
    int nSetId = 0;
    string dtStr = makeDateTime(chrono::system_clock::now());

    if (i_taskRunId > 0) {
        i_dbExec->update(
            "UPDATE task_run_lst SET status =" \
            " CASE" \
            " WHEN status = 'i' THEN " + toQuoted(i_isWaitTaskRun ? RunStatus::waitProgress : RunStatus::progress) + 
            " ELSE status" \
            " END," +
            " update_dt = " + toQuoted(dtStr) +
            " WHERE task_run_id = " + to_string(i_taskRunId) + 
            " AND status IN ('i', 'p', 'w')"
            );
        i_dbExec->update(
            "INSERT INTO task_run_set (task_run_id, set_id, run_id, task_id)" \
            " SELECT " +
            to_string(i_taskRunId) + ", M.set_id, 0, M.task_id" +
            " FROM" \
            " (" \
            " SELECT MIN(S.set_id) AS \"set_id\", S.task_id" \
            " FROM task_set S" \
            " INNER JOIN task_run_lst RL ON (RL.task_id = S.task_id)" \
            " WHERE RL.task_run_id = " + to_string(i_taskRunId) + 
            " AND RL.status IN  ('i', 'p', 'w')" \
            " AND NOT EXISTS" \
            " (" \
            " SELECT RS.set_id FROM task_run_set RS" \
            " WHERE RS.task_run_id = " + to_string(i_taskRunId) + 
            " AND RS.set_id = S.set_id" +
            " )" \
            " ) AS M" \
            " WHERE NOT M.set_id IS NULL"
            );
        string statCode = i_dbExec->selectToStr(
            "SELECT status FROM task_run_lst WHERE task_run_id = " + to_string(i_taskRunId)
            );
        nSetId = i_dbExec->selectToInt(
            "SELECT set_id FROM task_run_set" \
            " WHERE task_run_id = " + to_string(i_taskRunId) +
            " AND run_id = 0",
            -1);

        // model status (task run status): progress, wait, shutdown or exit
        mStatus = RunState::fromRunStatus(statCode);
        if (mStatus <= ModelStatus::undefined) throw MsgException("invalid (undefined) model run status");

        // no input sets exist: task completed or wait for additional input
        if (nSetId <= 0) {                          

            if (mStatus == ModelStatus::progress || (!i_isWaitTaskRun && mStatus == ModelStatus::init)) {
                mStatus = ModelStatus::shutdown;    // task completed
            }
            i_dbExec->rollback();                   // rollback all database changes
            return { 0, 0, mStatus };               // return empty data set and current model status: wait or exit
        }
    }

    // if this is a task then input set is found at this point and model status is "in progress"
    // else (not a task) then move run status from init to "in progress"
    if (mStatus == ModelStatus::init) mStatus = ModelStatus::progress;

    // find set id and make sure this set exist in database:
    // if this is a task then check is next set of modeling task exist in database
    // else (not a task) then set specified by model run options or it is model default set
    nSetId = findWorkset(nSetId, i_dbExec);

    // make new run name or use name specified by model run options
    string rn = argOpts().strOption(RunOptionsKey::runName);
    if (rn.empty()) {

        rn = OM_MODEL_NAME;
        if (i_taskRunId > 0) {
            string trn = argOpts().strOption(RunOptionsKey::taskRunName);
            if (!trn.empty()) {
                rn = trn;       // if task run name specified then use it as prefix of run name
            }
            else {
                rn += "_" + argOpts().strOption(RunOptionsKey::taskName);   // task run name prefix: modelName_taskName
            }
        }
        rn += "_" + argOpts().strOption(RunOptionsKey::setName);

        rn = toAlphaNumeric(rn, OM_NAME_DB_MAX);
    }

    // create new run entry
    i_dbExec->update(
        "INSERT INTO run_lst" \
        " (run_id, model_id, run_name, sub_count, sub_started, sub_completed, sub_restart, create_dt, status, update_dt, run_digest, run_stamp)" \
        " VALUES (" +
        to_string(nRunId) + ", " +
        to_string(modelId) + ", " +
        toQuoted(rn) + ", " +
        to_string(subValueCount) + ", " +
        to_string(subValueCount) + ", " +
        "0, " +
        "0, " +
        toQuoted(dtStr) + ", " +
        toQuoted(RunStatus::progress) + ", " +
        toQuoted(dtStr) +
        ", NULL, " +
        toQuoted(argOpts().strOption(ArgKey::runStamp)) + ")"
    );

    // if this is a next set of the modeling task then update task progress
    if (i_taskRunId > 0) {
        i_dbExec->update(
            "UPDATE task_run_set SET run_id = " + to_string(nRunId) +
            " WHERE task_run_id = " + to_string(i_taskRunId) + 
            " AND set_id = " + to_string(nSetId) 
            );
    }

    // save run options in run_option table
    createRunOptions(nRunId, i_dbExec);

    // copy input parameters from "base" run and working set into new run id
    createRunParameters(nRunId, nSetId, i_dbExec);

    // copy workset text into run text if workset exists and it is readonly
    i_dbExec->update(
        "INSERT INTO run_txt (run_id, lang_id, descr, note)" \
        " SELECT " + 
        to_string(nRunId) + ", WT.lang_id, WT.descr, WT.note" \
        " FROM workset_lst W" \
        " INNER JOIN workset_txt WT ON (WT.set_id = W.set_id)" \
        " WHERE W.set_id = " + to_string(nSetId) +
        " AND W.is_readonly <> 0");

    // completed: commit the changes
    i_dbExec->commit();

    return { nSetId, nRunId, mStatus };
}

// copy input parameters from working set and "base" run into new run id
// search for input parameter value in following order:
//   use value of parameter specified in run options: (command-line, ini-file, profile_option table)
//   use parameter.csv file if SubFrom.Parameter == csv or by default if parameters csv directory specified
//   use value of parameter from working set of model parameters
//   if working set based on model run then search by base run id to get parameter value
//   else raise an exception
void RunController::createRunParameters(int i_runId, int i_setId, IDbExec * i_dbExec) const
{
    // find input parameters workset
    if (i_setId <= 0) throw DbException("invalid set id or model working sets not found in database");

    // increase read only flag to "lock" workset until parameters copy not done
    string sSetId = to_string(i_setId);
    i_dbExec->update(
        "UPDATE workset_lst SET is_readonly = is_readonly + 1 WHERE set_id = " + sSetId
        );
    int nReadonly = i_dbExec->selectToInt(
        "SELECT is_readonly FROM workset_lst WHERE set_id = " + sSetId,
        0);
    if (nReadonly <= 1) throw DbException("workset must be read-only (set id: %d)", i_setId);

    // workset must exist
    vector<WorksetLstRow> wsVec = IWorksetLstTable::byKey(i_dbExec, i_setId);
    if (wsVec.empty()) throw DbException("workset must exist (set id: %d)", i_setId);

    const WorksetLstRow & wsRow = wsVec[0];

    // validate workset: it must be read-only and must be from the same model
    if (wsRow.modelId != modelId) throw DbException("invalid workset model id: %d, expected: %d (set id: %d)", wsRow.modelId, modelId, i_setId);
    // if (!wsRow.isReadonly) throw DbException("workset must be read-only (set id: %d)", i_setId);

    // get list of model parameters and list of parameters included into workset
    vector<ParamDicRow> paramVec = metaStore->paramDic->rows();
    vector<WorksetParamRow> wsParamVec = IWorksetParamTable::select(i_dbExec, i_setId);

    // if base run does not exist then workset must include all model parameters
    int baseWsRunId = wsRow.baseRunId;
    bool isBaseWsRun = baseWsRunId > 0;
    // if base run does not exist then workset must include all model parameters
    // if (!isBaseRunExist && paramVec.size() != wsParamVec.size()) throw DbException("workset must include all model parameters (set id: %d)", setId);

    // check if parameters csv directory specified and accessible
    string paramDir = argOpts().strOption(RunOptionsKey::paramDir);
    bool isParamDir = !paramDir.empty() && isFileExists(paramDir.c_str());

    // is csv conatins enum id's or enum codes
    bool isIdCsv = argOpts().boolOption(RunOptionsKey::useIdCsv);
    bool isIdValue = argOpts().boolOption(RunOptionsKey::useIdParamValue);

    // copy parameters into destination run, searching it by following order:
    //   from run options (command-line, ini-file, profile_option table)
    //   from parameter.csv file if exist
    //   from workset parameter value table
    //   if workset based on run then as link to base run of workset
    // calculate parameter values digest and store only single copy of parameter values
    string sRunId = to_string(i_runId);
    string sModelId = to_string(modelId);
    string sBaseWsRunId = to_string(baseWsRunId);
    string paramDot = string(RunOptionsKey::parameterPrefix) + ".";

    for (vector<ParamDicRow>::const_iterator paramIt = paramVec.cbegin(); paramIt != paramVec.cend(); ++paramIt) {

        // calculate parameter source: command line (or ini-file), generate "iota", workset, based run, run options
        bool isInserted = false;
        bool isCheckSubCount = false;
        bool isBaseRunFullCopy = false;
        bool isArgOption = argOpts().isOptionExist((paramDot + paramIt->paramName).c_str());
        int nParamSubCount = parameterSubCount(paramIt->paramId);   // if >1 then multiple sub-values expected

        auto wsParamRowIt = WorksetParamRow::byKey(i_setId, paramIt->paramId, wsParamVec);
        bool isExistInWs = wsParamRowIt != wsParamVec.cend();
        int defaultSubId = isExistInWs ? wsParamRowIt->defaultSubId : 0;    // default sub-value id for workset parameter, use 0 if parameter exist in base run

        ParamSubOpts subOpts = subOptsById(*paramIt, nParamSubCount, defaultSubId);
        bool isFromDb = subOpts.from == RunOptionsKey::dbSubValue;
        bool isFromCsv = subOpts.from == RunOptionsKey::csvSubValue;
        bool isFromIota = subOpts.from == RunOptionsKey::iotaSubValue;
        bool isFromDefault = subOpts.from == RunOptionsKey::defaultValue;

        if (!isParamDir && isFromCsv) throw DbException("invalid (empty) parameter.csv file path for parameter: %s", paramIt->paramName.c_str());

        // get dimensions name
        int nRank = paramIt->rank;

        vector<ParamDimsRow> paramDimVec = metaStore->paramDims->byModelIdParamId(modelId, paramIt->paramId);
        if (nRank < 0 || nRank != (int)paramDimVec.size())
            throw DbException("invalid parameter rank or dimensions not found for parameter: %s", paramIt->paramName.c_str());

        string sDimLst = "";
        for (int nDim = 0; nDim < nRank; nDim++) {
            sDimLst += paramDimVec[nDim].name + ", ";
        }

        // insert parameter from run options
        if (isArgOption) {

            // only one sub-value can be supplied on command-line (ini-file, profile table)
            if (nParamSubCount > 1)
                throw DbException("only one sub-value can be provided for parameter: %s", paramIt->paramName.c_str());

            // find parameter type 
            const TypeDicRow * typeRow = metaStore->typeDic->byKey(modelId, paramIt->typeId);
            if (typeRow == nullptr)
                throw DbException("invalid (not found) type of parameter: %s", paramIt->paramName.c_str());

            // get parameter value as sql string 
            // do special parameter value handling if required: 
            // bool conversion to 0/1 or quoted for string or enum code to id
            string sVal = argOpts().strOption((paramDot + paramIt->paramName).c_str());
            if (typeRow->isString()) sVal = toQuoted(sVal);  // "file" type is VARCHAR

            if (typeRow->isBool()) {
                if (!TypeDicRow::isBoolValid(sVal.c_str())) 
                    throw DbException("invalid value of parameter: %s", paramIt->paramName.c_str());

                sVal = TypeDicRow::isBoolTrue(sVal.c_str()) ? "1" : "0";
            }

            if (!typeRow->isBuiltIn() && !isIdValue) {
                const TypeEnumLstRow * enumRow = metaStore->typeEnumLst->byName(modelId, paramIt->typeId, sVal.c_str());
                if (enumRow == nullptr) 
                    throw DbException("invalid value of parameter: %s", paramIt->paramName.c_str());

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

            // find parameter type, it must be numeric
            const TypeDicRow * typeRow = metaStore->typeDic->byKey(modelId, paramIt->typeId);
            if (typeRow == nullptr) throw DbException("invalid (not found) type of parameter: %s", paramIt->paramName.c_str());
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

        // insert from parameter.csv file if sub-value from option = "csv" or from is default and csv directory specified
        if (!isInserted && (isFromCsv || isFromDefault && isParamDir)) {

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

        // copy parameter from workset parameter value table
        // copy parameter value notes from workset parameter text table
        if (!isInserted && (isFromDb || isFromDefault) && isExistInWs) {

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
            string flt = makeWhereSubId(subOpts);
            string subFlds = mapSelectedSubId(subOpts);

            // make source sub_id where clause
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

        // insert parameter values from workset base run:
        // if base run has same number of sub-values 
        // then insert link to parameter values from workset base run
        // else copy only part of source sub-values as new parameter values
        if (!isInserted && (isFromDb || isFromDefault) && isBaseWsRun) {

            // validate: parameter must exist in workset base run and must have enough sub-values
            int nSub = i_dbExec->selectToInt(
                "SELECT sub_count FROM run_parameter" \
                " WHERE run_id = " + sBaseWsRunId + " AND parameter_hid = " + to_string(paramIt->paramHid),
                0);
            if (nSub <= 0)
                throw DbException("parameter %d: %s must be included in base run (id: %d)", paramIt->paramId, paramIt->paramName.c_str(), baseWsRunId);
            if (nSub < nParamSubCount)
                throw DbException("parameter %d: %s must have %d sub-values in base run (id: %d)", paramIt->paramId, paramIt->paramName.c_str(), nParamSubCount, baseWsRunId);

            // if base run has same number of sub-values and it is in range [0, sub count - 1] then copy all else only part of source sub-values
            isBaseRunFullCopy = nParamSubCount == nSub &&
                ((nParamSubCount == 1 && (subOpts.kind == KindSubIds::single && subOpts.subIds[0] == 0 || subOpts.kind == KindSubIds::defaultId) && subOpts.subIds[0] == defaultSubId) ||
                (nParamSubCount > 1 && subOpts.kind == KindSubIds::range && subOpts.subIds[0] == 0 && subOpts.subIds.back() == nParamSubCount - 1));

            if (isBaseRunFullCopy) {            // copy parameter from base run of workset
                i_dbExec->update(
                    "INSERT INTO run_parameter (run_id, parameter_hid, base_run_id, sub_count, run_digest)" \
                    " SELECT " + sRunId + ", parameter_hid, base_run_id, sub_count, run_digest" \
                    " FROM run_parameter"
                    " WHERE run_id = " + sBaseWsRunId +
                    " AND parameter_hid = " + to_string(paramIt->paramHid)
                );
            }
            else {      // copy only required number of sub-values from base run of workset

                // use sub id options to make where sub_id filter and map source sub id's to parameter run sub id's
                string flt = makeWhereSubId(subOpts);
                string subFlds = mapSelectedSubId(subOpts);

                i_dbExec->update(
                    "INSERT INTO " + paramIt->dbRunTable + " (run_id, sub_id, " + sDimLst + " param_value)" +
                    " SELECT " + sRunId + ", " + subFlds + ", " + sDimLst + " param_value" +
                    " FROM " + paramIt->dbRunTable +
                    " WHERE run_id = " + sBaseWsRunId +
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

            // find parameter type
            const ParameterNameSizeItem * endItem = parameterNameSizeArr + PARAMETER_NAME_ARR_LEN;
            const ParameterNameSizeItem * pi = find_if(
                parameterNameSizeArr,
                endItem,
                [&paramIt](const ParameterNameSizeItem & i_item) -> bool { return paramIt->paramName == i_item.name; }
            );
            if (pi == endItem) throw ModelException("parameter %d: %s not found in model parameters", paramIt->paramId, paramIt->paramName.c_str());

            // calculate parameter values digest and store only single copy of parameter values
            unique_ptr<IParameterRunWriter> writer(IParameterRunWriter::create(
                i_runId,
                paramIt->paramName.c_str(),
                i_dbExec,
                meta(),
                argOpts().strOption(RunOptionsKey::doubleFormat).c_str()
            ));
            writer->digestParameter(i_dbExec, nParamSubCount, pi->typeOf);
        }
    }

    // unlock workset
    i_dbExec->update("UPDATE workset_lst SET is_readonly = 1 WHERE set_id = " + sSetId);
}

// make part of where clause to select sub_id's
const string RunController::makeWhereSubId(const MetaLoader::ParamSubOpts & i_subOpts) const
{
    switch (i_subOpts.kind) {
    case KindSubIds::single:
    case KindSubIds::defaultId:
        return "sub_id = " + to_string(i_subOpts.subIds[0]);

    case KindSubIds::range:
        return "sub_id BETWEEN " + to_string(i_subOpts.subIds[0]) + " AND " + to_string(i_subOpts.subIds[i_subOpts.subIds.size() - 1]);

    case KindSubIds::list:
        string flt = "sub_id IN (";
        for (size_t k = 0; k < i_subOpts.subIds.size(); k++) {
            flt += ((k > 0) ? ", " : "") + to_string(i_subOpts.subIds[k]);
        }
        return flt += ")";
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
        string sql = "CASE";
        for (size_t k = 0; k < i_subOpts.subIds.size(); k++) {
            sql += " WHEN sub_id = " + to_string(i_subOpts.subIds[k]) + " THEN " + to_string(k);
        }
        return sql += " END";
    }
    return "sub_id";    // default result is no mapping: return sub_id as is
}

// return parameter sub-value options by parameter id
const MetaLoader::ParamSubOpts RunController::subOptsById(const ParamDicRow & i_paramRow, int i_subCount, int i_defaultSubId) const
{
    // check if sub-value options specified
    ParamSubOpts ps;

    auto psIt = lower_bound(
        subOptsArr.cbegin(), subOptsArr.cend(), ParamSubOpts(i_paramRow.paramId), ParamSubOpts::keyLess
    );
    if (psIt != subOptsArr.cend() && psIt->paramId == i_paramRow.paramId) { // options found, use copy of it
        ps = *psIt;
        if (ps.from == RunOptionsKey::iotaSubValue) return ps;  // iota sub-values, return options as is
    }
    else {  // not found: create default options
        ps.paramId = i_paramRow.paramId;
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

// find id and name of source working set for input parameters:
//   if set id specified as run option then use such set id
//   if set name specified as run option then find set id by name
//   else use min(set id) as default set of model parameters
int RunController::findWorkset(int i_setId, IDbExec * i_dbExec)
{
    // find set id of parameters workset, default is first set id for that model
    int setId = (i_setId > 0) ? i_setId : argOpts().intOption(RunOptionsKey::setId, 0);
    string setName = argOpts().strOption(RunOptionsKey::setName);

    if (setId > 0) {
        int cnt = i_dbExec->selectToInt(
            "SELECT COUNT(*) FROM workset_lst WHERE set_id = " + to_string(setId), 0
        );
        if (cnt <= 0) throw DbException("working set id not found in database: %d", setId);
    }
    else {  // find set id by name, if setName option specified 

        if (!setName.empty()) {
            setId = i_dbExec->selectToInt(
                "SELECT MIN(set_id) FROM workset_lst WHERE model_id = " + to_string(modelId) +
                " AND set_name = " + toQuoted(setName),
                0);
            if (setId <= 0)
                throw DbException("model %s, id: %d does not contain working set with name: %s", metaStore->modelRow->name.c_str(), modelId, setName.c_str());
        }
    }

    // if set id not defined then use default working set for the model
    if (setId <= 0) {
        setId = i_dbExec->selectToInt(
            "SELECT MIN(set_id) FROM workset_lst WHERE model_id = " + to_string(modelId), 0
        );
        if (setId <= 0)
            throw DbException("model %s, id: %d must have at least one working set", metaStore->modelRow->name.c_str(), modelId);
    }

    // update run options: actual set id and set name can be different
    setName = i_dbExec->selectToStr("SELECT set_name FROM workset_lst WHERE set_id = " + to_string(setId));
    setArgOpt(RunOptionsKey::setId, to_string(setId));
    setArgOpt(RunOptionsKey::setName, setName);

    return setId;
}

/** save run options by inserting into run_option table */
void RunController::createRunOptions(int i_runId, IDbExec * i_dbExec) const
{
    // save options in database
    for (NoCaseMap::const_iterator optIt = argOpts().args.cbegin(); optIt != argOpts().args.cend(); optIt++) {

        // skip run id and connection string: it is already in database
        if (optIt->first == RunOptionsKey::restartRunId || optIt->first == RunOptionsKey::dbConnStr) continue;

        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value) VALUES (" +
            to_string(i_runId) + ", " + toQuoted(optIt->first) + ", " + toQuoted(optIt->second) + ")"
        );
    }

    // append "last" log file path, if not explictly specified and path is not empty
    string fp = theLog->lastLogPath();
    if (!argOpts().isOptionExist(ArgKey::logFilePath) && !fp.empty()) {
        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value)" \
            " VALUES (" + to_string(i_runId) + ", " + toQuoted(ArgKey::logFilePath) + ", " + toQuoted(fp.substr(0, OM_OPTION_DB_MAX)) + ")"
        );
    }

    // append "stamped" log file path, if path is not empty
    fp = theLog->stampedLogPath();
    if (!fp.empty()) {
        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value)" \
            " VALUES (" + to_string(i_runId) + ", 'OpenM.LogStampedFilePath', " + toQuoted(fp.substr(0, OM_OPTION_DB_MAX)) + ")"
        );
    }
}
