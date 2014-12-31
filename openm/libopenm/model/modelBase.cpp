// OpenM++ modeling library: base model class
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "model.h"
using namespace openm;

// model exception default error message
const char openm::modelUnknownErrorMessage[] = "unknown model error";

static recursive_mutex readMutex;   // mutex to lock read operations
static vector<int> paramReadVec;    // parameters id where read completed

// model public interface
IModel::~IModel() throw() { }

// base model class
ModelBase::ModelBase(
    bool i_isMpiUsed,
    int i_modelId,
    int i_runId,
    int i_subCount,
    int i_subNumber,
    IDbExec * i_dbExec,
    IMsgExec * i_msgExec,
    const MetaRunHolder * i_metaStore
    ) : 
    isMpiUsed(i_isMpiUsed),
    modelId(i_modelId),
    runId(i_runId),
    dbExec(i_dbExec),
    msgExec(i_msgExec),
    metaStore(i_metaStore),
    progressCount(0)
{
    if (i_msgExec == NULL) throw ModelException("invalid (NULL) message passing interface");
    if (i_metaStore == NULL) throw ModelException("invalid (NULL) model metadata");

    if ((!i_isMpiUsed || i_msgExec->isRoot()) && i_dbExec == NULL) 
        throw ModelException("database connection must be open at process %d", i_msgExec->rank());

    // set model run options
    runOpts.subSampleCount = i_subCount;
    runOpts.subSampleNumber = i_subNumber;
    runOpts.useSparse = metaStore->runOption->boolValue(RunOptionsKey::useSparse);
    runOpts.nullValue = metaStore->runOption->doubleValue(RunOptionsKey::sparseNull, DBL_EPSILON);

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

ModelBase::~ModelBase(void) throw()
{
    try {
        // theLog->logMsg("Model dtor");
    }
    catch (...) { }
}

// create new model
ModelBase * ModelBase::create(
    bool i_isMpiUsed, 
    int i_runId,
    int i_subCount,
    int i_subNumber,
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

    // create the model
    return new ModelBase(
        i_isMpiUsed, mdRow->modelId, i_runId, i_subCount, i_subNumber, i_dbExec, i_msgExec, i_metaStore
        );
}

/** model shutdown: save results and cleanup resources. */
void ModelBase::shutdown(
    bool i_isMpiUsed,
    int i_runId,
    int i_subCount,
    IDbExec * i_dbExec,
    IMsgExec * i_msgExec,
    const MetaRunHolder * i_metaStore
    )
{
    // find model in metadata tables
    const ModelDicRow * mdRow = i_metaStore->modelDic->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
    if (mdRow == NULL) throw ModelException("model not found in the database");

    // receive all output tables subsamples and write into database
    if (i_isMpiUsed && i_msgExec->isRoot()) {
        receiveSubSamples(mdRow->modelId, i_runId, i_subCount, i_dbExec, i_msgExec, i_metaStore);
    }

    // wait for send completion, if any outstanding
    i_msgExec->waitSendAll();

    // update number of completed subsamples and check if all subsamples completed
    bool isAll = false;

    if (i_isMpiUsed && i_msgExec->isRoot()) {
        i_dbExec->update(
            "UPDATE run_lst SET sub_completed = sub_count," \
            " status = 'p'," \
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(i_runId)
            );
        isAll = true;   // all subsamples saved in database
    }
    if (!i_isMpiUsed) {
        i_dbExec->beginTransaction();
        i_dbExec->update(
            "UPDATE run_lst SET sub_completed = sub_completed + 1," \
            " status = 'p'," \
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(i_runId)
            );
        isAll = i_subCount == i_dbExec->selectToInt(
            "SELECT sub_completed FROM run_lst WHERE run_id = " + to_string(i_runId), -1
            );
        i_dbExec->commit();
    }

    // if all subsamples saved in database then calculate output tables aggregated values
    // and mark this run as completed
    if (isAll) {

        writeOutputValues(mdRow->modelId, i_runId, i_subCount, i_dbExec, i_metaStore);

        i_dbExec->update(
            "UPDATE run_lst SET status = 's'," \
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(i_runId)
            );
    }
}

/** model shutdown on error: mark run as failure. */
void ModelBase::shutdownOnFail(bool i_isMpiUsed, int i_runId, IDbExec * i_dbExec, IMsgExec * i_msgExec)
{
    if (!i_isMpiUsed || i_msgExec->isRoot()) {
        i_dbExec->update(
            "UPDATE run_lst SET sub_completed = sub_count," \
            " status = 'e'," \
            " update_dt = " + toQuoted(makeDateTime(chrono::system_clock::now())) +
            " WHERE run_id = " + to_string(i_runId)
            );
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
void ModelBase::readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) input parameter name");

    try {
        // parameters shared between threads: read only in one thread
        lock_guard<recursive_mutex> lck(readMutex);

        // get parameter id by name
        const ParamDicRow * paramRow = metaStore->paramDic->byModelIdName(modelId, i_name);
        if (paramRow == NULL) throw DbException("parameter not found in parameters dictionary: %s", i_name);

        // check if parameter read already done
        int paramId = paramRow->paramId;
        bool isDone = std::any_of(
            paramReadVec.cbegin(), 
            paramReadVec.cend(), 
            [paramId](const int i_id) -> bool { return paramId == i_id; }
        );
        if (isDone) return;     // parameter read already done

        // read parameter from db
        if (!isMpiUsed || msgExec->isRoot()) {
            unique_ptr<IParameterReader> reader(
                IParameterReader::create(modelId, runId, i_name, dbExec, metaStore)
                );
            reader->readParameter(dbExec, i_type, i_size, io_valueArr);
        }

        // broadcast parameter to all modeling processes
        if (isMpiUsed) msgExec->bcast(msgExec->rootRank, i_type, i_size, io_valueArr);

        paramReadVec.push_back(paramId);    // parameter read completed
    }
    catch (exception & ex) {
        throw ModelException("Failed to read input parameter: %s. %s", i_name, ex.what());
    }
}

/**
* write output result table: array of accumulator values.
*
* @param[in] i_name      output table name
* @param[in] i_accCount  number of accumulators for the output table
* @param[in] i_size      number of values for each accumulator
* @param[in] i_valueArr  array of pointers to accumulator values
*
* usage example: \n
* @code
*      // declare model output table accumulators
*      static double salaryBySexSum[salarySize][sexSize];
*      static double salaryBySexCount[salarySize][sexSize];
*      ...
*      // run the model and calculate accumulator values in arrays above
*      ...
*      // write output table "salarySex" into database
*      const double * accArr[2];
*      accArr[0] = (double *)salaryBySexSum;
*      accArr[1] = (double *)salaryBySexCount;
*      i_model->writeOutputTable("salarySex", 2, salarySize * sexSize, salarySexAccArr);
* @endcode
*/
void ModelBase::writeOutputTable(const char * i_name, int i_accCount, long long i_size, const double * i_valueArr[])
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) output table name");
    if (i_accCount <= 0 || i_size <= 0) throw ModelException("invalid number of accumulators: %d or size: %lld for output table %s", i_accCount, i_size, i_name);
    if (i_valueArr == NULL) throw ModelException("invalid (empty) value array for output table %s", i_name);

    try {
        if (!isMpiUsed || msgExec->isRoot()) {  // write subsample into db: subsample number =0 or any if no MPI used

            unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
                modelId,
                runId,
                i_name, 
                dbExec, 
                metaStore,
                subSampleCount(), 
                runOptions()->useSparse, 
                runOptions()->nullValue
                ));
            for (int nAcc = 0; nAcc < i_accCount; nAcc++) {
                writer->writeAccumulator(dbExec, subSampleNumber(), nAcc, i_size, i_valueArr[nAcc]);
            }
        }
        else {  // send subsample data to root process

            // find output table db row
            const TableDicRow * tblRow = metaStore->tableDic->byModelIdName(modelId, i_name);
            if (tblRow == nullptr) throw new DbException("output table not found in table dictionary: %s", i_name);

            int tblId = tblRow->tableId;
            const vector<TableAccRow> accVec = metaStore->tableAcc->byModelId(modelId);

            // find starting index of table accumulators in db rows of accumulators
            int accPos = 0;
            while (accPos < (int)accVec.size() && accVec[accPos].tableId != tblId) {
                accPos++;
            }
            if (accPos >= (int)accVec.size()) throw ModelException("accumulators not found for output table %s", i_name);

            // send accumulators
            for (int nAcc = 0; nAcc < i_accCount; nAcc++) {
                msgExec->startSend(
                    IMsgExec::rootRank, 
                    (MsgTag)((int)MsgTag::outSubsampleBase + accPos + nAcc), 
                    typeid(double), 
                    i_size, 
                    (void *)(i_valueArr[nAcc])
                    );
            }
        }
    }
    catch (exception & ex) {
        throw ModelException("Failed to write output table: %s. %s", i_name, ex.what());
    }
}

/** write output tables aggregated values into database */
void ModelBase::writeOutputValues(
    int i_modelId, int i_runId, int i_subCount, IDbExec * i_dbExec, const MetaRunHolder * i_metaStore
    )
{
    const vector<TableDicRow> tblVec = i_metaStore->tableDic->byModelId(i_modelId);

    for (const TableDicRow & tblRow : tblVec) {

        unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
            i_modelId,
            i_runId,
            tblRow.tableName.c_str(),
            i_dbExec,
            i_metaStore,
            i_subCount
            ));

        writer->writeAllExpressions(i_dbExec);
    }
}

// helper struct to receive output table values for each accumulator
struct TblAccRecv
{
    int subId;              // subsample to receive: [1,...N-1]
    int tableId;            // output table id
    int accId;              // accumulator id
    int accMsgTag;          // message tag for that accumulator: index of accumulator in the list of accumulators
    long long valueSize;    // size of accumulator data
    bool isReceived;        // if true then data received

    TblAccRecv(int i_subId, int i_tableId, int i_accId, int i_accMsgTag, long long i_valueSize) :
        subId(i_subId),
        tableId(i_tableId),
        accId(i_accId),
        accMsgTag(i_accMsgTag),
        valueSize(i_valueSize),
        isReceived(false)
    { }
};

/** receive all output tables subsamples and write into database */
void ModelBase::receiveSubSamples(
    int i_modelId, int i_runId, int i_subCount, IDbExec * i_dbExec, IMsgExec * i_msgExec, const MetaRunHolder * i_metaStore
    )
{
    // receive output tables subsamples: [1,..N-1]
    // receive all subsamples except of zero subsample, which caculated localy at root process
    if (i_subCount <= 1) return;  // exit if only one subsample was produced by model

    // get sparse settings
    bool isSparse = i_metaStore->runOption->boolValue(RunOptionsKey::useSparse);
    double nullValue = i_metaStore->runOption->doubleValue(RunOptionsKey::sparseNull, DBL_EPSILON);

    // init list of accumulators to be received
    vector<TblAccRecv> recvVec;
    const vector<TableAccRow> accVec = i_metaStore->tableAcc->byModelId(i_modelId);

    int tblId = -1;
    long long valSize = 0;
    for (int nAcc = 0; nAcc < (int)accVec.size(); nAcc++) {

        // get accumulator data size
        if (tblId != accVec[nAcc].tableId) {
            tblId = accVec[nAcc].tableId;
            valSize = IOutputTableWriter::sizeOf(i_modelId, i_metaStore, tblId);
        }

        for (int nSub = 1; nSub < i_subCount; nSub++) {
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
                isReceived = accRecv.isReceived = i_msgExec->tryReceive(
                    accRecv.subId,
                    (MsgTag)((int)MsgTag::outSubsampleBase + accRecv.accMsgTag),
                    typeid(double),
                    accRecv.valueSize,
                    valueArr
                    );
                if (!isReceived) continue;

                // accumulator received: write it into database
                const TableDicRow * tblRow = i_metaStore->tableDic->byKey(i_modelId, accRecv.tableId);
                if (tblRow == nullptr) throw new DbException("output table not found in table dictionary, id: %d", accRecv.tableId);

                unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
                    i_modelId,
                    i_runId,
                    tblRow->tableName.c_str(),
                    i_dbExec,
                    i_metaStore,
                    i_subCount,
                    isSparse,
                    nullValue
                    ));
                writer->writeAccumulator(i_dbExec, accRecv.subId, accRecv.accId, accRecv.valueSize, valueArr);

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
