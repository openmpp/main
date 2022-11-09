/**
* @file
* OpenM++ modeling library: 
* root process run controller class to create new model run(s), send input parameters to children and receieve output tables.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "helper.h"
#include "model.h"
#include "modelHelper.h"
#include "runControllerImpl.h"

using namespace std;
using namespace openm;

/** initialize root modeling process.
*
* - (a) get number of sub-values, processes, threads, groups
* - (b) merge command line and ini-file options with db profile table
* - (c) initialize task run if required
* - (d) load metadata tables from database and broadcast it to child processes
*
* (a) get number of sub-values, processes, threads, groups
* --------------------------------------------------------
*
* Number of sub-values by default = 1 and it can be specified
* using command-line argument, ini-file or profile table in database
*
* Number of modeling threads by default = 1 and sub-values run sequentially in single thread.
* If more threads specified (i.e. by command-line argument) then sub-values run in parallel.
*
* For example: \n
*   model.exe -OpenM.SubValues 8 \n
*   model.exe -OpenM.SubValues 8 -OpenM.Threads 4 \n
*   mpiexec -n 2 model.exe -OpenM.SubValues 31 -OpenM.Threads 7
*
* Number of modeling processes = MPI world size
*
* Processes may be combined into groups for parallel run of input data sets
* and size of the group is such to calculate all sub-values by the group.
*
* Size of modeling group = number of sub-values / number of threads \n
*   group size always >= 1 therefore group always include at least one process \n
* Number of groups = number of processes / group size \n
*   by default is only one group, which include all modeling processes \n
*
* Root process may or may not be be used for modeling \n
*   if (group size * group count) == (process count) then "root is active" \n
*   else root process dedicated for data exchange and modeling done only by child processes \n
*   if run option "NotOnRoot" is true then no modeling done at root process
*
* Number of sub-values per process = total number of sub-values / group size \n
*   if total number of sub-values % number of processes != 0 then remainder calculated at group last process \n
*
* (b) merge command line and ini-file options with db profile table
*------------------------------------------------------------------
* model run options can be specified as (in order of priority):
*
*   - command line arguments, i.e.: \n
*       model.exe -Parameter.Population 1234
*
*   - through ini-file: \n
*       model.exe -s modelOne.ini
*
*   - as rows of profile_option table: \n
*     profile name can be command line argument or ini-file entry, i.e.: \n
*       model.exe -OpenM.Profile TestProfile
*
*   - some options have hard coded default values which used if nothing above is specified
*
*   run options which name starts with "Parameter." are treated as
*   value of scalar input parameter (see below). \n
*   if there is no input parameter with such name then exception raised. \n
*   for example, if command line is: \n
*       model.exe -Parameter.Population 1234 \n
*   and model does not have "Population" parameter then execution aborted.
*
* (c) initialize task run
* -----------------------
* modeling task can be specified by name or by task id:
*
*   model.exe -OpenM.SubValues 16 -OpenM.TaskName someTask \n
*   model.exe -OpenM.SubValues 16 -OpenM.Taskid 1 \n
*   model.exe -OpenM.SubValues 16 -OpenM.TaskName someTask -OpenM.TaskWait true
*
* if task specified then model would run for each input working set of parameters \n
* if task "wait" is true then modeling task is run under external supervision: \n
*   - model must wait until task status set as "completed" by other external process \n
*   - external process can append new inputs into task_set table and model will continue to run \n
*/
void RootController::init(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // load metadata table rows, except of run_option, which is may not created yet
    // load messages from database
    metaStore.reset(new MetaHolder);
    modelId = readMetaTables(dbExec, metaStore.get(), OM_MODEL_NAME, OM_MODEL_DIGEST);
    loadMessages(dbExec);

    // merge command line and ini-file arguments with profile_option table values
    mergeOptions(dbExec);

    // get main run control values: number of sub-values, threads
    subValueCount = argOpts().intOption(RunOptionsKey::subValueCount, 1);   // number of sub-values from command line or ini-file
    threadCount = argOpts().intOption(RunOptionsKey::threadCount, 1);       // max number of modeling threads
    bool isRootIdle = argOpts().boolOption(RunOptionsKey::notOnRoot);       // if true then do not run modeling threads at root process
    isWaitTaskRun = argOpts().boolOption(RunOptionsKey::taskWait);          // if true then task run under external supervision

    // broadcast basic run options from root to all other processes
    int nRootIdle = isRootIdle ? 1 : 0;
    msgExec->bcastInt(ProcessGroupDef::all, &subValueCount);
    msgExec->bcastInt(ProcessGroupDef::all, &threadCount);
    msgExec->bcastInt(ProcessGroupDef::all, &nRootIdle);
    msgExec->bcastInt(ProcessGroupDef::all, &modelId);

    // basic validation: number of processes expected to be > 1
    if (subValueCount <= 0) throw ModelException("Invalid number of sub-values: %d", subValueCount);
    if (threadCount <= 0) throw ModelException("Invalid number of modeling threads: %d", threadCount);
    if (processCount <= 1) throw ModelException("Invalid number of modeling processes: %d", processCount);

    // create groups for parallel run of modeling task
    // "is root active": root may be used for modeling in last group
    // if (group size * group count) == process count then "root is active"
    // else root process dedicated for data exchange and modeling done only by child processes
    rootGroupDef = ProcessGroupDef(subValueCount, threadCount, isRootIdle, msgExec->worldSize(), msgExec->rank());

    msgExec->createGroups(rootGroupDef.groupSize, rootGroupDef.groupCount);

    for (int nGroup = 0; nGroup < rootGroupDef.groupCount; nGroup++) {
        runGroupLst.emplace_back(1 + nGroup, subValueCount, rootGroupDef);
    }

    // first sub-value index and number of sub-values
    subFirstId = 0;
    selfSubCount = rootGroupDef.selfSubCount;

    if (subFirstId < 0 || selfSubCount <= 0 || subFirstId + selfSubCount > subValueCount)
        throw ModelException(
            "Invalid first sub-value index: %d or number of sub-values: %d", subFirstId, selfSubCount
            );

    // broadcast metadata tables from root to all child processes
    // broadcast basic model run options
    // broadcast model messages from root to all child processes
    broadcastMetaData();
    broadcastRunOptions();
    broadcastLanguageMessages();

    // if this is modeling task then find it in database
    // and create task run entry in database
    taskId = findTask(dbExec);
    if (taskId > 0) taskRunId = createTaskRun(taskId, dbExec);
}

// broadcast metadata tables from root to all modeling processes
void RootController::broadcastMetaData(void)
{
    // broadcast metadata tables
    broadcastMetaTable<IModelDicTable>(MsgTag::modelDic, metaStore->modelTable);
    broadcastMetaTable<ITypeDicTable>(MsgTag::typeDic, metaStore->typeDic);
    broadcastMetaTable<ITypeEnumLstTable>(MsgTag::typeEnumLst, metaStore->typeEnumLst);
    broadcastMetaTable<IParamDicTable>(MsgTag::parameterDic, metaStore->paramDic);
    broadcastMetaTable<IParamDimsTable>(MsgTag::parameterDims, metaStore->paramDims);
    broadcastMetaTable<ITableDicTable>(MsgTag::tableDic, metaStore->tableDic);
    broadcastMetaTable<ITableDimsTable>(MsgTag::tableDims, metaStore->tableDims);
    broadcastMetaTable<ITableAccTable>(MsgTag::tableAcc, metaStore->tableAcc);
    broadcastMetaTable<ITableExprTable>(MsgTag::tableExpr, metaStore->tableExpr);
}

// broadcast meta table db rows
template <typename MetaTbl>
void RootController::broadcastMetaTable(MsgTag i_msgTag, unique_ptr<MetaTbl> & io_tableUptr)
{
    unique_ptr<IPackedAdapter> packAdp(IPackedAdapter::create(i_msgTag));

    IRowBaseVec & rv = io_tableUptr->rowsRef();
    msgExec->bcastSendPacked(ProcessGroupDef::all, rv, *packAdp);
}

// broadcast basic model run options from root to group of modeling processes
void RootController::broadcastRunOptions(void)
{
    RunOptions opts = modelRunOptions();
    msgExec->bcastInt(ProcessGroupDef::all, &opts.subValueCount);
    msgExec->bcastInt(ProcessGroupDef::all, &opts.subValueId);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(bool), &opts.useSparse);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(double), &opts.nullValue);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(bool), &opts.isDbMicrodata);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(bool), &opts.isCsvMicrodata);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(int), &opts.progressPercent);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(double), &opts.progressStep);

    // broadcast number of parameters with sub-values and parameters id
    int n = (int)paramIdSubArr.size();
    msgExec->bcastInt(ProcessGroupDef::all, &n);

    if (n > 0) {
        msgExec->bcastSend(ProcessGroupDef::all, typeid(int), n, paramIdSubArr.data());
    }


    // broadcast number of microdata attributes and attribute indices in entity array
    n = (int)entityIdxArr.size();
    msgExec->bcastInt(ProcessGroupDef::all, &n);

    if (n > 0) {
        msgExec->bcastSend(ProcessGroupDef::all, typeid(int), n, entityIdxArr.data());
    }
}

/** broadcast model messages from root to all child processes. */
void RootController::broadcastLanguageMessages(void)
{
    // get language messages: merge of message.ini and database messages
    unordered_map<string, string> msgMap = theLog->getLanguageMessages();

    // broadcast from root to all child processes
    IRowBaseVec codeValueVec;
    for (unordered_map<string, string>::const_iterator it = msgMap.cbegin(); it != msgMap.cend(); it++) {
        IRowBaseUptr cvUptr(new CodeValueRow(it->first, it->second));
        codeValueVec.push_back(std::move(cvUptr));
    }
    unique_ptr<IPackedAdapter> packAdp(IPackedAdapter::create(MsgTag::codeValue));
    msgExec->bcastSendPacked(ProcessGroupDef::all, codeValueVec, *packAdp);
}

/** next run for root process: create new run and input parameters in database and support data exchange. 
*
* - (a) find id of source working set for input parameters
* - (b) create new "run" in database (if required)
* - (c) prepare model input parameters
* - (d) prepare to receive accumulators data from to child processes
*
* (a) find id of source working set for input parameters
* ------------------------------------------------------
* use following to find input parameters set id: \n
*   - if task id or task name specified then find task id in task_lst \n
*   - if set id specified as run option then use such set id \n
*   - if set name specified as run option then find set id by name \n
*   - else use min(set id) as default set of model parameters
*
* (b) create new "run" in database
* --------------------------------
* root process creates "new run" in database:
*
*   - insert new row with new run_id key into run_lst
*   - insert run options into run_option table under run_id
*   - save run options in database
*
* (c) prepare model input parameters
* ----------------------------------
* it creates a copy of input paramters from source working set under destination run_id
* 
* search for input parameter value in following order: \n
*   - use value of parameter specified as command line or ini-file argument or from profile_option table
*   - use parameter.csv file if parameters csv directory specified
*   - use value of parameter from working set of model parameters
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
int RootController::nextRun(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // exit if root process not "active": root process dedicated to data exchange
    if (!rootGroupDef.isRootActive) return 0;   // do not start any modeling at root

    // create new run and assign it to root modeling group
    return makeNextRun(rootRunGroup());
}

/** create new run and assign it to modeling group. */
int RootController::makeNextRun(RunGroup & i_runGroup)
{
    // if group inactive then or at error state then do not create new run for that group
    if (i_runGroup.state.isShutdownOrFinal()) return 0;

    // create new run:
    // find next working set of input parameters
    // if this is a modeling task then next set from that task
    // else if no run completed then get set by id or name or as default set for the model
    auto [nSetId, nRunId, mStatus] = createNewRun(taskRunId, isWaitTaskRun, dbExec);

    mStatus = theModelRunState->updateStatus(mStatus);  // update model status: progress, wait, shutdown

    i_runGroup.nextRun(nRunId, nSetId, mStatus);

    // broadcast metadata: run id and other run options from root to all other processes
    msgExec->bcastInt(i_runGroup.groupOne, (int *)&mStatus);
    msgExec->bcastInt(i_runGroup.groupOne, &nRunId);

    if (RunState::isShutdownOrFinal(mStatus) || nSetId <= 0 || nRunId <= 0) {
        return 0;   // task "wait" for next input set or all done: all sets from task or single run completed
    }

    // read new run options and broadcast from root to all other processes
    unique_ptr<IRunOptionTable> roTable(IRunOptionTable::create(dbExec, nRunId));
    unique_ptr<IPackedAdapter> packAdp(IPackedAdapter::create(MsgTag::runOption));

    IRowBaseVec & rv = roTable->rowsRef();
    msgExec->bcastSendPacked(i_runGroup.groupOne, rv, *packAdp);

    // create list of accumulators to be received from child modeling processes 
    appendAccReceiveList(nRunId, i_runGroup);

    // if this is a root modeling group then store run options in root metadata holder
    if (i_runGroup.groupOne == rootRunGroup().groupOne) metaStore->runOptionTable.swap(roTable);

    return nRunId;
}

/** exchange between root and child processes and threads.
*   start new run, send new input, receive accumulators of output tables, send and receive status update.
*   return true if any: data received, run completed, run started, status update received.
*/
bool RootController::childExchange(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // try to receive sub-values and wait for send completion, if any outstanding
    bool isReceived = receiveSubValues();
    msgExec->waitSendAll();

    // receive status update from children and save it
    bool isStatusUpdate = receiveStatusUpdate(OM_ACTIVE_SLEEP_TIME);
    updateRunState(dbExec, runStateStore().saveUpdated());

    // find modeling groups where all sub-values completed and finalize run results
    bool isCompleted = false;

    for (RunGroup & rg : runGroupLst) {

        // if root is "active" then skip root modeling group, it is done from main()
        if (rootGroupDef.isRootActive && rg.groupOne == rootRunGroup().groupOne) continue;
        if (rg.state.isFinal()) continue;    // group already shutdown

        // check if all run sub-values completed then finalize run completed in database
        if (rg.runId > 0) {
            if (rg.isSubDone.isAll()) {
                isCompleted = true;
                doShutdownRun(rg.runId, taskRunId, dbExec); // run completed
                rg.reset();                                 // group is ready for next run
            }
        }
    }

    // check if all groups already completed, all groups status one of: exit, done, error
    if (!updateModelRunStatus()) {
        return true;    // exit with error
    }

    // create new run and assign it to idle modeling group
    bool isNewRun = false;

    for (RunGroup & rg : runGroupLst) {

        // if root is "active" then skip root modeling group, it is done from main()
        if (rootGroupDef.isRootActive && rg.groupOne == rootRunGroup().groupOne) continue;

        // group already running
        if (rg.runId > 0 || rg.state.status() != ModelStatus::init) continue;

        // create new run and assign it to the group
        int nRunId = makeNextRun(rg);

        if (nRunId > 0) {
            readAllRunParameters(rg);   // broadcast input parameters to the group
            isNewRun = true;
        }

        if (nRunId <= 0 || theModelRunState->isShutdownOrFinal()) {
            break;          // task "wait" for next input set or all done: all sets from task or single run completed
        }
    }

    return isReceived || isCompleted || isNewRun || isStatusUpdate;
}

/** model process shutdown if exiting without completion (ie: exit on error). */
void RootController::shutdownOnExit(ModelStatus i_status)
{
    // finalize process: receive final status from children, update run and task with final status
    receiveStatusUpdate(OM_WAIT_SLEEP_TIME);
    doShutdownOnExit(i_status, rootRunGroup().runId, taskRunId, dbExec);
}

/** model process shutdown: wait for all child to be completed and do final cleanup. */
void RootController::shutdownWaitAll(void) 
{
    // receive outstanding run results
    long nAttempt = 1 + OM_WAIT_SLEEP_TIME / OM_ACTIVE_SLEEP_TIME;
    bool isAnyToRecv = true;
    bool isError = false;
    do {
        bool isReceived = receiveSubValues();  // receive outstanding sub-values

        isAnyToRecv = std::any_of(
            accRecvLst.cbegin(),
            accRecvLst.cend(),
            [](AccReceive i_recv) -> bool { return !i_recv.isReceived; }
        );

        // receive sub-values status update and set model process status to final: done, exit, error
        receiveStatusUpdate();
        updateRunState(dbExec, runStateStore().saveUpdated());  // update run status for all sub-values

        isError = !updateModelRunStatus();
        if (isError && --nAttempt <= 0) break;      // stop receive attempts if model status is error

        // no data received: if any accumulators outstanding then sleep before try again
        if (!isReceived && isAnyToRecv) this_thread::sleep_for(chrono::milliseconds(OM_RECV_SLEEP_TIME));
    }
    while (isAnyToRecv);

    // if model state is error then do one attempt to wait send and shutdown model with error
    if (isError) {
        msgExec->waitSendAll();
        shutdownOnExit(ModelStatus::error);                         // do shutdown on error
        throw ModelException("error in child modeiling process");   // exit from root process with error
    }

    // wait for send completion, if any outstanding
    msgExec->waitSendAll();

    // send "done" signal to all child processes
    int zeroRunId = 0;
    ModelStatus mStatus = ModelStatus::done;

    for (RunGroup & rg : runGroupLst) {

        // skip if group already shutdown
        if (rg.state.isShutdownOrFinal()) continue;

        // check if all run sub-values completed then finalize run completed in database
        if (rg.runId > 0) {
            if (rg.isSubDone.isAll()) doShutdownRun(rg.runId, taskRunId, dbExec);  // run completed
        }

        // send "done" to the group and update group final status
        msgExec->bcastInt(rg.groupOne, (int *)&mStatus);
        msgExec->bcastInt(rg.groupOne, &zeroRunId);
        rg.state.updateStatus(mStatus);
    }

    // finalize shutdown: receive final status from children, update database and status, set clean shutdown of MPI
    receiveStatusUpdate(OM_WAIT_SLEEP_TIME);
    doShutdownAll(taskRunId, dbExec);
    msgExec->setCleanExit(true);
}

/** model run shutdown: save results and update run status. */
void RootController::shutdownRun(int i_runId) 
{
    // receive outstanding sub-values for that run id
    long nAttempt = 1 + OM_WAIT_SLEEP_TIME / OM_ACTIVE_SLEEP_TIME;
    bool isAnyToRecv = true;
    bool isError = false;
    do {
        bool isReceived = receiveSubValues();  // receive outstanding sub-values

        isAnyToRecv = std::any_of(
            accRecvLst.cbegin(),
            accRecvLst.cend(),
            [i_runId](AccReceive i_recv) -> bool { return i_recv.runId == i_runId && !i_recv.isReceived; }
        );

        // receive sub-values status update and set model process status to final: done, exit, error
        receiveStatusUpdate();
        updateRunState(dbExec, runStateStore().saveUpdated());  // update run status for all sub-values

        isError = !updateModelRunStatus();
        if (isError && --nAttempt <= 0) break;      // stop receive attempts if model status is error

        // no data received: if any accumulators outstanding then sleep before try again
        if (!isReceived && isAnyToRecv) this_thread::sleep_for(chrono::milliseconds(OM_RECV_SLEEP_TIME));
    }
    while (isAnyToRecv);

    // wait for send completion, if any outstanding
    // if model state is error then do only one attempt to wait send
    msgExec->waitSendAll();

    // run completed OK
    if (!theModelRunState->isError()) {
        doShutdownRun(i_runId, taskRunId, dbExec);
        rootRunGroup().reset();
    }
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_subId     parameter sub-value index
* @param[in]     i_type      parameter value type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void RootController::readParameter(const char * i_name, int i_subId, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // read parameter from db
        unique_ptr<IParameterReader> reader(
            IParameterReader::create(rootRunGroup().runId, i_name, dbExec, meta())
            );
        reader->readParameter(dbExec, i_subId, i_type, i_size, io_valueArr);

        // broadcast parameter to root group child modeling processes
        msgExec->bcastSend(rootGroupDef.groupOne, i_type, i_size, io_valueArr);
    }
    catch (exception & ex) {
        throw ModelException("Failed to read input parameter: %s. %s", i_name, ex.what());
    }
}

/** read all input parameters by run id and broadcast to child processes. */
void RootController::readAllRunParameters(const RunGroup & i_runGroup) const
{
    for (size_t nPar = 0; nPar < PARAMETER_NAME_ARR_LEN; nPar++) {

        // create parameter reader to get from db parameter values for the group run id
        unique_ptr<IParameterReader> reader(
            IParameterReader::create(i_runGroup.runId, parameterNameSizeArr[nPar].name, dbExec, meta())
        );
        int nSubCount = parameterSubCount(reader->parameterId());

        // allocate memory to read parameter sub-value
        auto paramData = ValueArray(parameterNameSizeArr[nPar].typeOf, parameterNameSizeArr[nPar].size);

        // read each parameter sub-values from db and broadcast to all child modeling processes
        for (int nSub = 0; nSub < nSubCount; nSub++) {
            reader->readParameter(dbExec, nSub, parameterNameSizeArr[nPar].typeOf, parameterNameSizeArr[nPar].size, paramData.ptr());
            msgExec->bcastSend(i_runGroup.groupOne, parameterNameSizeArr[nPar].typeOf, parameterNameSizeArr[nPar].size, paramData.ptr());
        }
        paramData.cleanup();
    }
}

/** write output table accumulators.
*
* @param[in]     i_runOpts      model run options
* @param[in]     i_isLastTable  if true then it is last output table to write
* @param[in]     i_name         output table name
* @param[in]     i_size         number of cells for each accumulator
* @param[in,out] io_accValues   accumulator values
*/
void RootController::writeAccumulators(
    const RunOptions & i_runOpts,
    bool i_isLastTable,
    const char * i_name,
    size_t i_size,
    forward_list<unique_ptr<double[]> > & io_accValues
    )
{
    // write accumulators into database
    doWriteAccumulators(rootRunGroup().runId, dbExec, i_runOpts, i_name, i_size, io_accValues);

    // if all accumulators of sub-value completed then update restart sub-value index
    if (i_isLastTable) {
        rootRunGroup().isSubDone.setAt(i_runOpts.subValueId);       // mark that sub-value as completed
        updateRestartSubValueId(rootRunGroup().runId, dbExec, rootRunGroup().isSubDone.countFirst());
    }
}

/** append to list of accumulators to be received from child modeling processes. */
void RootController::appendAccReceiveList(int i_runId, const RunGroup & i_runGroup)
{
    const vector<TableAccRow> accVec = metaStore->tableAcc->byModelId(modelId);

    int tblId = -1;
    size_t valSize = 0;
    for (int nAcc = 0; nAcc < (int)accVec.size(); nAcc++) {

        // get accumulator data size
        if (tblId != accVec[nAcc].tableId) {
            tblId = accVec[nAcc].tableId;
            valSize = IOutputTableWriter::sizeOf(meta(), tblId);
        }

        for (int nSub = 0; nSub < subValueCount; nSub++) {

            int nRank = i_runGroup.rankBySubValueId(nSub);
            if (nRank == msgExec->rootRank) continue;

            accRecvLst.push_back(
                AccReceive(i_runId, nSub, subValueCount, nRank, tblId, accVec[nAcc].accId, nAcc, valSize)
            );
        }
    }
}

/** receive accumulators of output tables sub-values and write into database. */
bool RootController::receiveSubValues(void)
{
    // exit if nothing to receive
    if (accRecvLst.empty()) return false;

    // try to receive and save accumulators
    bool isAnyReceived = false;

    for (AccReceive & accRecv : accRecvLst) {

        if (accRecv.isReceived) continue;   // accumulator already received

        // allocate buffer to receive the data
        unique_ptr<double> valueUptr(new double[(int)accRecv.valueSize]);
        double * valueArr = valueUptr.get();

        // try to receive accumulator values
        accRecv.isReceived = msgExec->tryReceive(
            accRecv.senderRank, (MsgTag)accRecv.msgTag, typeid(double), accRecv.valueSize, valueArr
        );
        if (!accRecv.isReceived) continue;

        // accumulator received: write it into database
        const TableDicRow * tblRow = metaStore->tableDic->byKey(modelId, accRecv.tableId);
        if (tblRow == nullptr) throw DbException("output table not found in tables dictionary, id: %d", accRecv.tableId);

        unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
            accRecv.runId,
            tblRow->tableName.c_str(),
            dbExec,
            meta(),
            subValueCount,
            modelRunOptions().useSparse,
            modelRunOptions().nullValue
        ));
        writer->writeAccumulator(dbExec, accRecv.subValueId, accRecv.accId, accRecv.valueSize, valueArr);

        isAnyReceived = true;
    }

    // update restart sub-value in database and list of accumulators
    if (isAnyReceived) updateAccReceiveList();

    return isAnyReceived;
}

/** update restart sub-value in database and list of accumulators to be received. */
void RootController::updateAccReceiveList(void)
{
    // exit if nothing to receive
    if (accRecvLst.empty()) return;

    // collect accumulators received status: find if any accumulators not received
    struct RunSubDone {
        int runId;      // run id to to receive from child
        int subValueId; // sub-values number to receive
        bool isDone;    // if true then all accumulators received

        RunSubDone(int i_runId, int i_subId, bool i_isDone) : runId(i_runId), subValueId(i_subId), isDone(i_isDone) { }
    };
    vector<RunSubDone> rsdVec;

    for (AccReceive & accRecv : accRecvLst) {

        auto rsd = find_if(
            rsdVec.begin(),
            rsdVec.end(),
            [&accRecv](RunSubDone i_rsd) -> bool { return i_rsd.runId == accRecv.runId && i_rsd.subValueId == accRecv.subValueId; }
        );

        if (rsd != rsdVec.end()) {
            rsd->isDone &= accRecv.isReceived;  // update done status: it is true if all accumulators received
        }
        else {
            rsdVec.push_back(RunSubDone(accRecv.runId, accRecv.subValueId, accRecv.isReceived));
        }
    }

    // update modeling groups "sub-value done" status
    for (RunGroup & rg : runGroupLst) {

        // for each sub-value of that run check if all accumulators received
        bool isAny = false;

        for (int nSub = 0; nSub < subValueCount; nSub++) {

            auto rsd = find_if(
                rsdVec.begin(),
                rsdVec.end(),
                [&rg, nSub](RunSubDone i_rsd) -> bool { return i_rsd.runId == rg.runId && i_rsd.subValueId == nSub; }
            );

            if (rsd != rsdVec.end() && rsd->isDone) {
                rg.isSubDone.setAt(rsd->subValueId);    // "done" for that sub-value of that run id
                isAny = true;
            }
        }

        // if any changes in sub-value "done" status then update restart sub-value in database
        if (isAny) {
            updateRestartSubValueId(rg.runId, dbExec, rg.isSubDone.countFirst());
        }
    }

    // remove accumulators which received from the list
    accRecvLst.remove_if([](AccReceive i_recv) -> bool { return i_recv.isReceived; });
}

/** update process status if all run groups completed: done, exit or error.
*   return true if model process status is not error
*/
bool RootController::updateModelRunStatus(void)
{
    // check if all groups already completed, all groups status one of: exit, done, error
    bool isAllDone = true;
    ModelStatus maxStatus = ModelStatus::undefined;

    for (RunGroup & rg : runGroupLst) {
        ModelStatus gStatus = rg.state.status();
        isAllDone &= RunState::isFinal(gStatus);
        if (gStatus > maxStatus) maxStatus = gStatus;
    }

    // all group are at final status (exit, done or error)
    if (isAllDone) theModelRunState->updateStatus(maxStatus);   // set model process status to final
    
    return !theModelRunState->isError();
}

/** receive status update from all child processes. */
bool RootController::receiveStatusUpdate(long i_waitTime)
{
    // try to receive and save status update from child precesses
    unique_ptr<IPackedAdapter> packAdp(IPackedAdapter::create(MsgTag::statusUpdate));
    IRowBaseVec rsVec;
    bool isAnyReceived = false;

    long nAttempt = 1 + i_waitTime / OM_ACTIVE_SLEEP_TIME;

    bool isAnyActive = false;   // is any group still active: state is not done, exit or error
    do {
        isAnyActive = false;

        for (RunGroup & rg : runGroupLst) {

            if (rg.state.isFinal()) continue;    // group completed, for all processes in group status: done, exit or error

            for (int n = 0; n < rg.childCount; n++) {

                // try to receive child status update
                if (!msgExec->tryReceive(n + rg.firstChildRank, rsVec, *packAdp)) {
                    continue;     // no status update from that child
                }
                isAnyReceived = true;

                if (rsVec.size() <= 0) continue;    // no update for any of sub-values run state or child process run state

                // update sub-values run state
                RunState childState = runStateStore().fromRowVector(rsVec);
                rg.childState[n].updateStatus(childState.theStatus);
                rsVec.clear();

                // if status of any child process is error then mark entire group as error
                if (RunState::isError(childState.theStatus)) rg.state.updateStatus(childState.theStatus);
            }

            // if status of all child processes is exit or done then set entire group as exit or done
            if (!rootGroupDef.isRootActive || rg.groupOne != rootRunGroup().groupOne) {     // it not an active root group
                
                bool isAllExit = true;
                bool isAllDone = true;

                for (int n = 0; n < rg.childCount; n++) {
                    isAllExit &= rg.childState[n].status() == ModelStatus::exit;
                    isAllDone &= rg.childState[n].status() == ModelStatus::done;
                }
                if (isAllExit) rg.state.updateStatus(ModelStatus::exit);    // set group status as exit: status of all child processes is exit
                if (isAllDone) rg.state.updateStatus(ModelStatus::done);    // set group status as done: status of all child processes is done
            }

            isAnyActive |= !rg.state.isFinal();  // check if any group still "active", group status not: done, exit, error
        }

        if (isAnyActive && nAttempt > 0) {
            this_thread::sleep_for(chrono::milliseconds(OM_ACTIVE_SLEEP_TIME));
        }
    } while (isAnyActive && --nAttempt > 0);

    return isAnyReceived;
}
