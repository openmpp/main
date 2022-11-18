/**
* @file
* OpenM++ modeling library: 
* child process run controller class to receive input parameters from root and send output tables.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "helper.h"
#include "model.h"
#include "modelHelper.h"
#include "runControllerImpl.h"

using namespace std;
using namespace openm;

/** initialize child modeling process.
*
* - (a) get number of sub-values, processes, threads, groups
* - (d) receive metadata tables from root processes
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
* Number of modeling processes = MPI world size \n
*
* Size of modeling group = number of sub-values / number of threads \n
* Number of groups = number of processes / group size \n
*   by default only one group which include all modeling processes \n
*   group size always >= 1 therefore group always include at least one process \n
*
* Number of sub-values per child process = total number of sub-values / group size \n
*   if total number of sub-values % group size != 0 then remainder calculated at last group process \n
*/
void ChildController::init(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");

    // broadcast basic run options from root to all other processes
    int nRootIdle = 0;
    msgExec->bcastInt(ProcessGroupDef::all, &subValueCount);
    msgExec->bcastInt(ProcessGroupDef::all, &threadCount);
    msgExec->bcastInt(ProcessGroupDef::all, &nRootIdle);
    msgExec->bcastInt(ProcessGroupDef::all, &modelId);

    // basic validation: number of processes expected to be > 1
    if (subValueCount <= 0) throw ModelException("Invalid number of sub-values: %d", subValueCount);
    if (threadCount <= 0) throw ModelException("Invalid number of modeling threads: %d", threadCount);
    // if (processCount <= 1) throw ModelException("Invalid number of modeling processes: %d", processCount);

    // create groups for parallel run of modeling task
    groupDef = ProcessGroupDef(subValueCount, threadCount, (nRootIdle != 0), msgExec->worldSize(), msgExec->rank());

    msgExec->createGroups(groupDef.groupSize, groupDef.groupCount);

    // first sub-value index and number of sub-values
    subFirstId = groupDef.activeRank * groupDef.subPerProcess;
    selfSubCount = groupDef.selfSubCount;

    if (subFirstId < 0 || selfSubCount <= 0 || subFirstId + selfSubCount > subValueCount)
        throw ModelException(
            "Invalid first sub-value index: %d or number of sub-values: %d", subFirstId, selfSubCount
            );

    // receive metadata tables from root process
    // receive basic model run options
    // receive broadcasted model messages from root
    metaStore.reset(new MetaHolder);
    modelId = broadcastMetaData();
    broadcastRunOptions();
    broadcastLanguageMessages();

    // adjust number of active processes: exit from unused child processes
    if (msgExec->rank() > groupDef.groupSize * groupDef.groupCount) {
        theModelRunState->updateStatus(ModelStatus::exit);
    }
}

/** receive broadcasted metadata tables from root process. */
int ChildController::broadcastMetaData(void)
{
    broadcastMetaTable<IModelDicTable>(MsgTag::modelDic, metaStore->modelTable);
    broadcastMetaTable<ITypeDicTable>(MsgTag::typeDic, metaStore->typeDic);
    broadcastMetaTable<ITypeEnumLstTable>(MsgTag::typeEnumLst, metaStore->typeEnumLst);
    broadcastMetaTable<IParamDicTable>(MsgTag::parameterDic, metaStore->paramDic);
    broadcastMetaTable<IParamDimsTable>(MsgTag::parameterDims, metaStore->paramDims);
    broadcastMetaTable<ITableDicTable>(MsgTag::tableDic, metaStore->tableDic);
    broadcastMetaTable<ITableDimsTable>(MsgTag::tableDims, metaStore->tableDims);
    broadcastMetaTable<ITableAccTable>(MsgTag::tableAcc, metaStore->tableAcc);
    broadcastMetaTable<ITableExprTable>(MsgTag::tableExpr, metaStore->tableExpr);

    // find model by name digest
    metaStore->modelRow = metaStore->modelTable->byNameDigest(OM_MODEL_NAME, OM_MODEL_DIGEST);
    if (metaStore->modelRow == nullptr) throw DbException("model %s not found in the database", OM_MODEL_NAME);

    return metaStore->modelRow->modelId;
}

/** receive broadcasted meta table db rows from root process. */
template <typename MetaTbl>
void ChildController::broadcastMetaTable(MsgTag i_msgTag, unique_ptr<MetaTbl> & io_tableUptr)
{
    unique_ptr<IPackedAdapter> packAdp(IPackedAdapter::create(i_msgTag));

    IRowBaseVec rv;
    msgExec->bcastReceivePacked(ProcessGroupDef::all, rv, *packAdp);
    io_tableUptr.reset(MetaTbl::create(rv));
}

/** receive broadcasted basic model run options from root process. */
void ChildController::broadcastRunOptions(void)
{
    RunOptions opts;
    msgExec->bcastInt(ProcessGroupDef::all, &opts.subValueCount);
    msgExec->bcastInt(ProcessGroupDef::all, &opts.subValueId);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(bool), &opts.useSparse);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(double), &opts.nullValue);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(bool), &opts.isDbMicrodata);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(bool), &opts.isCsvMicrodata);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(bool), &opts.isTraceMicrodata);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(bool), &opts.isMicrodataEvents);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(int), &opts.progressPercent);
    msgExec->bcastValue(ProcessGroupDef::all, typeid(double), &opts.progressStep);

    setRunOptions(opts);    // update model run options

    // broadcast number of parameters with sub-values and parameters id
    int n = (int)paramIdSubArr.size();
    msgExec->bcastInt(ProcessGroupDef::all, &n);

    if (n > 0) {
        paramIdSubArr.resize(n);
        msgExec->bcastReceive(ProcessGroupDef::all, typeid(int), n, paramIdSubArr.data());
    }

    // broadcast number of microdata attributes and attribute indices in entity array
    n = (int)entityIdxArr.size();
    msgExec->bcastInt(ProcessGroupDef::all, &n);

    if (n > 0) {
        entityIdxArr.resize(n);
        msgExec->bcastReceive(ProcessGroupDef::all, typeid(int), n, entityIdxArr.data());
    }
}

/** receive broadcasted model messages from root process. */
void ChildController::broadcastLanguageMessages(void)
{
    // broadcast from root to all child processes
    IRowBaseVec codeValueVec;
    unique_ptr<IPackedAdapter> packAdp(IPackedAdapter::create(MsgTag::codeValue));
    msgExec->bcastReceivePacked(ProcessGroupDef::all, codeValueVec, *packAdp);

    // unpack messages
    unordered_map<string, string> msgMap;
    for (auto it = codeValueVec.cbegin(); it != codeValueVec.cend(); it++) {
        CodeValueRow * cvRow = dynamic_cast<CodeValueRow *>(it->get());
        msgMap[cvRow->code.c_str()] = cvRow->value.c_str();
    }

    // set language specific message for the log
    list<string> langLst = theLog->getLanguages();
    if (!msgMap.empty()) theLog->swapLanguageMessages(langLst, msgMap);
}

/** next run for child process: receive run id, run options and input parameters from root process. */
int ChildController::nextRun(void)
{
    if (msgExec == nullptr) throw MsgException("invalid (NULL) message passing interface");

    if (theModelRunState->isShutdownOrFinal()) return 0;      // exit if status not a continue status

    // broadcast metadata: model status, run id and other run options from root to all other processes
    // if run id received from the root <= 0 then all done, stop this child modeling process
    ModelStatus mStatus;
    msgExec->bcastInt(groupDef.groupOne, (int *)&mStatus);
    msgExec->bcastInt(groupDef.groupOne, &runId);

    // receive run options from root process
    if (runId > 0) {
        unique_ptr<IPackedAdapter> packAdp(IPackedAdapter::create(MsgTag::runOption));

        IRowBaseVec rv;
        msgExec->bcastReceivePacked(groupDef.groupOne, rv, *packAdp);
        metaStore->runOptionTable.reset(IRunOptionTable::create(rv));
    }

    theModelRunState->updateStatus(mStatus);     // update model status: progress, wait, shutdown, exit, done
    isFinalExchange = RunState::isFinal(mStatus);

    if (!isFinalExchange && runId > 0) openCsvMicrodata(runId); // create microdata CSV files for new model run

    return runId;
}

/** model process shutdown if exiting without completion (ie: exit on error). */
void ChildController::shutdownOnExit(ModelStatus i_status) {
    theModelRunState->updateStatus(i_status);
    if (!isFinalExchange) sendStatusUpdate();   // send last status update
}
 
/** model process shutdown: cleanup resources. */
void ChildController::shutdownWaitAll(void)
{
    ModelStatus mStatus = theModelRunState->updateStatus(ModelStatus::done);    // set model status as completed OK

    if (!isFinalExchange) {
        sendStatusUpdate();     // send last status update
        msgExec->waitSendAll(); // wait for send completion, if any outstanding
    }

    if (!RunState::isError(mStatus)) {
        msgExec->setCleanExit(true);    // if model status not error then do clean shutdown of MPI
    }
}

/** model run shutdown: save results and update run status. */
void ChildController::shutdownRun(int /*i_runId*/)
{ 
    sendStatusUpdate();     // send status update for that run
    msgExec->waitSendAll(); // wait for send completion, if any outstanding
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_subId     parameter sub-value index
* @param[in]     i_type      parameter type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void ChildController::readParameter(const char * i_name, int /*i_subId*/, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // receive broadcasted parameter from root process
        msgExec->bcastReceive(groupDef.groupOne, i_type, i_size, io_valueArr);
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
    forward_list<unique_ptr<double[]> > & io_accValues
    )
{
    // find output table db row and accumulators
    const TableDicRow * tblRow = metaStore->tableDic->byModelIdName(modelId, i_name);
    if (tblRow == nullptr) throw DbException("output table not found in tables dictionary: %s", i_name);

    int accIndex = (int)metaStore->tableAcc->indexOf(
        [&](const TableAccRow & i_row) -> bool { return i_row.modelId == modelId && i_row.tableId == tblRow->tableId; }
    );
    if (accIndex < 0) throw DbException("output table accumulators not found: %s", i_name);

    // send accumulators to root process
    for (auto & ap : io_accValues) {
        msgExec->startSend(
            IMsgExec::rootRank,
            (MsgTag)(AccReceive::accMsgTag(i_runOpts.subValueId, subValueCount, accIndex)),
            typeid(double),
            i_size,
            ap.release()
            );
        accIndex++;
    }
}

/** exchange between root and child process to send and receive status update. */
bool ChildController::childExchange(void) 
{
    // wait for send completion, if any outstanding
    msgExec->waitSendAll();

    // get process-wide model run state
    // if model status same and last progress report sent recently then exit
    auto nowTime = chrono::system_clock::now();
    ModelStatus mStatus = theModelRunState->status();
    if (mStatus == lastModelStatus && nowTime < lastTimeStatus + chrono::milliseconds(OM_WAIT_SLEEP_TIME)) {
        return false;
    }
    
    sendStatusUpdate();     // send status update

    lastModelStatus = mStatus;
    lastTimeStatus = nowTime;
    isFinalExchange = RunState::isFinal(mStatus);
    return true;
}

/** send sub-values run status update to root */
void ChildController::sendStatusUpdate(void)
{
    IRowBaseVec rsVec = runStateStore().saveToRowVector(runId);
    if (rsVec.size() > 0) {
        unique_ptr<IPackedAdapter> packAdp(IPackedAdapter::create(MsgTag::statusUpdate));
        msgExec->startSendPacked(IMsgExec::rootRank, rsVec, *packAdp);
    }
}

/** write microdata into database. */
void ChildController::writeDbMicrodata(const EntityItem & i_entityItem, uint64_t i_microdataKey, int i_eventId, const void * i_entityThis, string & /* io_line */)
{
}
