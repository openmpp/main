// OpenM++ modeling library: base model class
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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
    // set model run options
    runOpts.subSampleCount = i_subCount;
    runOpts.subSampleNumber = i_subNumber;
    runOpts.useSparse = metaStore->runOption->boolValue(RunOptionsKey::useSparse);
    runOpts.nullValue = metaStore->runOption->doubleValue(RunOptionsKey::sparseNull, DBL_EPSILON);
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
