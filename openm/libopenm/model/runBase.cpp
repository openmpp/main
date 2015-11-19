// OpenM++ modeling library: 
// model run basic support: initialize run, read input parameters and write output values
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
using namespace openm;

// model run basic support public interface
IRunBase::~IRunBase() throw() { }

// model run basic support
RunBase::RunBase(
    bool i_isMpiUsed,
    int i_modelId,
    int i_runId,
    int i_subSampleCount,
    int i_threadCount,
    IDbExec * i_dbExec,
    IMsgExec * i_msgExec,
    const MetaRunHolder * i_metaStore
    ) : 
    isMpiUsed(i_isMpiUsed),
    modelId(i_modelId),
    runId(i_runId),
    subCount(i_subSampleCount),
    threadCount(i_threadCount),
    dbExec(i_dbExec),
    msgExec(i_msgExec),
    metaStore(i_metaStore)
{
    // if trace log file enabled setup trace file name
    string traceFilePath;
    if (metaStore->runOption->boolValue(RunOptionsKey::traceToFile)) {
        traceFilePath = metaStore->runOption->strValue(RunOptionsKey::traceFilePath);
        if (traceFilePath.empty()) traceFilePath = metaStore->runOption->strValue(RunOptionsKey::setName) + ".txt";
    }

    // adjust trace log with actual settings specified in model run options
    theTrace->init(
        metaStore->runOption->boolValue(RunOptionsKey::traceToConsole),
        traceFilePath.c_str(),
        metaStore->runOption->boolValue(RunOptionsKey::traceUseTs),
        metaStore->runOption->boolValue(RunOptionsKey::traceUsePid),
        metaStore->runOption->boolValue(RunOptionsKey::traceNoMsgTime) || !metaStore->runOption->isExist(RunOptionsKey::traceNoMsgTime)
        );
}

// create new model run basic support
RunBase * RunBase::create(
    bool i_isMpiUsed, 
    int i_runId,
    int i_subCount,
    int i_threadCount,
    IDbExec * i_dbExec,
    IMsgExec * i_msgExec, 
    const MetaRunHolder * i_metaStore 
    )
{
    if (i_msgExec == NULL) throw MsgException("invalid (NULL) message passing interface");
    if (i_metaStore == NULL) throw ModelException("invalid (NULL) model metadata");
    if ((!i_isMpiUsed || i_msgExec->isRoot()) && i_dbExec == NULL) 
        throw ModelException("database connection must be open at process %d", i_msgExec->rank());

    // find model in metadata tables
    const ModelDicRow * mdRow = i_metaStore->modelDic->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
    if (mdRow == NULL) throw ModelException("model not found in the database");

    // create the run initialzer
    return new RunBase(i_isMpiUsed, mdRow->modelId, i_runId, i_subCount, i_threadCount, i_dbExec, i_msgExec, i_metaStore);
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_type      parameter type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void RunBase::readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // read parameter from db
        if (!isMpiUsed || msgExec->isRoot()) {
            unique_ptr<IParameterReader> reader(
                IParameterReader::create(modelId, runId, i_name, dbExec, metaStore)
                );
            reader->readParameter(dbExec, i_type, i_size, io_valueArr);
        }

        // broadcast parameter to all modeling processes
        if (isMpiUsed) msgExec->bcast(msgExec->rootRank, i_type, i_size, io_valueArr);
    }
    catch (exception & ex) {
        throw ModelException("Failed to read input parameter: %s. %s", i_name, ex.what());
    }
}

/** model shutdown on error: mark run as failure. */
void RunBase::shutdownOnFail(void)
{
    if (!isMpiUsed || msgExec->isRoot()) {
        dbExec->update(
            "UPDATE run_lst SET status = 'e'," \
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(runId)
            );
    }
}

/** model shutdown: save results and cleanup resources. */
void RunBase::shutdown(void)
{
    // find model in metadata tables
    const ModelDicRow * mdRow = metaStore->modelDic->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
    if (mdRow == NULL) throw ModelException("model not found in the database");

    // receive all output tables subsamples and write into database
    if (isMpiUsed && msgExec->isRoot()) {
        receiveSubSamples();
    }

    // wait for send completion, if any outstanding
    msgExec->waitSendAll();

    // update number of completed subsamples and check if all subsamples completed
    bool isAll = false;

    if (isMpiUsed && msgExec->isRoot()) {
        dbExec->update(
            "UPDATE run_lst SET sub_completed = sub_count," \
            " status = 'p'," \
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(runId)
            );
        isAll = true;   // all subsamples saved in database
    }
    if (!isMpiUsed) {
        unique_lock<recursive_mutex> lck = dbExec->beginTransactionThreaded();
        dbExec->update(
            "UPDATE run_lst SET sub_completed = sub_completed + " + to_string(threadCount) + ", " +
            " status = 'p'," \
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(runId)
            );
        int nSub = dbExec->selectToInt(
            "SELECT sub_completed FROM run_lst WHERE run_id = " + to_string(runId), -1
            );
        dbExec->commit();
        isAll = nSub == subCount;
    }

    // if all subsamples saved in database then calculate output tables aggregated values
    // and mark this run as completed
    if (isAll) {
#ifdef _DEBUG
        theLog->logMsg("Writing Output Tables Expressions");
#endif      
        writeOutputValues();
        dbExec->update(
            "UPDATE run_lst SET status = 's'," \
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(runId)
            );
    }
}

/** write output tables aggregated values into database */
void RunBase::writeOutputValues(void)
{
    const vector<TableDicRow> tblVec = metaStore->tableDic->byModelId(modelId);

    for (const TableDicRow & tblRow : tblVec) {
        unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
            modelId,
            runId,
            tblRow.tableName.c_str(),
            dbExec,
            metaStore,
            subCount
            ));
        writer->writeAllExpressions(dbExec);
    }
}


// helper struct to receive output table values for each accumulator
struct TblAccRecv
{
    int subId;              // subsample to receive: [threadCount,...N-1]
    int tableId;            // output table id
    int accId;              // accumulator id
    int accIndex;           // index of accumulator in the list of accumulators
    long long valueSize;    // size of accumulator data
    bool isReceived;        // if true then data received

    TblAccRecv(int i_subId, int i_tableId, int i_accId, int i_accIndex, long long i_valueSize) :
        subId(i_subId),
        tableId(i_tableId),
        accId(i_accId),
        accIndex(i_accIndex),
        valueSize(i_valueSize),
        isReceived(false)
    { }
};

/** receive all output tables subsamples and write into database */
void RunBase::receiveSubSamples()
{
    // receive output tables subsamples: [threadCount,..N-1]
    // receive all subsamples except of first threadCount subsamples, which caculated localy at root process
    if (subCount <= threadCount) return;  // exit if all subsamples was produced at root model process

    // get sparse settings
    bool isSparse = metaStore->runOption->boolValue(RunOptionsKey::useSparse);
    double nullValue = metaStore->runOption->doubleValue(RunOptionsKey::sparseNull, DBL_EPSILON);

    // init list of accumulators to be received
    vector<TblAccRecv> recvVec;
    const vector<TableAccRow> accVec = metaStore->tableAcc->byModelId(modelId);

    int tblId = -1;
    long long valSize = 0;
    for (int nAcc = 0; nAcc < (int)accVec.size(); nAcc++) {

        // get accumulator data size
        if (tblId != accVec[nAcc].tableId) {
            tblId = accVec[nAcc].tableId;
            valSize = IOutputTableWriter::sizeOf(modelId, metaStore, tblId);
        }

        for (int nSub = threadCount; nSub < subCount; nSub++) {
            recvVec.push_back(TblAccRecv(nSub, tblId, accVec[nAcc].accId, nAcc, valSize));
        }
    }

    // receive and save all accumulators
    bool isAnyToRecv = true;
    while (isAnyToRecv) {

        // try to receive accumulator data and save it into database
        bool isReceived = false;
        for (TblAccRecv & accRecv : recvVec) {

            if (!accRecv.isReceived) {      // if accumulator not yet received

                // allocate buffer to receive the data
                unique_ptr<double> valueUptr(new double[(int)accRecv.valueSize]);
                double * valueArr = valueUptr.get();

                // try to received
                isReceived = accRecv.isReceived = msgExec->tryReceive(
                    (accRecv.subId / threadCount),
                    (MsgTag)(((int)MsgTag::outSubsampleBase + accRecv.accIndex) * subCount + accRecv.subId),
                    typeid(double),
                    accRecv.valueSize,
                    valueArr
                    );
                if (!isReceived) continue;

                // accumulator received: write it into database
                const TableDicRow * tblRow = metaStore->tableDic->byKey(modelId, accRecv.tableId);
                if (tblRow == nullptr) throw new DbException("output table not found in table dictionary, id: %d", accRecv.tableId);

                unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
                    modelId,
                    runId,
                    tblRow->tableName.c_str(),
                    dbExec,
                    metaStore,
                    subCount,
                    isSparse,
                    nullValue
                    ));
                writer->writeAccumulator(dbExec, accRecv.subId, accRecv.accId, accRecv.valueSize, valueArr);

                break;  // data received
            }
        }

        // check if anything left to receive
        isAnyToRecv = std::any_of(
            recvVec.cbegin(),
            recvVec.cend(),
            [](TblAccRecv i_recv) -> bool { return !i_recv.isReceived; }
        );

        // no data received: if any accumulators outstanding then sleep before try again
        if (!isReceived && isAnyToRecv) sleepMilli(OM_RECV_SLEEP_TIME);
    }
}
