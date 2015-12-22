/**
* @file
* OpenM++ modeling library: run controller class to create new model run and support data exchange
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"

using namespace std;
using namespace openm;

/** model run controler: create new run and input parameters in database and support data exchange. 
*
* - (a) determine run id and subsamples calculated by current process
* - (b) if "root" process then create new "run" in database (if required)
* - (c) if "root" process then find id of source working set for input parameters
* - (d) if "root" process then prepare model input parameters
* - (e) if "root" process then prepare to receive accumulators data from to child processes
*
* (a) determine run id and number of subsamples calculated by current process
* ---------------------------------------------------------------------------
* Typically new run id and run input parameters created unless run id specified (see "custom run" below)
*
* If number of modelling threads by default = 1 subsamples run sequentially in single thread.
* If more threads specified (i.e. by command-line argument) then subsamples run in parallel.
*
* For example: \n
*   model.exe -General.Subsamples 8 \n
*   model.exe -General.Subsamples 8 -General.Threads 4 \n
*   mpiexec -n 2 model.exe -General.Subsamples 31 -General.Threads 7
*
* If MPI not used then number of subsamples calculated by process is same as total number of subsamples
*
* If MPI is used then
*
*   number of modelling processes = MPI world size \n
*   number of subsamples per process = total number of subsamples / number of processes \n
*   if total number of subsamples % number of processes != 0 then remainder calculated at root process \n
*
* "Custom run" case scenario:
*
*   run id > 0 expliictly specified as run option (i.e. on command line) \n
*   run_id key exist in run_lst table \n
*   modelling process will calculate only one subsample and exit \n
*   total number of subsamples = sub_count column value of run_lst table \n
*   process subsample number = sub_started column value of run_lst table.
*       
* (b) create new "run" in database (if required)
* ----------------------------------------------
* "root" process (process with subsample number =0) creates "new run" in database:
* 
*   - insert new row with run_id key into run_lst (except of "custom run" case)  
*   - insert run options into run_option table under run_id  
*   - check destination run id:
*       - it must be newly created run
*       - or empty existing run: in run_lst table value of sub_completed =0
*   - save run options in database (if required)
*
* (c) find id of source working set for input parameters
* ------------------------------------------------------
* use following to find input parameters set id: \n
*   - if task id or task name specified then find task id in task_lst \n
*   - if set id specified as run option then use such set id \n
*   - if set name specified as run option then find set id by name \n
*   - else use min(set id) as default set of model parameters
*
* (d) prepare model input parameters
* ----------------------------------
* it creates a copy of input paramters from source working set under destination run_id
* 
* search for input parameter value in following order: \n
*   - use value of parameter specified as command line or ini-file argument
*   - use value of parameter from working set of model parameters
*   - use value of parameter from profile_option table or any other run options source
*   - if working set based on model run then search by base run id to get parameter value
*   - else raise an exception
*
*   any scalar parameter value can be overriden by model run option with "Parameter" prefix \n
*   for example, command line: \n
*       model.exe -Parameter.Population 1234 \n
*   means input parameter with name "Population" will be =1234 \n
*   in that case working set value of "Population" input parameter ignored \n
*   because command line options have higher priority than database values.
*/
int RunController::initRun(bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, MetaRunHolder * io_metaStore)
{
    // if this is main (root) process then find input workset to run
    if (!i_isMpiUsed || i_msgExec->isRoot()) {

        if (!nextSetId(i_dbExec, io_metaStore)) {   // all sets from the modelling task completed

            runId = 0;
            if (i_isMpiUsed) broadcastRunId(i_msgExec, &runId);

            return runId;
        }

        // get next set id to run or use id  of "custom run" (must exist in database)
        runId = (taskId <= 0) ? argStore.intOption(RunOptionsKey::runId, 0) : 0;
    }
    else {
        runId = 0;  // temporary work-in-progress
    }

    // determine number of subsamples and first subsample number:
    // if run id = 0 then it is "normal" new model run case 
    // else this is "custom run" case where run id already exist in database
    if (runId == 0) {

        subPerProcess = subSampleCount / processCount;

        subFirstNumber = 0;
        if (i_isMpiUsed) {
            subFirstNumber = 
                i_msgExec->isRoot() ? (processCount - 1) * subPerProcess : (i_msgExec->rank() - 1) * subPerProcess;
        }

        selfSubCount = subPerProcess;
        if (i_isMpiUsed && i_msgExec->isRoot()) selfSubCount = subSampleCount - subFirstNumber;


        if (subFirstNumber < 0 || subPerProcess <= 0 || selfSubCount <= 0 || subFirstNumber + selfSubCount > subSampleCount)
            throw ModelException(
                "Invalid first subsample number: %d or number of subsamples: %d or subsamples per process: %d", subFirstNumber, selfSubCount, subPerProcess
                );
    }
    else {  
        if (runId < 0) throw ModelException("Invalid run id: %d", runId);
        if (processCount != 1) throw ModelException("Invalid combination of run id %d and number of processes: %d", runId, processCount);

        subPerProcess = selfSubCount = 1;   // "custom run": if run id specified then only one subsample calculated
    }

    // if this is main (root) process then create new run in database
    if (!i_isMpiUsed || i_msgExec->isRoot()) {

        // create new run or next subsample for existing run
        createRunSubsample(i_isMpiUsed, i_dbExec, i_msgExec, io_metaStore);

        // load run_option table rows
        io_metaStore->runOption.reset(IRunOptionTable::create(i_dbExec, runId));
    }

    // broadcast metadata: run id and other run options from root to all other processes
    if (i_isMpiUsed) {
        broadcastRunId(i_msgExec, &runId);  // temporary: work-in-porogress
        if (runId <= 0) return runId;       // temporary: work-in-porogress

        broadcastRunOptions(i_msgExec, io_metaStore->runOption);
    }

    // create list of accumulators to be received from child modelling processes 
    if (i_isMpiUsed && i_msgExec->isRoot()) {
        initAccReceiveList(io_metaStore);
    }

    return runId;
}

// find working set to run the model
// it may be next set of current modelling task or specified by run options (i.e. command line)
bool RunController::nextSetId(IDbExec * i_dbExec, const MetaRunHolder * i_metaStore)
{
    // find the model
    const ModelDicRow * mdRow = i_metaStore->modelDic->byKey(modelId);
    if (mdRow == nullptr) throw DbException("model %s not found in the database", OM_MODEL_NAME);

    // if this is a modelling task then find next set without run
    if (taskId > 0) {
        const auto taskItemIt =
            find_if(
                taskRunLst.cbegin(),
                taskRunLst.cend(),
                [](const TaskItem & i_item) -> bool { return i_item.runId == 0; }
        );

        if (taskItemIt == taskRunLst.cend()) return false;  // all sets completed

        setId = taskItemIt->setId;
        argStore.args[RunOptionsKey::setId] = to_string(setId);
        if (setId <= 0)
            throw DbException("invalid set id: %d in the task: %d of the model %s, id: %d", setId, taskId, mdRow->name.c_str(), mdRow->modelId);
    }
    else {
        if (runId > 0) return false;    // it is not a modelling task and run already completed
    }
    
    setId = findWorkset(i_dbExec, mdRow);
    return true;
}

// create new run or next subsample for existing run
// create input parameters and run options for all input working sets
void RunController::createRunSubsample(
    bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, const MetaRunHolder * i_metaStore
    )
{
    if (i_dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // get model info
    const ModelDicRow * mdRow = i_metaStore->modelDic->byKey(modelId);
    if (mdRow == nullptr) throw DbException("model %s not found in the database by id: %d", OM_MODEL_NAME, modelId);

    // update in transaction scope
    unique_lock<recursive_mutex> lck = i_dbExec->beginTransactionThreaded();

    // create new run:
    // it must be done for root MPI process or if MPI not used and run id not specified
    bool isNewRunCreated = false;
    bool isFirstRunCustom = false;

    if ((i_isMpiUsed && i_msgExec->isRoot()) || (!i_isMpiUsed && runId <= 0)) {

        // get next run id
        i_dbExec->update("UPDATE id_lst SET id_value = id_value + 1 WHERE id_key = 'run_id_set_id'");

        runId = i_dbExec->selectToInt("SELECT id_value FROM id_lst WHERE id_key = 'run_id_set_id'", 0);
        if (runId <= 0)
            throw DbException("invalid run id: %d", runId);

        string dtStr = makeDateTime(chrono::system_clock::now());
        string sRunId = to_string(runId);

        // create new run
        i_dbExec->update(
            "INSERT INTO run_lst (run_id, model_id, run_name, sub_count, sub_started, sub_completed, create_dt, status, update_dt)" \
            " VALUES (" +
            sRunId + ", " +
            to_string(modelId) + ", " +
            toQuoted(string(OM_MODEL_NAME) + " " + dtStr) + ", " +
            to_string(subSampleCount) + ", " +
            to_string(subSampleCount) + ", " +
            "0, " +
            toQuoted(dtStr) + ", "
            "'p', " +
            toQuoted(dtStr) + ")"
            );
        isNewRunCreated = true;

        // if this is a next set of the modelling task then update task progress
        if (taskId > 0) {
            
            // update task item list status
            auto taskItemIt =
                find_if(
                    taskRunLst.begin(),
                    taskRunLst.end(),
                    [this](const TaskItem & i_item) -> bool { return i_item.setId == setId; }
            );
            if (taskItemIt == taskRunLst.end())
                throw DbException("invalid set id: %d in the task: %d of the model %s, id: %d", setId, taskId, mdRow->name.c_str(), mdRow->modelId);

            taskItemIt->runId = runId;
            taskItemIt->state.statusIfNotExit(ModelStatus::progress);

            // update task progress in database
            i_dbExec->update(
                "UPDATE task_run_lst SET status = 'p', update_dt = " + toQuoted(dtStr) + 
                "  WHERE task_run_id = " + to_string(taskRunId)
                );
            i_dbExec->update(
                "INSERT INTO task_run_set (task_id, set_id, run_id, task_run_id)" \
                " VALUES (" +
                to_string(taskId) + ", " +
                to_string(setId) + ", " +
                sRunId + ", " +
                to_string(taskRunId) + ")"
                );
        }
    }
    else { // create next subsample: if MPI is not used and number of subsamples >1 then run id must already exist in database

        if (runId <= 0)  throw DbException("invalid run id: %d", runId);

        // actual subsample number: get next subsample number from database
        string sRunId = to_string(runId);
        i_dbExec->update(
            "UPDATE run_lst SET status = 'p', sub_started = sub_started + 1 WHERE run_id = " + sRunId
            );

        subFirstNumber = i_dbExec->selectToInt("SELECT sub_started - 1 FROM run_lst WHERE run_id = " + sRunId, -1);
        if (subFirstNumber < 0)
            throw DbException("invalid run id: %d", runId);     // run id not found

        int nSubCount = i_dbExec->selectToInt("SELECT sub_count FROM run_lst WHERE run_id = " + sRunId, 0);
        if (subSampleCount > 1 && nSubCount != subSampleCount)
            throw DbException("invalid number of sub-samples: %d, it must be %d (run id: %d)", subSampleCount, nSubCount, runId);

        subSampleCount = nSubCount;     // actual number of subsamples

        if (subFirstNumber >= subSampleCount)
            throw DbException("invalid sub-sample number: %d, it must be less than %d (run id: %d)", subFirstNumber, subSampleCount, runId);

        // validate destination run: it must be new run or empty run
        if (subFirstNumber == 0) {
            int nCompleted = i_dbExec->selectToInt("SELECT sub_completed FROM run_lst WHERE run_id = " + sRunId, 0);
            if (nCompleted != 0)
                throw DbException("destination run must be empty, but it already have %d completed subsamples (run id: %d)", nCompleted, runId);
            isFirstRunCustom = true;
        }
    }

    // if new run created (ie: MPI root process)
    // save run options in run_option table
    // copy input parameters from working set and "base" run into new run id
    if (isNewRunCreated || isFirstRunCustom) {

        // save run options in run_option table
        createRunOptions(isFirstRunCustom, i_dbExec);

        // copy input parameters from "base" run and working set into new run id
        createRunParameters(isNewRunCreated, i_dbExec, mdRow, i_metaStore);
    }

    // completed: commit the changes
    i_dbExec->commit();
}

// create run options and save it in run_option table
void RunController::createRunOptions(bool i_isFirstRunCustom, IDbExec * i_dbExec)
{
    // save options in database
    for (NoCaseMap::const_iterator propIt = argStore.args.cbegin(); propIt != argStore.args.cend(); propIt++) {

        // skip run id and connection string: it is already in database
        if (propIt->first == RunOptionsKey::runId || propIt->first == RunOptionsKey::dbConnStr) continue;

        // remove subsample count from run_option, it is stored in run_lst
        // if (propIt->first == RunOptionsKey::subSampleCount) continue;

        // remove set id and set name for custom run: set is unknown for custom runs
        if (i_isFirstRunCustom && 
            (propIt->first == RunOptionsKey::setId || propIt->first == RunOptionsKey::setName)) continue;

        i_dbExec->update(
            "INSERT INTO run_option (run_id, option_key, option_value) VALUES (" +
            to_string(runId) + ", " + toQuoted(propIt->first) + ", " + toQuoted(propIt->second) + ")"
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
void RunController::createRunParameters(
    bool i_isNewRunCreated, IDbExec * i_dbExec, const ModelDicRow * i_mdRow, const MetaRunHolder * i_metaStore
    )
{
    // find input parameters workset
    if (setId <= 0) throw DbException("invalid set id or model working sets not found in database");

    // increase read only flag to "lock" workset until parameters copy not done
    i_dbExec->update(
        "UPDATE workset_lst SET is_readonly = is_readonly + 1 WHERE set_id = " + to_string(setId)
        );

    // workset must exist
    vector<WorksetLstRow> wsVec = IWorksetLstTable::byKey(i_dbExec, setId);
    if (wsVec.empty()) 
        throw DbException("workset must exist (set id: %d)", setId);

    // validate workset: it must be read-only and must be from the same model
    if (!wsVec[0].isReadonly) throw DbException("workset must be read-only (set id: %d)", setId);
    if (wsVec[0].modelId != modelId)
        throw DbException("invalid workset model id: %d, expected: %d (set id: %d)", wsVec[0].modelId, modelId, setId);

    // get list of model parameters and list of parameters included into workset
    vector<ParamDicRow> paramVec = i_metaStore->paramDic->rows();
    vector<WorksetParamRow> wsParamVec = IWorksetParamTable::select(i_dbExec, setId);

    // if base run does not exist then workset must include all model parameters
    int baseRunId = wsVec[0].runId;
    bool isBaseRunExist = baseRunId > 0;
    // if (!isBaseRunExist && paramVec.size() != wsParamVec.size()) throw DbException("workset must include all model parameters (set id: %d)", setId);

    // copy parameters from run options, source set or base run into destination run
    string sRunId = to_string(runId);

    for (vector<ParamDicRow>::const_iterator paramIt = paramVec.cbegin(); paramIt != paramVec.cend(); ++paramIt) {

        string paramTblName = i_mdRow->modelPrefix + i_mdRow->paramPrefix + paramIt->dbNameSuffix;
        string setTblName = i_mdRow->modelPrefix + i_mdRow->setPrefix + paramIt->dbNameSuffix;
        string argName = string(RunOptionsKey::parameterPrefix) + "." + paramIt->paramName;

        // skip if parameter already exists for destination run
        if (!i_isNewRunCreated) {
            if (0 < i_dbExec->selectToLong("SELECT COUNT(*) FROM " + paramTblName + "  WHERE run_id = " + sRunId, 0)) continue;
        }

        // calculate parameter source: command line (or ini-file), workset, based run, run options
        bool isFromSet = WorksetParamRow::byKey(setId, paramIt->paramId, wsParamVec) != wsParamVec.cend();
        bool isArgOption = argStore.isOptionExist(argName.c_str());

        // get dimensions name
        int nRank = paramIt->rank;

        vector<ParamDimsRow> paramDimVec = i_metaStore->paramDims->byModelIdParamId(modelId, paramIt->paramId);
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
            const TypeDicRow * typeRow = i_metaStore->typeDic->byKey(modelId, paramIt->typeId);
            if (typeRow == nullptr)
                throw DbException("invalid (not found) type of parameter: %s", paramIt->paramName.c_str());

            // get parameter value as sql string 
            string sVal = argStore.strOption(argName.c_str());
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
                " FROM " + setTblName + " WHERE set_id = " + to_string(setId)
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
                throw DbException("parameter %d not found: %s, it must specified as model run option or be included in workset (set id: %d), ", paramIt->paramId, paramIt->paramName.c_str(), setId);
            else
                throw DbException("parameter %d not found: %s, workset (set id: %d) must include this parameter", paramIt->paramId, paramIt->paramName.c_str(), setId);
        }
    }

    // unlock workset
    i_dbExec->update("UPDATE workset_lst SET is_readonly = 1 WHERE set_id = " + to_string(setId));
}

// create list of accumulators to be received from child modelling processes
// root process calculate last subsamples
void RunController::initAccReceiveList(const MetaRunHolder * i_metaStore)
{
    if (subSampleCount <= subPerProcess) return;    // exit if all subsamples was produced at root model process

    const vector<TableAccRow> accVec = i_metaStore->tableAcc->byModelId(modelId);

    int tblId = -1;
    long long valSize = 0;
    for (int nAcc = 0; nAcc < (int)accVec.size(); nAcc++) {

        // get accumulator data size
        if (tblId != accVec[nAcc].tableId) {
            tblId = accVec[nAcc].tableId;
            valSize = IOutputTableWriter::sizeOf(modelId, i_metaStore, tblId);
        }

        for (int nSub = 0; nSub < subFirstNumber; nSub++) {
            accRecvVec.push_back(
                AccReceiveItem(nSub, subSampleCount, subPerProcess, tblId, accVec[nAcc].accId, nAcc, valSize)
                );
        }
    }
}

/** receive outstanding accumulators and wait until outstanding send completed. */
void RunController::receiveSendLast(
    bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, const MetaRunHolder * i_metaStore
    )
{
    // receive outstanding subsamples
    if (i_isMpiUsed && i_msgExec->isRoot()) {

        bool isAnyToRecv = true;
        do {
            isAnyToRecv = receiveSubSamples(i_isMpiUsed, i_dbExec, i_msgExec, i_metaStore);

            if (isAnyToRecv) this_thread::sleep_for(chrono::milliseconds(OM_RECV_SLEEP_TIME));
        }
        while (isAnyToRecv);

        // temporary: work-in-progress
        accRecvVec.clear();
    }

    // wait for send completion, if any outstanding
    i_msgExec->waitSendAll();
}


/** receive accumulators of output tables subsamples and write into database.
*
* @return  true if not all accumulators for all subsamples received yet (if data not ready)
*/
bool RunController::receiveSubSamples(
    bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec, const MetaRunHolder * i_metaStore
    )
{
    if (!i_isMpiUsed || !i_msgExec->isRoot()) return false; // exit if this is not a root process
    if (accRecvVec.empty()) return false;                   // exit if nothing to receive

    // get sparse settings
    bool isSparse = i_metaStore->runOption->boolValue(RunOptionsKey::useSparse);
    double nullValue = i_metaStore->runOption->doubleValue(RunOptionsKey::sparseNull, DBL_EPSILON);

    // try to receive and save accumulators
    bool isReceived = false;
    for (AccReceiveItem & accRecv : accRecvVec) {

        if (!accRecv.isReceived) {      // if accumulator not yet received

            // allocate buffer to receive the data
            unique_ptr<double> valueUptr(new double[(int)accRecv.valueSize]);
            double * valueArr = valueUptr.get();

            // try to received
            accRecv.isReceived = i_msgExec->tryReceive(
                accRecv.senderRank, (MsgTag)accRecv.msgTag, typeid(double), accRecv.valueSize, valueArr
                );
            if (!accRecv.isReceived) continue;

            // accumulator received: write it into database
            const TableDicRow * tblRow = i_metaStore->tableDic->byKey(modelId, accRecv.tableId);
            if (tblRow == nullptr) throw new DbException("output table not found in table dictionary, id: %d", accRecv.tableId);

            unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
                modelId,
                runId,
                tblRow->tableName.c_str(),
                i_dbExec,
                i_metaStore,
                subSampleCount,
                isSparse,
                nullValue
                ));
            writer->writeAccumulator(i_dbExec, accRecv.subNumber, accRecv.accId, accRecv.valueSize, valueArr);

            isReceived = true;
        }
    }

    // check if anything left to receive
    bool isAnyToRecv = std::any_of(
        accRecvVec.cbegin(),
        accRecvVec.cend(),
        [](AccReceiveItem i_recv) -> bool { return !i_recv.isReceived; }
    );

    // no data received: if any accumulators outstanding then sleep before try again
    if (!isReceived && isAnyToRecv) this_thread::sleep_for(chrono::milliseconds(OM_RECV_SLEEP_TIME));

    return isAnyToRecv;
}
