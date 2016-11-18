/**
* @file
* OpenM++ modeling library: 
* root process run controller class to create new model run(s), send input parameters to children and receieve output tables.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
#include "modelHelper.h"
#include "runControllerImpl.h"

using namespace std;
using namespace openm;

/** initialize root modeling process.
*
* - (a) get number of subsamples, processes, threads, groups
* - (b) merge command line and ini-file options with db profile table
* - (c) initialize task run if required
* - (d) load metadata tables from database and broadcast it to child processes
*
* (a) get number of subsamples, processes, threads, groups
* --------------------------------------------------------
*
* Number of subsamples by default = 1 and it can be specified
* using command-line argument, ini-file or profile table in database
*
* Number of modeling threads by default = 1 and subsamples run sequentially in single thread.
* If more threads specified (i.e. by command-line argument) then subsamples run in parallel.
*
* For example: \n
*   model.exe -General.Subsamples 8 \n
*   model.exe -General.Subsamples 8 -General.Threads 4 \n
*   mpiexec -n 2 model.exe -General.Subsamples 31 -General.Threads 7
*
* Number of modeling processes = MPI world size
*
* Processes may be combined into groups for parallel run of input data sets
* and size of the group is such to calculate all subsamples by the group.
*
* Size of modeling group = number of subsamples / number of threads \n
*   group size always >= 1 therefore group always include at least one process \n
* Number of groups = number of processes / group size \n
*   by default is only one group, which include all modeling processes \n
*
* Root process may or may not be be used for modeling \n
*   if (group size * group count) == process count then "root is active" \n
*   else root process dedicated for data exchange and modeling done only by child processes
*
* Number of subsamples per process = total number of subsamples / group size \n
*   if total number of subsamples % number of processes != 0 then remainder calculated at group last process \n
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
*   - as rows of profile_option table, default profile_name='model name'
*     profile name also can be command line argument or ini-file entry, i.e.: \n
*       model.exe -OpenM.OptionsProfile TestProfile
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
*   model.exe -General.Subsamples 16 -OpenM.TaskName someTask \n
*   model.exe -General.Subsamples 16 -OpenM.Taskid 1 \n
*   model.exe -General.Subsamples 16 -OpenM.TaskName someTask -OpenM.TaskWait true
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
    metaStore.reset(new MetaHolder);
    modelId = readMetaTables(dbExec, metaStore.get());

    // merge command line and ini-file arguments with profile_option table values
    mergeProfile(dbExec);

    // get main run control values: number of subsamples, threads
    subSampleCount = argOpts().intOption(RunOptionsKey::subSampleCount, 1); // number of subsamples from command line or ini-file
    threadCount = argOpts().intOption(RunOptionsKey::threadCount, 1);       // max number of modeling threads
    isWaitTaskRun = argOpts().boolOption(RunOptionsKey::taskWait);          // if true then task run under external supervision

    // broadcast basic run options from root to all other processes
    broadcastInt(ProcessGroupDef::all, msgExec, &subSampleCount);
    broadcastInt(ProcessGroupDef::all, msgExec, &threadCount);
    broadcastInt(ProcessGroupDef::all, msgExec, &modelId);

    // basic validation: number of processes expected to be > 1
    if (subSampleCount <= 0) throw ModelException("Invalid number of subsamples: %d", subSampleCount);
    if (threadCount <= 0) throw ModelException("Invalid number of modeling threads: %d", threadCount);
    if (processCount <= 1) throw ModelException("Invalid number of modeling processes: %d", processCount);

    // create groups for parallel run of modeling task
    // "is root active": root may be used for modeling in last group
    // if (group size * group count) == process count then "root is active"
    // else root process dedicated for data exchange and modeling done only by child processes
    rootGroupDef = ProcessGroupDef(subSampleCount, threadCount, msgExec->worldSize(), msgExec->rank());

    msgExec->createGroups(rootGroupDef.groupSize, rootGroupDef.groupCount);

    for (int nGroup = 0; nGroup < rootGroupDef.groupCount; nGroup++) {
        runGroupLst.emplace_back(1 + nGroup, subSampleCount, rootGroupDef);
    }

    // first subsample number and number of subsamples
    subFirstNumber = 0;
    selfSubCount = rootGroupDef.selfSubCount;

    if (subFirstNumber < 0 || selfSubCount <= 0 || subFirstNumber + selfSubCount > subSampleCount)
        throw ModelException(
            "Invalid first subsample number: %d or number of subsamples: %d", subFirstNumber, selfSubCount
            );

    // broadcast metadata tables from root to all other processes
    broadcastMetaData(ProcessGroupDef::all, msgExec, metaStore.get());

    // broadcast basic model run options
    broadcastRunOptions(ProcessGroupDef::all, msgExec);

    // if this is modeling task then find it in database
    // and create task run entry in database
    taskId = findTask(dbExec);
    if (taskId > 0) taskRunId = createTaskRun(taskId, dbExec);
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
    // create new run:
    // find next working set of input parameters
    // if this is a modeling task then next set from that task
    // else if no run completed then get set by id or name or as default set for the model
    SetRunItem nowSetRun = createNewRun(taskRunId, isWaitTaskRun, dbExec);

    ModelStatus mStatus = theModelRunState.updateStatus(nowSetRun.state.status());  // update model status: progress, wait, shutdown

    i_runGroup.nextRun(nowSetRun.runId, nowSetRun.setId, mStatus);

    // broadcast metadata: run id and other run options from root to all other processes
    broadcastInt(i_runGroup.groupOne, msgExec, (int *)&mStatus);
    broadcastInt(i_runGroup.groupOne, msgExec, &nowSetRun.runId);

    if (ModelRunState::isShutdownOrExit(mStatus) || nowSetRun.isEmpty()) {
        return 0;   // task "wait" for next input set or all done: all sets from task or single run completed
    }

    // create list of accumulators to be received from child modeling processes 
    appendAccReceiveList(nowSetRun.runId, i_runGroup);

    return nowSetRun.runId;
}

/** communicate with child processes to send new input and receive accumulators of output tables. */
bool RootController::childExchange(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // try to receive subsamples and wait for send completion, if any outstanding
    bool isReceived = receiveSubSamples();
    msgExec->waitSendAll();

    // find modeling groups where all subsamples completed and finalize run results
    bool isCompleted = false;

    for (RunGroup & rg : runGroupLst) {

        // if root is "active" then skip root modeling group, it is done from main()
        if (rootGroupDef.isRootActive && rg.groupOne == rootRunGroup().groupOne) continue;
        if (rg.state.isExit()) continue;    // group already shutdown

        // check if all run subsamples completed then finalize run completed in database
        if (rg.runId > 0) {
            if (rg.isSubDone.isAll()) {
                isCompleted = true;
                doShutdownRun(rg.runId, taskRunId, dbExec); // run completed
                rg.reset();                                 // group is ready for next run
            }
        }
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

        if (nRunId <= 0 || theModelRunState.isShutdownOrExit()) {
            return isReceived || isCompleted || isNewRun;   // task "wait" for next input set or all done: all sets from task or single run completed
        }
    }

    return isReceived || isCompleted || isNewRun;
}

/** model process shutdown: wait for all child to be completed and do final cleanup. */
void RootController::shutdownWaitAll(void) 
{
    // receive outstanding run results
    bool isAnyToRecv = true;
    do {
        bool isReceived = receiveSubSamples();  // receive outstanding subsamples

        isAnyToRecv = std::any_of(
            accRecvLst.cbegin(),
            accRecvLst.cend(),
            [](AccReceive i_recv) -> bool { return !i_recv.isReceived; }
        );

        // no data received: if any accumulators outstanding then sleep before try again
        if (!isReceived && isAnyToRecv) this_thread::sleep_for(chrono::milliseconds(OM_RECV_SLEEP_TIME));
    }
    while (isAnyToRecv);

    // wait for send completion, if any outstanding
    msgExec->waitSendAll();
   
    // send "done" signal to all child processes
    int zeroRunId = 0;
    ModelStatus mStatus = ModelStatus::done;

    for (RunGroup & rg : runGroupLst) {

        // skip if group already shutdown
        if (rg.state.isShutdownOrExit()) continue;

        // check if all run subsamples completed then finalize run completed in database
        if (rg.runId > 0) {
            if (rg.isSubDone.isAll()) doShutdownRun(rg.runId, taskRunId, dbExec);  // run completed
        }

        // send "done" to the group and update group final status
        broadcastInt(rg.groupOne, msgExec, (int *)&mStatus);
        broadcastInt(rg.groupOne, msgExec, &zeroRunId);
        rg.state.updateStatus(mStatus);
    }

    // finalize shutdown: update database and status
    doShutdownAll(taskRunId, dbExec); 
}

/** model run shutdown: save results and update run status. */
void RootController::shutdownRun(int i_runId) 
{
    // receive outstanding subsamples for that run id
    bool isAnyToRecv = true;
    do {
        bool isReceived = receiveSubSamples();  // receive outstanding subsamples

        isAnyToRecv = std::any_of(
            accRecvLst.cbegin(),
            accRecvLst.cend(),
            [i_runId](AccReceive i_recv) -> bool { return i_recv.runId == i_runId && !i_recv.isReceived; }
        );

        // no data received: if any accumulators outstanding then sleep before try again
        if (!isReceived && isAnyToRecv) this_thread::sleep_for(chrono::milliseconds(OM_RECV_SLEEP_TIME));
    }
    while (isAnyToRecv);

    // wait for send completion, if any outstanding
    msgExec->waitSendAll();

    // run completed
    doShutdownRun(i_runId, taskRunId, dbExec);
    rootRunGroup().reset();
}

// append to list of accumulators to be received from child modeling processes
void RootController::appendAccReceiveList(int i_runId, const RunGroup & i_runGroup)
{
    const vector<TableAccRow> accVec = metaStore->tableAcc->byModelId(modelId);

    int tblId = -1;
    size_t valCount = 0;
    for (int nAcc = 0; nAcc < (int)accVec.size(); nAcc++) {

        // get accumulator data size
        if (tblId != accVec[nAcc].tableId) {
            tblId = accVec[nAcc].tableId;
            valCount = IOutputTableWriter::sizeOf(metaStore.get(), tblId);
        }

        for (int nSub = 0; nSub < subSampleCount; nSub++) {

            int nRank = i_runGroup.rankBySubsampleNumber(nSub);
            if (nRank == msgExec->rootRank) continue;

            accRecvLst.push_back(
                AccReceive(i_runId, nSub, subSampleCount, nRank, tblId, accVec[nAcc].accId, nAcc, valCount)
                );
        }
    }
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_type      parameter value type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void RootController::readParameter(const char * i_name, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // read parameter from db
        unique_ptr<IParameterReader> reader(
            IParameterReader::create(rootRunGroup().runId, i_name, dbExec, metaStore.get())
            );
        reader->readParameter(dbExec, i_type, i_size, io_valueArr);

        // broadcast parameter to root group child modeling processes
        msgExec->bcast(rootGroupDef.groupOne, i_type, i_size, io_valueArr);
    }
    catch (exception & ex) {
        throw ModelException("Failed to read input parameter: %s. %s", i_name, ex.what());
    }
}

/** read all input parameters by run id and broadcast to child processes. */
void RootController::readAllRunParameters(const RunGroup & i_runGroup) const
{
    unique_ptr<char> byteArr;
    unique_ptr<string[]> strArr;
    void * paramData = nullptr;

    for (size_t k = 0; k < PARAMETER_NAME_ARR_LEN; k++) {

        // allocate memory to read parameter
        if (parameterNameSizeArr[k].typeOf == typeid(string)) {
            strArr.reset(new string[parameterNameSizeArr[k].size]);
            paramData = strArr.get();
        }
        else {
            size_t packSize = IPackedAdapter::packedSize(parameterNameSizeArr[k].typeOf, parameterNameSizeArr[k].size);
            byteArr.reset(new char[packSize]);
            paramData = byteArr.get();
        }

        // read parameter from db
        unique_ptr<IParameterReader> reader(
            IParameterReader::create(i_runGroup.runId, parameterNameSizeArr[k].name, dbExec, metaStore.get())
            );
        reader->readParameter(dbExec, parameterNameSizeArr[k].typeOf, parameterNameSizeArr[k].size, paramData);

        // broadcast parameter to all child modeling processes
        msgExec->bcast(i_runGroup.groupOne, parameterNameSizeArr[k].typeOf, parameterNameSizeArr[k].size, paramData);
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
    forward_list<unique_ptr<double> > & io_accValues
    )
{
    // write accumulators into database
    doWriteAccumulators(rootRunGroup().runId, dbExec, i_runOpts, i_name, i_size, io_accValues);

    // if all accumulators of subsample completed then update restart subsample number
    if (i_isLastTable) {
        rootRunGroup().isSubDone.setAt(i_runOpts.subSampleNumber);      // mark that subsample as completed
        updateRestartSubsample(rootRunGroup().runId, dbExec, rootRunGroup().isSubDone.countFirst());
    }
}

/** receive accumulators of output tables subsamples and write into database. */
bool RootController::receiveSubSamples(void)
{
    // exit if nothing to receive
    if (accRecvLst.empty()) return false;

    // try to receive and save accumulators
    bool isAnyReceived = false;

    for (AccReceive & accRecv : accRecvLst) {

        if (!accRecv.isReceived) {      // if accumulator not yet received

            // allocate buffer to receive the data
            unique_ptr<double> valueUptr(new double[(int)accRecv.valueCount]);
            double * valueArr = valueUptr.get();

            // try to received
            accRecv.isReceived = msgExec->tryReceive(
                accRecv.senderRank, (MsgTag)accRecv.msgTag, typeid(double), accRecv.valueCount, valueArr
            );
            if (!accRecv.isReceived) continue;

            // accumulator received: write it into database
            const TableDicRow * tblRow = metaStore->tableDic->byKey(modelId, accRecv.tableId);
            if (tblRow == nullptr) throw new DbException("output table not found in table dictionary, id: %d", accRecv.tableId);

            unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
                accRecv.runId,
                tblRow->tableName.c_str(),
                dbExec,
                metaStore.get(),
                subSampleCount,
                modelRunOptions().useSparse,
                modelRunOptions().nullValue
            ));
            writer->writeAccumulator(dbExec, accRecv.subNumber, accRecv.accId, accRecv.valueCount, valueArr);

            isAnyReceived = true;
        }
    }

    // update restart subsample in database and list of accumulators
    if (isAnyReceived) updateAccReceiveList();

    return isAnyReceived;
}

/** update restart subsample in database and list of accumulators to be received. */
void RootController::updateAccReceiveList(void)
{
    // exit if nothing to receive
    if (accRecvLst.empty()) return;

    // collect accumulators received ststus: find if any accumulators not received
    struct RunSubDone {
        int runId;      // run id to to receive from child
        int subNumber;  // subsamples number to receive
        bool isDone;    // if true then all accumulators received

        RunSubDone(int i_runId, int i_subNumber, bool i_isDone) : runId(i_runId), subNumber(i_subNumber), isDone(i_isDone) { }
    };
    vector<RunSubDone> rsdVec;

    for (AccReceive & accRecv : accRecvLst) {

        auto rsd = find_if(
            rsdVec.begin(),
            rsdVec.end(),
            [&accRecv](RunSubDone i_rsd) -> bool { return i_rsd.runId == accRecv.runId && i_rsd.subNumber == accRecv.subNumber; }
        );

        if (rsd != rsdVec.end()) {
            rsd->isDone &= accRecv.isReceived;  // update done status: it is true if all accumulators received
        }
        else {
            rsdVec.push_back(RunSubDone(accRecv.runId, accRecv.subNumber, accRecv.isReceived));
        }
    }

    // update modeling groups "subsample done" status
    for (RunGroup & rg : runGroupLst) {

        // for each subsample of that run check if all accumulators received
        bool isAny = false;

        for (int nSub = 0; nSub < subSampleCount; nSub++) {

            auto rsd = find_if(
                rsdVec.begin(),
                rsdVec.end(),
                [&rg, nSub](RunSubDone i_rsd) -> bool { return i_rsd.runId == rg.runId && i_rsd.subNumber == nSub; }
            );

            if (rsd != rsdVec.end() && rsd->isDone) {
                rg.isSubDone.setAt(rsd->subNumber);     // "done" for that subsample of that run id
                isAny = true;
            }
        }

        // if any changes in subsample "done" status then update restart subsample in database
        if (isAny) {
            updateRestartSubsample(rg.runId, dbExec, rg.isSubDone.countFirst());
        }
    }

    // remove accumulators which received from the list
    accRecvLst.remove_if([](AccReceive i_recv) -> bool { return i_recv.isReceived; });
}
