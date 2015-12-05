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
    IDbExec * i_dbExec,
    IMsgExec * i_msgExec,
    const MetaRunHolder * i_metaStore
    ) : 
    isMpiUsed(i_isMpiUsed),
    modelId(i_modelId),
    runId(i_runId),
    subCount(i_subSampleCount),
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
    int i_modelId,
    int i_runId,
    int i_subCount,
    IDbExec * i_dbExec,
    IMsgExec * i_msgExec, 
    const MetaRunHolder * i_metaStore 
    )
{
    if (i_msgExec == NULL) throw MsgException("invalid (NULL) message passing interface");
    if (i_metaStore == NULL) throw ModelException("invalid (NULL) model metadata");
    if ((!i_isMpiUsed || i_msgExec->isRoot()) && i_dbExec == NULL) 
        throw ModelException("database connection must be open at process %d", i_msgExec->rank());

    // create the run initialzer
    return new RunBase(
        i_isMpiUsed, i_modelId, i_runId, i_subCount, i_dbExec, i_msgExec, i_metaStore
        );
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

/** model run shutdown on error: mark run as failure. */
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

/** model run shutdown: save results and cleanup resources. 
*
* @param[in] i_processSubCount  number of subsamples calculated at current process
*/
void RunBase::shutdown(int i_processSubCount)
{
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
            "UPDATE run_lst SET sub_completed = sub_completed + " + to_string(i_processSubCount) + ", " +
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
