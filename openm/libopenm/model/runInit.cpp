// OpenM++ modeling library: model run initialzer
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "model.h"
using namespace openm;

// run initialization public interface
IRunInit::~IRunInit() throw() { }

// model run initialzer
RunInitBase::RunInitBase(
    bool i_isMpiUsed,
    int i_modelId,
    int i_runId,
    int i_subCount,
    IDbExec * i_dbExec,
    IMsgExec * i_msgExec,
    const MetaRunHolder * i_metaStore
    ) : 
    isMpiUsed(i_isMpiUsed),
    modelId(i_modelId),
    runId(i_runId),
    subCount(i_subCount),
    dbExec(i_dbExec),
    msgExec(i_msgExec),
    metaStore(i_metaStore)
{
    if (i_msgExec == NULL) throw ModelException("invalid (NULL) message passing interface");
    if (i_metaStore == NULL) throw ModelException("invalid (NULL) model metadata");

    if ((!i_isMpiUsed || i_msgExec->isRoot()) && i_dbExec == NULL) 
        throw ModelException("database connection must be open at process %d", i_msgExec->rank());

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

// create new model run initialzer
RunInitBase * RunInitBase::create(
    bool i_isMpiUsed, 
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

    // find model in metadata tables
    const ModelDicRow * mdRow = i_metaStore->modelDic->byNameTimeStamp(OM_MODEL_NAME, OM_MODEL_TIMESTAMP);
    if (mdRow == NULL) throw ModelException("model not found in the database");

    // create the run initialzer
    return new RunInitBase(i_isMpiUsed, mdRow->modelId, i_runId, i_subCount, i_dbExec, i_msgExec, i_metaStore);
}

/**
* read input parameter values.
*
* @param[in]     i_name      parameter name
* @param[in]     i_type      parameter type
* @param[in]     i_size      parameter size (number of parameter values)
* @param[in,out] io_valueArr array to return parameter values, size must be =i_size
*/
void RunInitBase::readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr)
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
