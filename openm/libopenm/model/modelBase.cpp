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
* write output result table: vector of accumulator values and release accumulators memory.
*
* @param[in]     i_name         output table name
* @param[in]     i_size         number of cells for each accumulator
* @param[in,out] io_accValues   accumulator values
*
* usage example: \n
* @code
*      // allocate model output table accumulators: [N_accumulators][N_cells]
*      double * acc[N_accumulators];
*      forward_list<unique_ptr<double> > acc_storage; 
*
*      auto it = acc_storage.before_begin();
*      for (int k = 0; k < N_accumulators; ++k) {
*        it = acc_storage.insert_after(it, unique_ptr<double>(new double[N_cells]));
*        acc[k] = it->get();
*      }
*      ...
*      // run the model and calculate accumulator values
*      for (int k = 0; k < N_accumulators; ++k) {
*        std::fill(acc[k], &acc[k][N_cells], 0.0);
*      }
*      ...
*      // write output table "salaryBySex" into database
*      i_model->writeOutputTable("salaryBySex", N_cells, acc_storage);
*      // at this point any kind of table->acc[k][j] will cause memory access violation
* @endcode
*/
void ModelBase::writeOutputTable(const char * i_name, long long i_size, forward_list<unique_ptr<double> > & io_accValues)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) output table name");
    if (io_accValues.empty() || i_size <= 0) throw ModelException("invalid (empty) accumulators or size: %lld for output table %s", i_size, i_name);

    try {
        forward_list<unique_ptr<double> > accValLst;
        accValLst.swap(io_accValues);                   // release accumulators memory at return

        // find output table db row
        const TableDicRow * tblRow = metaStore->tableDic->byModelIdName(modelId, i_name);
        if (tblRow == nullptr) throw new DbException("output table not found in table dictionary: %s", i_name);

        int tblId = tblRow->tableId;

        // validate accumulator(s) arrays
        int srcCount = 0;
        for (const auto & apc : accValLst) {
            if (apc.get() == nullptr) throw ModelException("invalid (empty) array of accumulator values for output table %s", i_name);
            srcCount++;
        }

        // validate number of accumulators
        int accCount = (int)metaStore->tableAcc->byModelIdTableId(modelId, tblId).size();
        if (accCount <= 0 || accCount != srcCount) throw DbException("invalid number of accumulators: %d, expected: %d for output table accumulators not found for table: %s", srcCount, accCount, i_name);

        // if no MPI used or at main (root) process then write subsample into db 
        // else start sending subsample data to root process
        if (!isMpiUsed || msgExec->isRoot()) {  

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
            int nAcc = 0;
            for (const auto & apc : accValLst) {
                writer->writeAccumulator(dbExec, subSampleNumber(), nAcc, i_size, apc.get());
                nAcc++;
            }
        }
        else {  // initiate send of subsample data to root process

            // find starting index of table accumulators in db rows of accumulators
            const vector<TableAccRow> accVec = metaStore->tableAcc->byModelId(modelId);

            int accPos = 0;
            while (accPos < (int)accVec.size() && accVec[accPos].tableId != tblId) {
                accPos++;
            }
            if (accPos >= (int)accVec.size()) throw ModelException("accumulators not found for output table %s", i_name);

            // start sending accumulators
            int nAcc = 0;
            for (auto & ap : accValLst) {
                msgExec->startSend(
                    IMsgExec::rootRank,
                    (MsgTag)(((int)MsgTag::outSubsampleBase + accPos + nAcc) * subSampleCount() + subSampleNumber()),
                    typeid(double),
                    i_size,
                    ap.release()
                    );
                nAcc++;
            }
        }
    }
    catch (exception & ex) {
        throw ModelException("Failed to write output table: %s. %s", i_name, ex.what());
    }
}
