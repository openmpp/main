/**
* @file
* OpenM++ modeling library: 
* root process run controller class to create new model run(s), send input parameters to children and receieve output tables.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"

using namespace std;
using namespace openm;

/** initialize root modeling process.
*
* - (a) get number of subsamples, processes, threads
* - (b) merge command line and ini-file options with db profile table
* - (c) initialize task run if required
* - (d) load metadata tables from database and broadcast it to child processes
*
* (a) get number of subsamples, processes, threads
* ------------------------------------------------
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
* Number of modeling processes = MPI world size \n
* Number of subsamples per process = total number of subsamples / number of processes \n
*   if total number of subsamples % number of processes != 0 then remainder calculated at root process \n
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
*
* if task specified then model would run for each input working set of parameters
*/
void RootController::init(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // load metadata table rows, except of run_option, which is may not created yet
    metaStore.reset(new MetaRunHolder);
    const ModelDicRow * mdRow = readMetaTables(dbExec, metaStore.get());
    modelId = mdRow->modelId;

    // merge command line and ini-file arguments with profile_option table values
    mergeProfile(dbExec, mdRow);

    // get main run control values: number of subsamples, threads
    subSampleCount = argOpts().intOption(RunOptionsKey::subSampleCount, 1); // number of subsamples from command line or ini-file
    threadCount = argOpts().intOption(RunOptionsKey::threadCount, 1);       // max number of modeling threads

    // send initial state and main control values to all child processes
    sendInitialState();

    // basic validation: number of processes expected to be > 1
    if (subSampleCount <= 0) throw ModelException("Invalid number of subsamples: %d", subSampleCount);
    if (processCount <= 1) throw ModelException("Invalid number of modeling processes: %d", processCount);
    if (threadCount <= 0) throw ModelException("Invalid number of modeling threads: %d", threadCount);
    if (subSampleCount < processCount)
        throw ModelException("Error: number of subsamples: %d less than number of processes: %d", subSampleCount, processCount);

    // if this is modeling task then find it in database
    taskId = findTask(dbExec, mdRow);

    if (taskId > 0) {
        taskRunLst = readTask(taskId, mdRow, dbExec);

        // create task run entry in database
        taskRunId = createTaskRun(taskId, dbExec);
    }

    // broadcast metadata: subsample count and meta tables from root to all other processes
    broadcastMetaData(msgExec, metaStore.get());
}

// send initial state and main control values to all child processes: subsample count, thread count
void RootController::sendInitialState(void)
{
    int initMsg[processInitMsgSize];
    initMsg[0] = static_cast<int>(ModelStatus::init);
    initMsg[1] = modelId;
    initMsg[2] = subSampleCount;
    initMsg[3] = threadCount;

    for (int nChild = IMsgExec::rootRank + 1; nChild < processCount; nChild++) {

        if (nChild >= subSampleCount) initMsg[0] = static_cast<int>(ModelStatus::exit);     // send exit to unused child

        unique_ptr<unsigned char> msgData(IPackedAdapter::packArray(typeid(int), processInitMsgSize, &initMsg));
        msgExec->startSend(
            nChild, MsgTag::initial, typeid(int), processInitMsgSize, msgData.release()
            );
    }
    msgExec->waitSendAll(); // wait until send completed
}

/** next run for root process: create new run and input parameters in database and support data exchange. 
*
* - (a) get number of subsamples calculated by current process
* - (b) find id of source working set for input parameters
* - (c) create new "run" in database (if required)
* - (d) prepare model input parameters
* - (e) prepare to receive accumulators data from to child processes
*
* (a) get number of subsamples calculated by current process
* ----------------------------------------------------------
* number of modeling processes = MPI world size \n
* number of subsamples per process = total number of subsamples / number of processes \n
* if total number of subsamples % number of processes != 0 then remainder calculated at root process \n
*
* (b) find id of source working set for input parameters
* ------------------------------------------------------
* use following to find input parameters set id: \n
*   - if task id or task name specified then find task id in task_lst \n
*   - if set id specified as run option then use such set id \n
*   - if set name specified as run option then find set id by name \n
*   - else use min(set id) as default set of model parameters
*
* (c) create new "run" in database
* --------------------------------
* root process creates "new run" in database:
*
*   - insert new row with new run_id key into run_lst
*   - insert run options into run_option table under run_id
*   - save run options in database
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
int RootController::nextRun(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");
    if (dbExec == nullptr) throw ModelException("invalid (NULL) database connection");

    // find next working set of input parameters
    // if this is a modeling task then next set from that task
    // else if no run completed then get set by id or name or as default set for the model
    setId = 0;
    if (taskId > 0 || runId <= 0)  setId = nextSetId(taskId, dbExec, taskRunLst);

    if (setId <= 0) {   // all done: all sets from task or single run
        runId = 0;
        broadcastRunId(msgExec, &runId);
        return runId;
    }

    // get number of subsamples and first subsample number
    subPerProcess = subSampleCount / processCount;
    subFirstNumber = (processCount - 1) * subPerProcess;
    selfSubCount = subSampleCount - subFirstNumber;

    if (subFirstNumber < 0 || subPerProcess <= 0 || selfSubCount <= 0 || subFirstNumber + selfSubCount > subSampleCount)
        throw ModelException(
            "Invalid first subsample number: %d or number of subsamples: %d or subsamples per process: %d", subFirstNumber, selfSubCount, subPerProcess
            );

    // create new run and  load run_option table rows
    runId = createNewRun(setId, taskId, taskRunId, dbExec, taskRunLst);
    metaStore->runOption.reset(IRunOptionTable::create(dbExec, runId));

    // broadcast metadata: run id and other run options from root to all other processes
    broadcastRunId(msgExec, &runId);                    // temporary: work-in-porogress
    broadcastRunOptions(msgExec, metaStore->runOption);

    // create list of accumulators to be received from child modeling processes 
    initAccReceiveList();

    // temporary: work-in-porogress
    subDoneVec.assign(subSampleCount, false);

    return runId;
}

// create list of accumulators to be received from child modeling processes
// root process calculate last subsamples
void RootController::initAccReceiveList(void)
{
    if (subSampleCount <= subPerProcess) return;    // exit if all subsamples was produced at root model process

    const vector<TableAccRow> accVec = metaStore->tableAcc->byModelId(modelId);

    int tblId = -1;
    long long valSize = 0;
    for (int nAcc = 0; nAcc < (int)accVec.size(); nAcc++) {

        // get accumulator data size
        if (tblId != accVec[nAcc].tableId) {
            tblId = accVec[nAcc].tableId;
            valSize = IOutputTableWriter::sizeOf(modelId, metaStore.get(), tblId);
        }

        for (int nSub = 0; nSub < subFirstNumber; nSub++) {
            accRecvVec.push_back(
                AccReceiveItem(nSub, subSampleCount, subPerProcess, tblId, accVec[nAcc].accId, nAcc, valSize)
                );
        }
    }
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_type      parameter type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void RootController::readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // read parameter from db
        unique_ptr<IParameterReader> reader(
            IParameterReader::create(modelId, runId, i_name, dbExec, metaStore.get())
            );
        reader->readParameter(dbExec, i_type, i_size, io_valueArr);

        // broadcast parameter to all child modeling processes
        msgExec->bcast(msgExec->rootRank, i_type, i_size, io_valueArr);
    }
    catch (exception & ex) {
        throw ModelException("Failed to read input parameter: %s. %s", i_name, ex.what());
    }
}

/** write output table accumulators.
*
* @param[in]     i_runOpts      model run options
* @param[in]     i_isLast       if true then it is last output table to write
* @param[in]     i_name         output table name
* @param[in]     i_size         number of cells for each accumulator
* @param[in,out] io_accValues   accumulator values
*/
void RootController::writeAccumulators(
    const RunOptions & i_runOpts,
    bool i_isLast,
    const char * i_name,
    long long i_size,
    forward_list<unique_ptr<double> > & io_accValues
    )
{
    // write accumulators into database
    doWriteAccumulators(runId, dbExec, i_runOpts, i_name, i_size, io_accValues);

    // if all accumulators of subsample completed then update restart subsample number
    if (i_isLast) {
        subDoneVec[i_runOpts.subSampleNumber] = true;    // mark that subsample as completed
        updateRestartSubsample(runId, dbExec, subDoneVec);
    }
}

/** receive outstanding accumulators and wait until outstanding send completed. */
void RootController::receiveSendLast(void)
{
    // receive outstanding subsamples
    receiveLast();

    // wait for send completion, if any outstanding
    msgExec->waitSendAll();
}

/** receive outstanding accumulators. */
void RootController::receiveLast(void)
{
    // receive outstanding subsamples
    bool isAnyToRecv = true;
    do {
        isAnyToRecv = receiveSubSamples();

        if (isAnyToRecv) this_thread::sleep_for(chrono::milliseconds(OM_RECV_SLEEP_TIME));
    }
    while (isAnyToRecv);

    // temporary: work-in-progress
    accRecvVec.clear();
}

/** receive accumulators of output tables subsamples and write into database.
*
* @return  true if not all accumulators for all subsamples received yet (if data not ready)
*/
bool RootController::receiveSubSamples(void)
{
    // exit if nothing to receive
    if (accRecvVec.empty()) return false;

    // get sparse settings
    bool isSparse = metaStore->runOption->boolValue(RunOptionsKey::useSparse);
    double nullValue = metaStore->runOption->doubleValue(RunOptionsKey::sparseNull, DBL_EPSILON);

    // try to receive and save accumulators
    bool isReceived = false;
    for (AccReceiveItem & accRecv : accRecvVec) {

        if (!accRecv.isReceived) {      // if accumulator not yet received

            // allocate buffer to receive the data
            unique_ptr<double> valueUptr(new double[(int)accRecv.valueSize]);
            double * valueArr = valueUptr.get();

            // try to received
            accRecv.isReceived = msgExec->tryReceive(
                accRecv.senderRank, (MsgTag)accRecv.msgTag, typeid(double), accRecv.valueSize, valueArr
                );
            if (!accRecv.isReceived) continue;

            // accumulator received: write it into database
            const TableDicRow * tblRow = metaStore->tableDic->byKey(modelId, accRecv.tableId);
            if (tblRow == nullptr) throw new DbException("output table not found in table dictionary, id: %d", accRecv.tableId);

            unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
                modelId,
                runId,
                tblRow->tableName.c_str(),
                dbExec,
                metaStore.get(),
                subSampleCount,
                isSparse,
                nullValue
                ));
            writer->writeAccumulator(dbExec, accRecv.subNumber, accRecv.accId, accRecv.valueSize, valueArr);

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
