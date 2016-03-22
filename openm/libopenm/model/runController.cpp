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
    bool isRunId = i_argOpts.intOption(RunOptionsKey::runId, 0) > 0;

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

// create new run, create input parameters and run options for input working sets
// find working set to run the model, it can be:
//  next set of current modeling task
//  set id or set name specified by run options (i.e. command line)
//  default working set for that model
RunController::SetRunItem RunController::createNewRun(int i_taskRunId, bool i_isWaitTaskRun, IDbExec * i_dbExec) const
{
    // find the model
    const ModelDicRow * mdRow = metaStore->modelDic->byKey(modelId);
    if (mdRow == nullptr) throw DbException("model %s not found in the database", OM_MODEL_NAME);

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
    i_dbExec->update(
        "INSERT INTO run_lst" \
        " (run_id, model_id, run_name, sub_count, sub_started, sub_completed, sub_restart, create_dt, status, update_dt)" \
        " VALUES (" +
        to_string(nRunId) + ", " +
        to_string(mdRow->modelId) + ", " +
        toQuoted(string(OM_MODEL_NAME) + " " + dtStr) + ", " +
        to_string(subSampleCount) + ", " +
        to_string(subSampleCount) + ", " +
        "0, " +
        "0, " +
        toQuoted(dtStr) + ", " +
        toQuoted(RunStatus::progress) + ", " +
        toQuoted(dtStr) + ")"
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
    nSetId = findWorkset(nSetId, i_dbExec, mdRow);

    // save run options in run_option table
    createRunOptions(nRunId, nSetId, i_dbExec);

    // copy input parameters from "base" run and working set into new run id
    createRunParameters(nRunId, nSetId, i_dbExec, mdRow);

    // completed: commit the changes
    i_dbExec->commit();

    return SetRunItem(nSetId, nRunId, mStatus);
}

// create run options and save it in run_option table
void RunController::createRunOptions(int i_runId, int i_setId, IDbExec * i_dbExec) const
{
    // save options in database
    for (NoCaseMap::const_iterator propIt = argOpts().args.cbegin(); propIt != argOpts().args.cend(); propIt++) {

        // skip run id and connection string: it is already in database
        if (propIt->first == RunOptionsKey::runId || propIt->first == RunOptionsKey::dbConnStr) continue;

        // remove subsample count from run_option, it is stored in run_lst
        // if (propIt->first == RunOptionsKey::subSampleCount) continue;

        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value) VALUES (" +
            to_string(i_runId) + ", " + toQuoted(propIt->first) + ", " + toQuoted(propIt->second) + ")"
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
//   use value of parameter specified as command line or ini-file argument
//   use value of parameter from profile_option table or any other run options source
//   use value of parameter from working set of model parameters
//   if working set based on model run then search by base run id to get parameter value
//   else raise an exception
void RunController::createRunParameters(int i_runId, int i_setId, IDbExec * i_dbExec, const ModelDicRow * i_modelRow) const
{
    // find input parameters workset
    if (i_setId <= 0) throw DbException("invalid set id or model working sets not found in database");

    // increase read only flag to "lock" workset until parameters copy not done
    i_dbExec->update(
        "UPDATE workset_lst SET is_readonly = is_readonly + 1 WHERE set_id = " + to_string(i_setId)
        );

    // workset must exist
    vector<WorksetLstRow> wsVec = IWorksetLstTable::byKey(i_dbExec, i_setId);
    if (wsVec.empty())
        throw DbException("workset must exist (set id: %d)", i_setId);

    // validate workset: it must be read-only and must be from the same model
    if (!wsVec[0].isReadonly) throw DbException("workset must be read-only (set id: %d)", i_setId);
    if (wsVec[0].modelId != i_modelRow->modelId)
        throw DbException("invalid workset model id: %d, expected: %d (set id: %d)", wsVec[0].modelId, i_modelRow->modelId, i_setId);

    // get list of model parameters and list of parameters included into workset
    vector<ParamDicRow> paramVec = metaStore->paramDic->rows();
    vector<WorksetParamRow> wsParamVec = IWorksetParamTable::select(i_dbExec, i_setId);

    // if base run does not exist then workset must include all model parameters
    int baseRunId = wsVec[0].runId;
    bool isBaseRunExist = baseRunId > 0;
    // if (!isBaseRunExist && paramVec.size() != wsParamVec.size()) throw DbException("workset must include all model parameters (set id: %d)", setId);

    // copy parameters from run options, source set or base run into destination run
    string sRunId = to_string(i_runId);

    for (vector<ParamDicRow>::const_iterator paramIt = paramVec.cbegin(); paramIt != paramVec.cend(); ++paramIt) {

        string paramTblName = i_modelRow->modelPrefix + i_modelRow->paramPrefix + paramIt->dbNameSuffix;
        string setTblName = i_modelRow->modelPrefix + i_modelRow->setPrefix + paramIt->dbNameSuffix;
        string argName = string(RunOptionsKey::parameterPrefix) + "." + paramIt->paramName;

        // calculate parameter source: command line (or ini-file), workset, based run, run options
        bool isFromSet = WorksetParamRow::byKey(i_setId, paramIt->paramId, wsParamVec) != wsParamVec.cend();
        bool isArgOption = argOpts().isOptionExist(argName.c_str());

        // get dimensions name
        int nRank = paramIt->rank;

        vector<ParamDimsRow> paramDimVec = metaStore->paramDims->byModelIdParamId(i_modelRow->modelId, paramIt->paramId);
        if (nRank < 0 || nRank != (int)paramDimVec.size())
            throw DbException("invalid parameter rank or dimensions not found for parameter: %s", paramIt->paramName.c_str());

        string sDimLst = "";
        for (int nDim = 0; nDim < nRank; nDim++) {
            sDimLst += paramDimVec[nDim].name + ", ";
        }

        // execute insert to copy parameter from run parameters, workset or base run
        bool isInserted = false;
        if (isArgOption) {

            // find parameter type 
            const TypeDicRow * typeRow = metaStore->typeDic->byKey(i_modelRow->modelId, paramIt->typeId);
            if (typeRow == nullptr)
                throw DbException("invalid (not found) type of parameter: %s", paramIt->paramName.c_str());

            // get parameter value as sql string 
            string sVal = argOpts().strOption(argName.c_str());
            if (equalNoCase(typeRow->name.c_str(), "file")) sVal = toQuoted(sVal);  // "file" type is VARCHAR

                                                                                    // insert the value
            i_dbExec->update(
                "INSERT INTO " + paramTblName + " (run_id, value) VALUES (" + sRunId + ", " + sVal + ")"
                );
            isInserted = true;
        }
        if (!isInserted && isFromSet) {
            i_dbExec->update(
                "INSERT INTO " + paramTblName + " (run_id, " + sDimLst + " value)" +
                " SELECT " + sRunId + ", " + sDimLst + " value" +
                " FROM " + setTblName + " WHERE set_id = " + to_string(i_setId)
                );
            isInserted = true;
        }
        if (!isInserted && isBaseRunExist) {
            i_dbExec->update(
                "INSERT INTO " + paramTblName + " (run_id, " + sDimLst + " value)" +
                " SELECT " + sRunId + ", " + sDimLst + " value" +
                " FROM " + paramTblName + " WHERE run_id = " + to_string(baseRunId)
                );
            isInserted = true;
        }
        if (!isInserted) {
            if (nRank <= 0)
                throw DbException("parameter %d not found: %s, it must specified as model run option or be included in workset (set id: %d), ", paramIt->paramId, paramIt->paramName.c_str(), i_setId);
            else
                throw DbException("parameter %d not found: %s, workset (set id: %d) must include this parameter", paramIt->paramId, paramIt->paramName.c_str(), i_setId);
        }
    }

    // unlock workset
    i_dbExec->update("UPDATE workset_lst SET is_readonly = 1 WHERE set_id = " + to_string(i_setId));
}

/** impelementation of model process shutdown if exiting without completion. */
void RunController::doShutdownOnExit(ModelStatus i_status, int i_runId, int i_taskRunId, IDbExec * i_dbExec)
{
    theModelRunState.updateStatus(i_status);    // set model status

    // update run status and task status in database
    {
        unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

        string sDt = makeDateTime(chrono::system_clock::now());

        i_dbExec->update(
            "UPDATE run_lst SET status = " +
            (!ModelRunState::isError(i_status) ? toQuoted(RunStatus::exit) : toQuoted(RunStatus::error)) + "," +
            " update_dt = " + toQuoted(sDt) +
            " WHERE run_id = " + to_string(i_runId)
            );

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
    // update run status: all subsamples completed at this point
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
            modelId,
            i_runId,
            tblRow.tableName.c_str(),
            i_dbExec,
            metaStore.get(),
            subSampleCount
            ));
        writer->writeAllExpressions(i_dbExec);
    }
}

/** write output table accumulators. */
void RunController::doWriteAccumulators(
    int i_runId,
    IDbExec * i_dbExec,
    const RunOptions & i_runOpts,
    const char * i_name,
    long long i_size,
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
        modelId,
        i_runId,
        i_name,
        i_dbExec,
        metaStore.get(),
        subSampleCount,
        i_runOpts.useSparse,
        i_runOpts.nullValue
        ));

    for (const auto & apc : io_accValues) {
        writer->writeAccumulator(
            i_dbExec, i_runOpts.subSampleNumber, metaStore->tableAcc->byIndex(nAcc)->accId, i_size, apc.get()
            );
        nAcc++;
    }
}

/** update subsample number to restart the run */
void RunController::updateRestartSubsample(int i_runId, IDbExec * i_dbExec, size_t i_subRestart) const
{
    // update restart subsample number
    if (i_subRestart > 0) {
        i_dbExec->update(
            "UPDATE run_lst SET status = " + toQuoted(RunStatus::progress) + "," +
            " sub_restart = " + to_string(i_subRestart) + "," +
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(i_runId)
            );
    }
}
