// OpenM++ modeling library: base model class
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "model.h"
using namespace openm;

// model exception default error message
const char openm::modelUnknownErrorMessage[] = "unknown model error";

// model public interface
IModel::~IModel() throw() { }

// base model class
ModelBase::ModelBase(
    bool i_isMpiUsed,
    int i_modelId,
    int i_subCount,
    int i_subNumber,
    IDbExec * i_dbExec,
    IMsgExec * i_msgExec,
    const MetaRunHolder * i_metaStore
    ) : 
    modelId(i_modelId),
    isMpiUsed(i_isMpiUsed),
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
    int runId = metaStore->runId;

    runOpts.subSampleCount = i_subCount;
    runOpts.subSampleNumber = i_subNumber;
    runOpts.useSparse = metaStore->runOption->boolValue(runId, RunOptionsKey::useSparse);
    runOpts.nullValue = metaStore->runOption->doubleValue(runId, RunOptionsKey::sparseNull, DBL_EPSILON);

    // if trace log file enabled setup trace file name
    string traceFilePath;
    if (metaStore->runOption->boolValue(runId, RunOptionsKey::traceToFile)) {
        traceFilePath = metaStore->runOption->strValue(runId, RunOptionsKey::traceFilePath);
        if (traceFilePath.empty()) traceFilePath = metaStore->runOption->strValue(runId, RunOptionsKey::setName) + ".txt";
    }

    // adjust trace log with actual settings specified in model run options
    theTrace->init(
        metaStore->runOption->boolValue(runId, RunOptionsKey::traceToConsole),
        traceFilePath.c_str(),
        metaStore->runOption->boolValue(runId, RunOptionsKey::traceUseTs),
        metaStore->runOption->boolValue(runId, RunOptionsKey::traceUsePid),
        metaStore->runOption->boolValue(runId, RunOptionsKey::traceNoMsgTime) || !metaStore->runOption->isExist(runId, RunOptionsKey::traceNoMsgTime)
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
        i_isMpiUsed, mdRow->modelId, i_subCount, i_subNumber, i_dbExec, i_msgExec, i_metaStore
        );
}

/** model shutdown: save results and cleanup resources. */
void ModelBase::shutdown(void)
{
    // receive all output tables subsamples and write into database
    if (isMpiUsed && msgExec->isRoot()) receiveSubSamples();

    // wait for send completion, if any outstanding
    msgExec->waitSendAll();

    // update number of completed subsamples and check if all subsamples completed
    bool isAll = false;

    if (isMpiUsed && msgExec->isRoot()) {
        dbExec->update(
            "UPDATE run_lst SET sub_completed = sub_count WHERE run_id = " + to_string(metaStore->runId)
            );
        isAll = true;   // all subsamples saved in database
    }
    if (!isMpiUsed) {
        dbExec->beginTransaction();
        dbExec->update(
            "UPDATE run_lst SET sub_completed = sub_completed + 1 WHERE run_id = " + to_string(metaStore->runId)
            );
        isAll = subSampleCount() == dbExec->selectToInt(
            "SELECT sub_completed FROM run_lst WHERE run_id = " + to_string(metaStore->runId), -1
            );
        dbExec->commit();
    }

    // if all subsamples saved in database then calculate output tables aggregated values
    if (isAll) writeOutputValues();
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
        // read parameter from db
        if (!isMpiUsed || msgExec->isRoot()) {
            unique_ptr<IParameterReader> reader(
                IParameterReader::create(modelId, metaStore->runId, i_name, dbExec, metaStore)
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

/** write output tables aggregated values into database */
void ModelBase::writeOutputValues(void)
{
    const vector<TableDicRow> tblVec = metaStore->tableDic->byModelId(modelId);

    for (const TableDicRow & tblRow : tblVec) {

        unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
            modelId, 
            metaStore->runId,
            tblRow.tableName.c_str(), 
            dbExec, 
            metaStore,
            subSampleCount(), 
            runOptions()->useSparse, 
            runOptions()->nullValue
            ));

            writer->writeAllExpressions(dbExec);
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
                metaStore->runId,
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
void ModelBase::receiveSubSamples(void)
{
    // receive output tables subsamples: [1,..N-1]
    // receive all subsamples except of zero subsample, which caculated localy at root process
    if (subSampleCount() <= 1) return;  // exit if only one subsample was produced by model

    // init list of accumulators to be received
    vector<TblAccRecv> recvVec;
    const vector<TableAccRow> accVec = metaStore->tableAcc->byModelId(modelId);

    int tblId = -1;
    for (int nAcc = 0; nAcc < (int)accVec.size(); nAcc++) {

        // get accumulator data size
        long long valSize = 0;
        if (tblId != accVec[nAcc].tableId) {
            tblId = accVec[nAcc].tableId;
            valSize = IOutputTableWriter::sizeOf(modelId, metaStore, tblId);
        }

        for (int nSub = 1; nSub < subSampleCount(); nSub++) {
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
                    accRecv.subId,
                    (MsgTag)((int)MsgTag::outSubsampleBase + accRecv.accMsgTag),
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
                    metaStore->runId,
                    tblRow->tableName.c_str(),
                    dbExec,
                    metaStore,
                    runOptions()->subSampleCount,
                    runOptions()->useSparse,
                    runOptions()->nullValue
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
