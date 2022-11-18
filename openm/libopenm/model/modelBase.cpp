/**
 * @file
 * OpenM++ modeling library: modeling thread implementation to calculate model sub-value
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
#include "dbValue.h"

using namespace openm;

/** model exception default error message */
const char openm::modelUnknownErrorMessage[] = "unknown model error";

/** simulation exception default error message */
const char openm::simulationUnknownErrorMessage[] = "unknown error in simulation";

/** model sub-value run public interface */
IModel::~IModel() noexcept { }

/** create new model sub-value run base class */
ModelBase::ModelBase(
    int i_modelId,
    int i_runId,
    int i_subCount,
    int i_subId,
    RunController * i_runCtrl,
    const MetaHolder * i_metaStore
    ) : 
    modelId(i_modelId),
    runId(i_runId),
    runCtrl(i_runCtrl),
    metaStore(i_metaStore)
{
    // set model run options
    runOpts = i_runCtrl->modelRunOptions(i_subCount, i_subId);

    // setup accumulators list of id's
    const vector<TableDicRow> tblVec = metaStore->tableDic->byModelId(modelId);

    for (const TableDicRow & tblRow : tblVec) {
        if (!runCtrl->isSuppressed(tblRow.tableId)) tableDoneVec.push_back(TableDoneItem(tblRow));
    }

    // trace microdata writing enabled then write csv header for each entity and allocate csv buffer
    if (runOpts.isTraceMicrodata)
    {
        for (const auto & eRow : metaStore->entityDic->rows())
        { 
            microdataBuf = runCtrl->csvHeaderMicrodata(eRow.entityId);
            if (!microdataBuf.empty()) theTrace->logMsg(microdataBuf.c_str());
        }
        microdataBuf.reserve(OM_STR_DB_MAX);
    }
}

/** create new model sub-value run */
ModelBase * ModelBase::create(
    int i_runId,
    int i_subCount,
    int i_subId,
    RunController * i_runCtrl,
    const MetaHolder * i_metaStore 
    )
{
    if (i_metaStore == NULL) throw ModelException("invalid (NULL) model metadata");
    if (i_runCtrl == NULL) throw ModelException("invalid (NULL) run controller interface");

    // create the model sub-value run
    return new ModelBase(i_metaStore->modelRow->modelId, i_runId, i_subCount, i_subId, i_runCtrl, i_metaStore);
}

/** return index of parameter sub-value in the storage array for current modeling thread */
int ModelBase::parameterSubValueIndex(const char * i_name) const 
{
    const ParamDicRow * paramRow = metaStore->paramDic->byModelIdName(modelId, i_name);
    if (paramRow == nullptr) throw DbException("parameter not found in parameters dictionary: %s", (i_name != nullptr ? i_name : ""));

    if (runOptions()->subValueId < runCtrl->subFirstId || runOptions()->subValueId >= runCtrl->subFirstId + runCtrl->selfSubCount)
        throw DbException("parameter: %s sub-value index: %d out of range", (i_name != nullptr ? i_name : ""), runOptions()->subValueId);

    return runCtrl->parameterSubValueIndex(paramRow->paramId, runOptions()->subValueId);
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
void ModelBase::writeOutputTable(const char * i_name, size_t i_size, forward_list<unique_ptr<double[]> > & io_accValues)
{
    if (i_name == NULL || i_name[0] == '\0') throw ModelException("invalid (empty) output table name");
    if (io_accValues.empty() || i_size <= 0) throw ModelException("invalid (empty) accumulators or size: %zd for output table %s", i_size, i_name);

    try {
        forward_list<unique_ptr<double[]> > accValLst;
        accValLst.swap(io_accValues);                   // release accumulators memory at return

        // find output table db row
        const TableDicRow * tblRow = metaStore->tableDic->byModelIdName(modelId, i_name);
        if (tblRow == nullptr) throw DbException("output table not found in tables dictionary: %s", i_name);

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

        // write sub-value into database or start sending data to root process
        runCtrl->writeAccumulators(runOpts, isLastTable, i_name, i_size, accValLst);
    }
    catch (exception & ex) {
        throw ModelException("Failed to write output table: %s. %s", i_name, ex.what());
    }
}

/** write microdata into the database and/or CSV file.
*
* @param   i_entityKind     entity kind id: model metadata entity id in database.
* @param   i_microdataKey   unique entity instance id.
* @param   i_eventId        event id, if microdata events enabled.
* @param   i_isSameEntity   if true then event entity the same as microdata entity.
* @param   i_entityThis     entity class instance this pointer.
*/
void ModelBase::writeMicrodata(int i_entityKind, uint64_t i_microdataKey, int i_eventId, bool i_isSameEntity, const void * i_entityThis)
{
    if (!isMicrodata()) return;     // microdata writing is not enabled

    if (i_entityThis == nullptr) throw ModelException("invalid (NULL) entity this pointer, entity kind: %d microdata key: %llu", i_entityKind, i_microdataKey);

    try {
        auto [isFound, entItem] = runCtrl->writeMicrodata(i_entityKind, i_microdataKey, i_eventId, i_isSameEntity, i_entityThis, microdataBuf);

        if (isFound && runOpts.isTraceMicrodata) {
            runCtrl->makeCsvLineMicrodata(entItem, i_microdataKey, i_eventId, i_isSameEntity, i_entityThis, microdataBuf);
            theTrace->logMsg(microdataBuf.c_str());
        }
    }
    catch (exception & ex) {
        throw ModelException("Failed to write microdata entity kind: %d microdata key: %llu. %s", i_entityKind, i_microdataKey, ex.what());
    }
}
