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

// model run basic support public interface
IRunBase::~IRunBase() throw() { }

/** last cleanup */
RunController::~RunController(void) throw() { }

/** create run controller, load metadata tables and broadcast it to all modeling processes. */
RunController * RunController::create(const ArgReader & i_argOpts, bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec)
{
    int nProcess = i_isMpiUsed ? i_msgExec->worldSize() : 1;    // number of processes: MPI world size

    bool isTask = i_argOpts.intOption(RunOptionsKey::taskId, 0) > 0 || i_argOpts.strOption(RunOptionsKey::taskName) != "";
    bool isRunId = i_argOpts.intOption(RunOptionsKey::restartRunId, 0) > 0;

    // multiple processes: create run controller for root or child process
    // single process run: create single process run controller 
    // "restart run": if no modeling task and run id explicitly specified
    unique_ptr<RunController> ctrl;

    if (nProcess > 1) {
        if (i_msgExec->isRoot()) {
            ctrl.reset(new RootController(nProcess, i_argOpts, i_dbExec, i_msgExec));
        }
        else {
            ctrl.reset(new ChildController(nProcess, i_argOpts, i_msgExec));
        }
    }
    else {
        if (!isTask && isRunId) {
            ctrl.reset(new RestartController(i_argOpts, i_dbExec));
        }
        else {
            ctrl.reset(new SingleController(i_argOpts, i_dbExec));
        }
    }

    // load metadata tables and broadcast it to all modeling processes
    ctrl->init();

    return ctrl.release();
}

/** return index of parameter by name */
int RunController::parameterIdByName(const char * i_name) const
{
    if (i_name == nullptr || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    const ParamDicRow * paramRow = metaStore->paramDic->byModelIdName(modelId, i_name);
    if (paramRow == nullptr) throw DbException("parameter not found in parameters dictionary: %s", i_name);

    return paramRow->paramId;
}

// create new run, create input parameters and run options for input working sets
// find working set to run the model, it can be:
//  next set of current modeling task
//  set id or set name specified by run options (i.e. command line)
//  default working set for that model
RunController::SetRunItem RunController::createNewRun(int i_taskRunId, bool i_isWaitTaskRun, IDbExec * i_dbExec) const
{
    // if this is not a part of task then and model status not "initial"
    // then exit with "no more" data to signal all input completed because it was single input set
    ModelStatus mStatus = theModelRunState.status();
    if (i_taskRunId <= 0 && mStatus != ModelStatus::init) return SetRunItem(0, 0, ModelStatus::shutdown);

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
        string stat = i_dbExec->selectToStr(
            "SELECT status FROM task_run_lst WHERE task_run_id = " + to_string(i_taskRunId)
            );
        nSetId = i_dbExec->selectToInt(
            "SELECT set_id FROM task_run_set" \
            " WHERE task_run_id = " + to_string(i_taskRunId) +
            " AND run_id = 0",
            -1);

        // model status (task run status): progress, wait, shutdown or exit
        mStatus = ModelRunState::fromRunStatus(stat);
        if (mStatus <= ModelStatus::undefined) throw MsgException("invalid (undefined) model run status");

        // no input sets exist: task completed or wait for additional input
        if (nSetId <= 0) {                          

            if (mStatus == ModelStatus::progress || (!i_isWaitTaskRun && mStatus == ModelStatus::init))
                mStatus = ModelStatus::shutdown;    // task completed
            
            i_dbExec->rollback();                   // rollback all database changes
            return SetRunItem(0, 0, mStatus);       // return empty data set and current model status: wait or exit
        }
    }

    // if this is a task then input set is found at this point and model status is "in progress"
    // else (not a task) then move run status from init to "in progress"
    if (mStatus == ModelStatus::init) mStatus = ModelStatus::progress;

    // create new run
    string sn = argOpts().strOption(RunOptionsKey::runName);
    if (sn.empty()) sn = toAlphaNumeric(string(OM_MODEL_NAME) + "_" + dtStr + "_" + to_string(nRunId));

    i_dbExec->update(
        "INSERT INTO run_lst" \
        " (run_id, model_id, run_name, sub_count, sub_started, sub_completed, sub_restart, create_dt, status, update_dt, run_digest)" \
        " VALUES (" +
        to_string(nRunId) + ", " +
        to_string(modelId) + ", " +
        toQuoted(sn) + ", " +
        to_string(subValueCount) + ", " +
        to_string(subValueCount) + ", " +
        "0, " +
        "0, " +
        toQuoted(dtStr) + ", " +
        toQuoted(RunStatus::progress) + ", " +
        toQuoted(dtStr) + 
        ", NULL)"
        );

    // if this is a next set of the modeling task then update task progress
    if (i_taskRunId > 0) {
        i_dbExec->update(
            "UPDATE task_run_set SET run_id = " + to_string(nRunId) +
            " WHERE task_run_id = " + to_string(i_taskRunId) + 
            " AND set_id = " + to_string(nSetId) 
            );
    }

    // find workset: next set of modeling task, set specified by model run options or default set
    nSetId = findWorkset(nSetId, i_dbExec);

    // save run options in run_option table
    createRunOptions(nRunId, nSetId, i_dbExec);

    // copy input parameters from "base" run and working set into new run id
    createRunParameters(nRunId, nSetId, i_dbExec);

    // completed: commit the changes
    i_dbExec->commit();

    return SetRunItem(nSetId, nRunId, mStatus);
}

// create run options and save it in run_option table
void RunController::createRunOptions(int i_runId, int i_setId, IDbExec * i_dbExec) const
{
    // save options in database
    for (NoCaseMap::const_iterator optIt = argOpts().args.cbegin(); optIt != argOpts().args.cend(); optIt++) {

        // skip run id and connection string: it is already in database
        if (optIt->first == RunOptionsKey::restartRunId || optIt->first == RunOptionsKey::dbConnStr) continue;

        // remove sub-value count from run_option, it is stored in run_lst
        // if (optIt->first == RunOptionsKey::subValueCount) continue;

        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value) VALUES (" +
            to_string(i_runId) + ", " + toQuoted(optIt->first) + ", " + toQuoted(optIt->second) + ")"
            );
    }

    // append working set id, if not explictly specified
    if (!argOpts().isOptionExist(RunOptionsKey::setId)) {
        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value)" \
            " SELECT " + to_string(i_runId) + ", " + toQuoted(RunOptionsKey::setId) + ", set_id" + 
            " FROM workset_lst WHERE set_id = " + to_string(i_setId)
            );
    }
    // append working set name, if not explictly specified
    if (!argOpts().isOptionExist(RunOptionsKey::setName)) {
        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value)" \
            " SELECT " + to_string(i_runId) + ", " + toQuoted(RunOptionsKey::setName) + ", set_name" + 
            " FROM workset_lst WHERE set_id = " + to_string(i_setId)
            );
    }
}

// copy input parameters from working set and "base" run into new run id
// search for input parameter value in following order:
//   use value of parameter specified in run options: (command-line, ini-file, profile_option table)
//   use parameter.csv file if parameters csv directory specified
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

    // workset must exist
    vector<WorksetLstRow> wsVec = IWorksetLstTable::byKey(i_dbExec, i_setId);
    if (wsVec.empty()) throw DbException("workset must exist (set id: %d)", i_setId);

    const WorksetLstRow & wsRow = wsVec[0];

    // validate workset: it must be read-only and must be from the same model
    if (!wsRow.isReadonly) throw DbException("workset must be read-only (set id: %d)", i_setId);
    if (wsRow.modelId != modelId)
        throw DbException("invalid workset model id: %d, expected: %d (set id: %d)", wsRow.modelId, modelId, i_setId);

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
    string subValueDot = string(RunOptionsKey::subValuePrefix) + ".";

    for (vector<ParamDicRow>::const_iterator paramIt = paramVec.cbegin(); paramIt != paramVec.cend(); ++paramIt) {

        // calculate parameter source: command line (or ini-file), generate "iota", workset, based run, run options
        bool isInserted = false;
        bool isBaseRunFullCopy = false;
        bool isArgOption = argOpts().isOptionExist((paramDot + paramIt->paramName).c_str());
        bool isIotaSubValues = argOpts().strOption((subValueDot + paramIt->paramName).c_str()) == RunOptionsKey::iotaSubValue;
        bool isExistInWs = WorksetParamRow::byKey(i_setId, paramIt->paramId, wsParamVec) != wsParamVec.cend();

        int nParamSubCount = parameterSubCount(paramIt->paramId);   // if >1 then multiple sub-values expected

        if (!isParamDir && argOpts().strOption((subValueDot + paramIt->paramName).c_str()) == RunOptionsKey::csvSubValue)
            throw
            DbException("invalid (empty) parameter.csv file path for parameter: %s", paramIt->paramName.c_str());

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
        if (!isInserted && isIotaSubValues) {

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

        // insert from parameter.csv file if csv directory specified
        if (!isInserted && isParamDir) {

            // if parameter.csv exist then copy it into parameter value table
            string csvPath = makeFilePath(paramDir.c_str(), paramIt->paramName.c_str(), ".csv");

            if (isFileExists(csvPath.c_str())) {
#ifdef _DEBUG
                theLog->logMsg("Read", csvPath.c_str());
#endif      
                // read from csv file, parse csv lines and insert values into parameter run table
                unique_ptr<IParameterRunWriter> writer(IParameterRunWriter::create(
                    i_runId,
                    paramIt->paramName.c_str(),
                    i_dbExec,
                    metaStore.get()
                ));
                writer->loadCsvParameter(i_dbExec, nParamSubCount, csvPath.c_str(), isIdCsv);

                isInserted = true;
            }
        }

        // copy parameter from workset parameter value table
        // copy parameter value notes from workset parameter text table
        if (!isInserted && isExistInWs) {

            // validate: parameter must exist in workset and must have enough sub-values
            int nSub = i_dbExec->selectToInt(
                "SELECT sub_count FROM workset_parameter" \
                " WHERE set_id = " + sSetId + " AND parameter_hid = " + to_string(paramIt->paramHid),
                0);
            if (nSub <= 0)
                throw DbException("parameter %d: %s must be included in workset (id: %d)", paramIt->paramId, paramIt->paramName.c_str(), i_setId);
            if (nSub < nParamSubCount)
                throw DbException("parameter %d: %s must have %d sub-values in workset (id: %d)", paramIt->paramId, paramIt->paramName.c_str(), nParamSubCount, i_setId);

            i_dbExec->update(
                "INSERT INTO " + paramIt->dbRunTable + " (run_id, sub_id, " + sDimLst + " param_value)" +
                " SELECT " + sRunId + ", sub_id, " + sDimLst + " param_value" +
                " FROM " + paramIt->dbSetTable +
                " WHERE set_id = " + sSetId +
                (nSub == nParamSubCount ?
                    "" :
                    (nParamSubCount <= 1 ?
                        " AND sub_id = 0" :
                        " AND sub_id BETWEEN 0 AND " + to_string(nParamSubCount - 1)
                        )
                    )
            );
            i_dbExec->update(
                "INSERT INTO run_parameter_txt (run_id, parameter_hid, lang_id, note)" \
                " SELECT " + sRunId + ", parameter_hid, lang_id, note" +
                " FROM workset_parameter_txt" +
                " WHERE set_id = " + sSetId +
                " AND parameter_hid = " + to_string(paramIt->paramHid)
                );
            isInserted = true;
        }

        // insert parameter values from workset base run:
        // if base run has same number of sub-values 
        // then insert link to parameter values from workset base run
        // else copy only part of source sub-values as new parameter values
        if (!isInserted && isBaseWsRun) {

            // validate: parameter must exist in workset base run and must have enough sub-values
            int nSub = i_dbExec->selectToInt(
                "SELECT sub_count FROM run_parameter" \
                " WHERE run_id = " + sBaseWsRunId + " AND parameter_hid = " + to_string(paramIt->paramHid),
                0);
            if (nSub <= 0)
                throw DbException("parameter %d: %s must be included in base run (id: %d)", paramIt->paramId, paramIt->paramName.c_str(), baseWsRunId);
            if (nSub < nParamSubCount)
                throw DbException("parameter %d: %s must have %d sub-values in base run (id: %d)", paramIt->paramId, paramIt->paramName.c_str(), nParamSubCount, baseWsRunId);

            // if base run has same number of sub-values then copy all else only part of source sub-values
            isBaseRunFullCopy = nParamSubCount == nSub;

            if (isBaseRunFullCopy) {            // copy parameter from base run of workset
                i_dbExec->update(
                    "INSERT INTO run_parameter (run_id, parameter_hid, base_run_id, sub_count, run_digest)" \
                    " SELECT " + sRunId + ", E.parameter_hid, E.base_run_id, E.sub_count, E.run_digest" \
                    " FROM run_parameter E"
                    " WHERE E.run_id = " + sBaseWsRunId + " AND E.parameter_hid = " + to_string(paramIt->paramHid)
                );
            }
            else {      // copy only required number of sub-values from base run of workset
                i_dbExec->update(
                    "INSERT INTO " + paramIt->dbRunTable + " (run_id, sub_id, " + sDimLst + " param_value)" +
                    " SELECT " + sRunId + ", sub_id, " + sDimLst + " param_value" +
                    " FROM " + paramIt->dbSetTable +
                    " WHERE set_id = " + sSetId +
                    (nParamSubCount <= 1 ?
                        " AND sub_id = 0" :
                        " AND sub_id BETWEEN 0 AND " + to_string(nParamSubCount - 1)
                        )
                );
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
                metaStore.get(),
                argOpts().strOption(RunOptionsKey::doubleFormat).c_str()
            ));
            writer->digestParameter(i_dbExec, nParamSubCount, pi->typeOf);
        }
    }

    // unlock workset
    i_dbExec->update("UPDATE workset_lst SET is_readonly = 1 WHERE set_id = " + sSetId);
}

/** impelementation of model process shutdown if exiting without completion. */
void RunController::doShutdownOnExit(ModelStatus i_status, int i_runId, int i_taskRunId, IDbExec * i_dbExec)
{
    theModelRunState.updateStatus(i_status);    // set model status

    // update run status and task status in database
    {
        unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

        string sDt = makeDateTime(chrono::system_clock::now());

        // if error then update run status only else update run status and run digest
        if (ModelRunState::isError(i_status)) {
            i_dbExec->update(
                "UPDATE run_lst SET status = " + toQuoted(RunStatus::error) + "," +
                " update_dt = " + toQuoted(sDt) +
                " WHERE run_id = " + to_string(i_runId)
            );
        }
        else {
            i_dbExec->update(
                "UPDATE run_lst SET status = " + toQuoted(RunStatus::exit) + "," +
                " update_dt = " + toQuoted(sDt) +
                " WHERE run_id = " + to_string(i_runId)
            );

            string sDigest = IRunLstTable::digestRun(i_dbExec, modelId, i_runId);

            i_dbExec->update(
                "UPDATE run_lst SET run_digest = " + ((!sDigest.empty()) ? toQuoted(sDigest) : "NULL") +
                " WHERE run_id = " + to_string(i_runId)
            );
        }

        // update task run progress
        if (i_taskRunId > 0) {
            i_dbExec->update(
                "UPDATE task_run_lst SET status = " +
                (!ModelRunState::isError(i_status) ? toQuoted(RunStatus::exit) : toQuoted(RunStatus::error)) + "," +
                " update_dt = " + toQuoted(sDt) +
                " WHERE task_run_id = " + to_string(i_taskRunId)
                );
        }
        i_dbExec->commit();
    }
}

/** implementation of model run shutdown. */
void RunController::doShutdownRun(int i_runId, int i_taskRunId, IDbExec * i_dbExec)
{
    // update run status: all sub-values completed at this point
    string sDt = makeDateTime(chrono::system_clock::now());
    {
        unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

        i_dbExec->update(
            "UPDATE run_lst SET status = " + toQuoted(RunStatus::progress) + ", " +
            " sub_completed = sub_count," +
            " sub_restart = sub_count,"
            " update_dt = " + toQuoted(sDt) +
            " WHERE run_id = " + to_string(i_runId)
        );

        // update task progress
        if (i_taskRunId > 0) {
            i_dbExec->update(
                "UPDATE task_run_lst SET update_dt = " + toQuoted(sDt) +
                " WHERE task_run_id = " + to_string(i_taskRunId)
            );
        }

        i_dbExec->commit();
    }

    // calculate output tables aggregated values and mark this run as completed
#ifdef _DEBUG
    theLog->logMsg("Writing Output Tables Expressions");
#endif      
    writeOutputValues(i_runId, i_dbExec);

    // update run status as completed
    sDt = makeDateTime(chrono::system_clock::now());
    {
        unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

        i_dbExec->update(
            "UPDATE run_lst SET status = " + toQuoted(RunStatus::done) + "," +
            " update_dt = " + toQuoted(sDt) +
            " WHERE run_id = " + to_string(i_runId)
        );

        string sDigest = IRunLstTable::digestRun(i_dbExec, modelId, i_runId);

        i_dbExec->update(
            "UPDATE run_lst SET run_digest = " + ((!sDigest.empty()) ? toQuoted(sDigest) : "NULL") +
            " WHERE run_id = " + to_string(i_runId)
        );

        // update task progress
        if (i_taskRunId > 0) {
            i_dbExec->update(
                "UPDATE task_run_lst SET update_dt = " + toQuoted(sDt) +
                " WHERE task_run_id = " + to_string(i_taskRunId)
            );
        }
        i_dbExec->commit();
    }
}

/** implementation model process shutdown: cleanup resources. */
void RunController::doShutdownAll(int i_taskRunId, IDbExec * i_dbExec)
{
    theModelRunState.updateStatus(ModelStatus::done);   // set model status as completed OK

    if (i_taskRunId > 0) {      // update task status as completed
        i_dbExec->update(
            "UPDATE task_run_lst SET status = " + toQuoted(RunStatus::done) + ", " +
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE task_run_id = " + to_string(i_taskRunId)
            );
    }
}

/** write output tables aggregated values into database */
void RunController::writeOutputValues(int i_runId, IDbExec * i_dbExec) const
{
    const vector<TableDicRow> tblVec = metaStore->tableDic->byModelId(modelId);

    for (const TableDicRow & tblRow : tblVec) {
        unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
            i_runId,
            tblRow.tableName.c_str(),
            i_dbExec,
            metaStore.get(),
            subValueCount,
            argOpts().strOption(RunOptionsKey::doubleFormat).c_str()
        ));
        writer->writeAllExpressions(i_dbExec);

        // calculate output table values digest and store only single copy of output values
        writer->digestOutput(i_dbExec);
    }
}

/** write output table accumulators. */
void RunController::doWriteAccumulators(
    int i_runId,
    IDbExec * i_dbExec,
    const RunOptions & i_runOpts,
    const char * i_name,
    size_t i_size,
    forward_list<unique_ptr<double> > & io_accValues
) const
{
    // find output table db row and accumulators
    const TableDicRow * tblRow = metaStore->tableDic->byModelIdName(modelId, i_name);
    if (tblRow == nullptr) throw new DbException("output table not found in table dictionary: %s", i_name);

    // find index of first accumulator: table rows ordered by model id, table id and accumulators id
    int nAcc = (int)metaStore->tableAcc->indexOf(
        [&](const TableAccRow & i_row) -> bool { return i_row.modelId == modelId && i_row.tableId == tblRow->tableId; }
    );
    if (nAcc < 0) throw new DbException("output table accumulators not found: %s", i_name);

    // write accumulators into database
    unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
        i_runId,
        i_name,
        i_dbExec,
        metaStore.get(),
        subValueCount,
        i_runOpts.useSparse,
        i_runOpts.nullValue
    ));

    for (const auto & apc : io_accValues) {
        writer->writeAccumulator(
            i_dbExec, i_runOpts.subValueId, metaStore->tableAcc->byIndex(nAcc)->accId, i_size, apc.get()
        );
        nAcc++;
    }
}

/** update sub-value index to restart the run */
void RunController::updateRestartSubValueId(int i_runId, IDbExec * i_dbExec, size_t i_subRestart) const
{
    // update restart sub-value index
    if (i_subRestart > 0) {
        i_dbExec->update(
            "UPDATE run_lst SET status = " + toQuoted(RunStatus::progress) + "," +
            " sub_restart = " + to_string(i_subRestart) + "," +
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(i_runId)
            );
    }
}
