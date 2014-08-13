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
    if (i_metaStore == NULL) throw ModelException("invalid (NULL) metadata tables storage");

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
    if (i_metaStore == NULL) throw ModelException("invalid (NULL) metadata tables storage");
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
            writer->writeSubSample(dbExec, subSampleNumber(), i_accCount, i_size, i_valueArr);
        }
        else {  // send subsample data to root process

            // find starting index of table accumulators in db rows of accumulators
            int tblId = metaStore->tableDic->byModelIdName(modelId, i_name)->tableId;
            const vector<TableAccRow> accVec = metaStore->tableAcc->byModelId(modelId);

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
struct TblSizeRecv
{
    int tableId;            // output table id
    int accStartPos;        // index of first accumulator in the list of accumulators
    int accCount;           // number of accumulators 
    long long valueSize;    // size of accumulator data

    TblSizeRecv(int i_tableId, int i_accStartPos, int i_accCount, long long i_valueSize) :
        tableId(i_tableId),
        accStartPos(i_accStartPos),
        accCount(i_accCount),
        valueSize(i_valueSize)
    { }

    // find by table id
    static vector<TblSizeRecv>::const_iterator byTableId(int i_tableId, const vector<TblSizeRecv> & i_tblSizeVec)
    {
        vector<TblSizeRecv>::const_iterator tblSizeIt = std::find_if(
            i_tblSizeVec.cbegin(), 
            i_tblSizeVec.cend(), 
            [i_tableId](const TblSizeRecv & i_tblSize) -> bool { return i_tblSize.tableId == i_tableId; }
            );
        if (tblSizeIt == i_tblSizeVec.cend()) throw ModelException("output table size info not found, id: %d", i_tableId);

        return tblSizeIt;
    }
};

// collect each table size and accumulators position
static void collectTableSize(
    int i_modelId, const MetaRunHolder * i_metaStore, const vector<TableAccRow> & i_accVec, vector<TblSizeRecv> & io_tblSizeVec
    );

// try to receive accumulator data for any subsample of any output table, return true if received
static bool tryReceiveAccumulator(
    int i_subSampleCount,
    IMsgExec * i_msgExec,
    const vector<TableAccRow> & i_accVec, 
    const vector<TblSizeRecv> & i_tblSizeVec, 
    vector<bool> & io_isRecvVec, 
    vector<unique_ptr<double> > & io_valueVec
    );


// if there is a output table subsample where all accumulators received 
// then save such table(s) in database and release accumulators data buffers
// return true if any subsample saved
static bool writeSubSamples(
    int i_modelId, 
    IDbExec * i_dbExec, 
    const MetaRunHolder * i_metaStore,
    const RunOptions * i_runOpt,
    const vector<TableDicRow> & i_tblVec,
    const vector<TableAccRow> & i_accVec, 
    const vector<TblSizeRecv> & i_tblSizeVec, 
    const vector<bool> & i_isRecvVec, 
    vector<unique_ptr<double> > & io_valueVec,
    vector<bool> & io_isSavedVec
    );

/** receive all output tables subsamples and write into database */
void ModelBase::receiveSubSamples(void)
{
    // receive output tables subsamples: [1,..N-1]
    // receive all sabsamples except of zero subsample, which caculated localy at root process
    if (subSampleCount() <= 1) return;  // exit if only one subsample was produced by model

    // collect tables and accumulators size and position
    const vector<TableDicRow> tblVec = metaStore->tableDic->byModelId(modelId);
    const vector<TableAccRow> accVec = metaStore->tableAcc->byModelId(modelId);
    int tblSize = (int)tblVec.size();
    int accSize = (int)accVec.size();
    int subSize = subSampleCount() - 1;

    vector<TblSizeRecv> tblSizeVec;
    collectTableSize(modelId, metaStore, accVec, tblSizeVec);
    if (tblSize != (int)tblSizeVec.size()) throw new DbException("invalid output tables accumulators list");

    vector<bool> isRecvVec(subSize * accSize, false);           // received flags: if true then table accumulator data received
    vector<unique_ptr<double> > valueVec(subSize * accSize);    // buffers to receive the data
    vector<bool> isSavedVec(subSize * tblSize, false);          // saved flags: if true then table subsample already saved in database

    // receive and save accumulators for all subsamples
    bool isAnyToRecv = true;
    while (isAnyToRecv) {

        // try to receive accumulator data for any subsample of any output table
        bool isReceived = tryReceiveAccumulator(subSampleCount(), msgExec, accVec, tblSizeVec, isRecvVec, valueVec);

        isAnyToRecv = std::any_of(
            isRecvVec.cbegin(),
            isRecvVec.cend(),
            [](bool i_isRecv) -> bool { return !i_isRecv; }
        );

         // no data received: if any subsamples outstanding then sleep before try again
        if (!isReceived) {
            if (isAnyToRecv) sleepMilli(OM_RECV_SLEEP_TIME);
            continue;
        }

        // if data received and there is some tables where all accumulators received 
        // then save such tables and release accumulators data buffers
        bool isAnySaved = false;
        do {
            isAnySaved = writeSubSamples(
                modelId, dbExec, metaStore, runOptions(), tblVec, accVec, tblSizeVec, isRecvVec, valueVec, isSavedVec
                );
        } while (isAnySaved);
    }    
}

// try to receive accumulator data for any subsample of any output table, return true if received
bool tryReceiveAccumulator(
    int i_subSampleCount,
    IMsgExec * i_msgExec,
    const vector<TableAccRow> & i_accVec, 
    const vector<TblSizeRecv> & i_tblSizeVec, 
    vector<bool> & io_isRecvVec, 
    vector<unique_ptr<double> > & io_valueVec
    )
{
    int subSize = i_subSampleCount - 1;     // receive [1, N-1] subsample, zero subsample produced by root process
    int accSize = (int)i_accVec.size();
    bool isReceived = false;

    for (int nSub = 0; !isReceived && nSub < subSize; nSub++) {
        for (int nAcc = 0; !isReceived && nAcc < accSize; nAcc++) {

            int recvPos = nSub * accSize + nAcc;

            if (!io_isRecvVec[recvPos]) {       // if this subsample not yet received

                // find size of the accumulator for current table
                long long valueSize = TblSizeRecv::byTableId(i_accVec[nAcc].tableId, i_tblSizeVec)->valueSize;

                // allocate buffer to receive the data
                unique_ptr<double> valueUptr(new double[(int)valueSize]);
                double * valueArr = valueUptr.get();

                // try to received
                isReceived = i_msgExec->tryReceive(
                    nSub + 1, 
                    (MsgTag)((int)MsgTag::outSubsampleBase + nAcc),
                    typeid(double), 
                    valueSize, 
                    valueArr
                    );

                // if received move data to main vector of accumulator values
                if (isReceived) {
                    io_valueVec[recvPos].swap(valueUptr);
                    io_isRecvVec[recvPos] = true;
                }
            }
        }
    }

    return isReceived;
}

// if there is a output table subsample where all accumulators received 
// then save such table(s) in database and release accumulators data buffers
// return true if any subsample saved
bool writeSubSamples(
    int i_modelId, 
    IDbExec * i_dbExec, 
    const MetaRunHolder * i_metaStore,
    const RunOptions * i_runOpt,
    const vector<TableDicRow> & i_tblVec,
    const vector<TableAccRow> & i_accVec, 
    const vector<TblSizeRecv> & i_tblSizeVec, 
    const vector<bool> & i_isRecvVec, 
    vector<unique_ptr<double> > & io_valueVec,
    vector<bool> & io_isSavedVec
    )
{
    int subSize = i_runOpt->subSampleCount - 1;         // receive [1, N-1] subsample, zero subsample produced by root process
    int tblSize = (int)i_tblVec.size();
    int accSize = (int)i_accVec.size();

    bool isAnySaved = false;

    // find table subsample where all accumulators received and not yet saved
    for (int nTbl = 0; nTbl < tblSize; nTbl++) {
                        
        vector<TblSizeRecv>::const_iterator tblSizeIt = TblSizeRecv::byTableId(i_tblVec[nTbl].tableId, i_tblSizeVec);

        // check if all accumulators received 
        for (int nSub = 0; nSub < subSize; nSub++) {
                        
            int savedPos = nSub * tblSize + nTbl;
            if (io_isSavedVec[savedPos]) continue;      // output table subsample already saved

            // check if all accumulators for the table subsample received
            bool isAllRecv = true;
            for (int nAcc = 0; isAllRecv && nAcc < tblSizeIt->accCount; nAcc++) {
                isAllRecv &= i_isRecvVec[nSub * accSize + nAcc + tblSizeIt->accStartPos];
            }
            if (!isAllRecv) continue;       // accumultor(s) not yet received, skip to next table

            // write subsample accumulators data in database
            {
                // get pointers to received accumulators data
                unique_ptr<const double *> accUptr(new const double * [tblSizeIt->accCount]);
                const double ** accArr = accUptr.get();

                for (int nAcc = 0; nAcc < tblSizeIt->accCount; nAcc++) {
                    accArr[nAcc] = io_valueVec[nSub * accSize + nAcc + tblSizeIt->accStartPos].get();
                }

                // save accumulators data in database
                unique_ptr<IOutputTableWriter> writer(IOutputTableWriter::create(
                    i_modelId, 
                    i_metaStore->runId,
                    i_tblVec[nTbl].tableName.c_str(), 
                    i_dbExec, 
                    i_metaStore,
                    i_runOpt->subSampleCount, 
                    i_runOpt->useSparse, 
                    i_runOpt->nullValue
                    ));
                writer->writeSubSample(i_dbExec, nSub + 1, tblSizeIt->accCount, tblSizeIt->valueSize, accArr);
            }

            isAnySaved = io_isSavedVec[savedPos] = true;   // subsample accumulators saved

            // release accumulators data bufferes
            for (int nAcc = 0; nAcc < tblSizeIt->accCount; nAcc++) {
                io_valueVec[nSub * accSize + nAcc + tblSizeIt->accStartPos].release();
            }
        }
    }

    return isAnySaved;
}

// collect each table size and  accumulators position
void collectTableSize(
    int i_modelId, const MetaRunHolder * i_metaStore, const vector<TableAccRow> & i_accVec, vector<TblSizeRecv> & io_tblSizeVec
    )
{
    // for each table get accumulators count and size
    int tblId = -1;
    int accCount = 0;

    for (int nAcc = 0; nAcc < (int)i_accVec.size(); nAcc++) {

        if (i_accVec[nAcc].tableId != tblId) {

            // update accumulators count for previous table
            if (!io_tblSizeVec.empty()) io_tblSizeVec.back().accCount = accCount;

            // add current table to the list with initial zero accumulators count
            tblId = i_accVec[nAcc].tableId;
            accCount = 0;
            io_tblSizeVec.push_back(
                TblSizeRecv(tblId, nAcc, accCount, IOutputTableWriter::sizeOf(i_modelId, i_metaStore, tblId))
                );
        }

        accCount++;
    }

    // update last table accumulators count
    if (!io_tblSizeVec.empty()) io_tblSizeVec.back().accCount = accCount;
}
