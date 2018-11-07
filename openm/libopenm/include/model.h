/**
 * @file
 * OpenM++ modeling library: modeling thread implementation to calculate model sub-value
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MODEL_H
#define MODEL_H

#include <memory>
#include <stdexcept>
#include <mutex>
#include <future>
#include <list>
using namespace std;

#include "libopenm/omLog.h"
#include "libopenm/omModel.h"
#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
#include "modelRunState.h"
#include "metaHolder.h"
#include "runController.h"

namespace openm
{
    /** model sub-value run base class */
    class ModelBase : public IModel
    {
    public:
        ~ModelBase(void) throw() { }

        /** model factory: create new model sub-value run. */
        static ModelBase * create(
            int i_runId,
            int i_subCount,
            int i_subId,
            RunController * i_runCtrl,
            const MetaHolder * i_metaStore 
            );

        /** number of sub-values */
        int subValueCount(void) const throw() override { return runOptions()->subValueCount; }
        
        /** sub-value index of current modeling process */
        int subValueId(void) const throw() override { return runOptions()->subValueId; }

        /** return index of parameter sub-value in the array of sub-values, used to find thread local parameter values */
        int parameterSubValueIndex(const char * i_name) const override;

        /** return model run options */
        const RunOptions * runOptions(void) const override { return &runOpts; }

        /** write result into output table and release accumulators memory. */
        void writeOutputTable(const char * i_name, size_t i_size, forward_list<unique_ptr<double> > & io_accValues) override;

        /** set modeling progress count and value */
        void updateProgress(int i_count, double i_value = 0.0) override { runCtrl->runStateStore().updateProgress(runId, runOpts.subValueId, i_count, i_value); }

    private:
        int modelId;                        // model id in database
        int runId;                          // model run id
        RunController * runCtrl;            // run controller interface
        const MetaHolder * metaStore;       // metadata tables
        RunOptions runOpts;                 // model run options

        ModelBase(
            int i_modelId,
            int i_runId,
            int i_subCount,
            int i_subId,
            RunController * i_runCtrl,
            const MetaHolder * i_metaStore
            );

        // helper struct to store output table save status
        struct TableDoneItem
        {
            int tableId;    // output table id
            bool isDone;    // if true then table is written into db or send to root process

            TableDoneItem(const TableDicRow & i_tableRow) : tableId(i_tableRow.tableId), isDone(false)
            { }
        };

        vector<TableDoneItem> tableDoneVec;     // status for all output tables of sub-value

    private:
        ModelBase(const ModelBase & i_model) = delete;
        ModelBase & operator=(const ModelBase & i_model) = delete;
    };
}

#endif  // MODEL_H
