/**
* @file
* OpenM++ modeling library: 
* child process run controller class to receive input parameters from root and send output tables.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"

using namespace std;
using namespace openm;

/** initialize child modeling process.
*
* - (a) get number of subsamples, processes, threads
* - (d) receive metadata tables from root processes
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
*/
void ChildController::init(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");

    // receive initial state and main control values from root process: subsample count, thread count
    ModelStatus startStatus = receiveInitialState();
    theModelRunState.updateStatus(startStatus);

    if (ModelRunState::isExit(startStatus)) return;     // exit if received status not a continue

    // basic validation: number of processes expected to be > 1
    if (subSampleCount <= 0) throw ModelException("Invalid number of subsamples: %d", subSampleCount);
    if (processCount <= 1) throw ModelException("Invalid number of modeling processes: %d", processCount);
    if (threadCount <= 0) throw ModelException("Invalid number of modeling threads: %d", threadCount);
    if (subSampleCount < processCount)
        throw ModelException("Error: number of subsamples: %d less than number of processes: %d", subSampleCount, processCount);

    // broadcast metadata: subsample count and meta tables from root to all other processes
    metaStore.reset(new MetaRunHolder);
    broadcastMetaData(msgExec, metaStore.get());
}

// receive initial state and main control values from root process: subsample count, thread count
ModelStatus ChildController::receiveInitialState(void)
{
    int initMsg[processInitMsgSize];
    msgExec->startRecv(IMsgExec::rootRank, MsgTag::initial, typeid(int), processInitMsgSize, initMsg);
    msgExec->waitRecvAll();

    modelId = initMsg[1];
    subSampleCount = initMsg[2];
    threadCount = initMsg[3];
    return 
        static_cast<ModelStatus>(initMsg[0]);
}

/** next run for child process: receive run id, run options and input parameters from root process.
*
* Number of modeling processes = MPI world size \n
* Number of subsamples per process = total number of subsamples / number of processes \n
*   if total number of subsamples % number of processes != 0 then remainder calculated at root process \n
*/
int ChildController::nextRun(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");
    
    // get number of subsamples and first subsample number
    selfSubCount = subPerProcess = subSampleCount / processCount;
    subFirstNumber = (msgExec->rank() - 1) * subPerProcess;

    if (subFirstNumber < 0 || subPerProcess <= 0 || selfSubCount <= 0 || subFirstNumber + selfSubCount > subSampleCount)
        throw ModelException(
            "Invalid first subsample number: %d or number of subsamples: %d or subsamples per process: %d", subFirstNumber, selfSubCount, subPerProcess
            );

    // broadcast metadata: run id and other run options from root to all other processes
    broadcastRunId(msgExec, &runId);    // temporary: work-in-porogress
    if (runId <= 0) return runId;       // temporary: work-in-porogress

    broadcastRunOptions(msgExec, metaStore->runOption);

    return runId;
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_type      parameter type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void ChildController::readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // receive broadcasted parameter from root process
        msgExec->bcast(msgExec->rootRank, i_type, i_size, io_valueArr);
    }
    catch (exception & ex) {
        throw ModelException("Failed to read input parameter: %s. %s", i_name, ex.what());
    }
}

/** send output table accumulators to root process.
*
* @param[in]     i_runOpts      model run options
* @param[in]     i_isLast       if true then it is last output table to send
* @param[in]     i_name         output table name
* @param[in]     i_size         number of cells for each accumulator
* @param[in,out] io_accValues   accumulator values
*/
void ChildController::writeAccumulators(
    const RunOptions & i_runOpts, 
    bool /*i_isLast*/,
    const char * i_name, 
    long long i_size, 
    forward_list<unique_ptr<double> > & io_accValues
    )
{
    // find output table db row and accumulators
    const TableDicRow * tblRow = metaStore->tableDic->byModelIdName(modelId, i_name);
    if (tblRow == nullptr) throw new DbException("output table not found in table dictionary: %s", i_name);

    int accIndex = (int)metaStore->tableAcc->indexOf(
        [&](const TableAccRow & i_row) -> bool { return i_row.modelId == modelId && i_row.tableId == tblRow->tableId; }
    );
    if (accIndex < 0) throw new DbException("output table accumulators not found: %s", i_name);

    // send accumulators to root process
    for (auto & ap : io_accValues) {
        msgExec->startSend(
            IMsgExec::rootRank,
            (MsgTag)(RunController::accMsgTag(i_runOpts.subSampleNumber, subSampleCount, accIndex)),
            typeid(double),
            i_size,
            ap.release()
            );
        accIndex++;
    }
}

/** receive outstanding accumulators and wait until outstanding send completed. */
void ChildController::receiveSendLast(void)
{
    // receive outstanding subsamples: only at MPI root process
    // wait for send completion, if any outstanding
    msgExec->waitSendAll();
}
