/**
 * @file
 * OpenM++ modeling library: modeling thread implementation to calculate model subsample
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
using namespace openm;

/** model exception default error message */
const char openm::modelUnknownErrorMessage[] = "unknown model error";

/** simulation exception default error message */
const char openm::simulationUnknownErrorMessage[] = "unknown error in simulation";

/** model subsample run public interface */
IModel::~IModel() throw() { }

/** create new model subsample run base class */
ModelBase::ModelBase(
    int i_modelId,
    int i_runId,
    int i_subCount,
    int i_subNumber,
    RunController * i_runCtrl,
    const MetaRunHolder * i_metaStore
    ) : 
    modelId(i_modelId),
    runId(i_runId),
    runCtrl(i_runCtrl),
    metaStore(i_metaStore)
{
    // set model run options
    runOpts = i_runCtrl->modelRunOptions(i_subCount, i_subNumber);

    // setup accumulators list of id's
    const vector<TableDicRow> tblVec = metaStore->tableDic->byModelId(modelId);

    for (const TableDicRow & tblRow : tblVec) {
        tableDoneVec.push_back(TableDoneItem(tblRow));
    }
}

/** create new model subsample run */
ModelBase * ModelBase::create(
    int i_runId,
    int i_subCount,
    int i_subNumber,
    RunController * i_runCtrl,
    const MetaRunHolder * i_metaStore 
    )
{
    if (i_metaStore == NULL) throw ModelException("invalid (NULL) model metadata");
    if (i_runCtrl == NULL) throw ModelException("invalid (NULL) run controller interface");

    // create the model subsample run
    return new ModelBase(i_metaStore->modelRow->modelId, i_runId, i_subCount, i_subNumber, i_runCtrl, i_metaStore);
}

/**
* write result into output table and release accumulators memory.
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
void ModelBase::writeOutputTable(const char * i_name, size_t i_size, forward_list<unique_ptr<double> > & io_accValues)
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

        int accCount = (int)metaStore->tableAcc->countOf(
            [&](const TableAccRow & i_row) -> bool { return i_row.modelId == modelId && i_row.tableId == tblRow->tableId; }
        );

        if (srcCount <= 0 || accCount != srcCount) throw DbException("invalid number of accumulators: %d for output table : %s", srcCount, i_name);

        // update table write status and check if all tables completed
        bool isLastTable = true;
        for (auto & td : tableDoneVec) {
            if (td.tableId == tblId) td.isDone = true;
            if (!td.isDone) isLastTable = false;
        }

        // write subsample into database or start sending data to root process
        runCtrl->writeAccumulators(runOpts, isLastTable, i_name, i_size, accValLst);
    }
    catch (exception & ex) {
        throw ModelException("Failed to write output table: %s. %s", i_name, ex.what());
    }
}
