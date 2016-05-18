/**
* @file
* OpenM++ modeling library: 
* child process run controller class to receive input parameters from root and send output tables.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
#include "modelHelper.h"
#include "runControllerImpl.h"

using namespace std;
using namespace openm;

/** initialize child modeling process.
*
* - (a) get number of subsamples, processes, threads, groups
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
*
* Size of modeling group = number of subsamples / number of threads \n
* Number of groups = number of processes / group size \n
*   by default only one group which include all modeling processes \n
*   group size always >= 1 therefore group always include at least one process \n
*
* Number of subsamples per child process = total number of subsamples / group size \n
*   if total number of subsamples % group size != 0 then remainder calculated at last group process \n
*/
void ChildController::init(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");

    // broadcast basic run options from root to all other processes
    broadcastInt(ProcessGroupDef::all, msgExec, &subSampleCount);
    broadcastInt(ProcessGroupDef::all, msgExec, &threadCount);
    broadcastInt(ProcessGroupDef::all, msgExec, &modelId);

    // basic validation: number of processes expected to be > 1
    if (subSampleCount <= 0) throw ModelException("Invalid number of subsamples: %d", subSampleCount);
    if (threadCount <= 0) throw ModelException("Invalid number of modeling threads: %d", threadCount);
    // if (processCount <= 1) throw ModelException("Invalid number of modeling processes: %d", processCount);

    // create groups for parallel run of modeling task
    groupDef = ProcessGroupDef(subSampleCount, threadCount, msgExec->worldSize(), msgExec->rank());

    msgExec->createGroups(groupDef.groupSize, groupDef.groupCount);

    // first subsample number and number of subsamples
    subFirstNumber = groupDef.activeRank * groupDef.subPerProcess;
    selfSubCount = groupDef.selfSubCount;

    if (subFirstNumber < 0 || selfSubCount <= 0 || subFirstNumber + selfSubCount > subSampleCount)
        throw ModelException(
            "Invalid first subsample number: %d or number of subsamples: %d", subFirstNumber, selfSubCount
            );

    // adjust number of active processes: exit from unused child processes
    if (msgExec->rank() > groupDef.groupSize * groupDef.groupCount) {
        theModelRunState.updateStatus(ModelStatus::exit);
    }

    // broadcast metadata tables from root to all other processes
    metaStore.reset(new MetaRunHolder);
    modelId = broadcastMetaData(ProcessGroupDef::all, msgExec, metaStore.get());

    // receive model run options
    broadcastRunOptions(ProcessGroupDef::all, msgExec);
}

/** next run for child process: receive run id, run options and input parameters from root process. */
int ChildController::nextRun(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");

    if (theModelRunState.isShutdownOrExit()) return 0;      // exit if status not a continue status

    // broadcast metadata: model status, run id and other run options from root to all other processes
    // if run id received from the root <= 0 then all done, stop this child modeling process
    ModelStatus mStatus;
    broadcastInt(groupDef.groupOne, msgExec, (int *)&mStatus);
    broadcastInt(groupDef.groupOne, msgExec, &runId);

    theModelRunState.updateStatus(mStatus);     // update model status: progress, wait, shutdown, exit

    return runId;
}

/** model run shutdown: save results and update run status. */
void ChildController::shutdownRun(int /*i_runId*/)
{ 
    // wait for send completion, if any outstanding
    msgExec->waitSendAll();
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_type      parameter type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void ChildController::readParameter(const char * i_name, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // receive broadcasted parameter from root process
        msgExec->bcast(groupDef.groupOne, i_type, i_size, io_valueArr);
    }
    catch (exception & ex) {
        throw ModelException("Failed to read input parameter: %s. %s", i_name, ex.what());
    }
}

/** send output table accumulators to root process.
*
* @param[in]     i_runOpts      model run options
* @param[in]     i_isLastTable  if true then it is last output table to send
* @param[in]     i_name         output table name
* @param[in]     i_size         number of cells for each accumulator
* @param[in,out] io_accValues   accumulator values
*/
void ChildController::writeAccumulators(
    const RunOptions & i_runOpts, 
    bool /*i_isLastTable*/,
    const char * i_name, 
    size_t i_size, 
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
            (MsgTag)(AccReceive::accMsgTag(i_runOpts.subSampleNumber, subSampleCount, accIndex)),
            typeid(double),
            i_size,
            ap.release()
            );
        accIndex++;
    }
}
