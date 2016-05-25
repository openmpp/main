/**
 * @file
 * OpenM++ modeling library: modeling thread implementation to calculate model subsample
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
#include <forward_list>
using namespace std;

#include "libopenm/omCommon.h"
#include "libopenm/omModel.h"
#include "libopenm/db/dbMetaRow.h"
#include "libopenm/common/argReader.h"
#include "helper.h"
#include "dbExec.h"
#include "dbMetaTable.h"
#include "dbParameter.h"
#include "dbOutputTable.h"
#include "modelRunState.h"
#include "metaRunHolder.h"
#include "msg.h"
#include "metaLoader.h"
#include "runController.h"

namespace openm
{
    /** default error message: "unknown model error" */
    extern const char modelUnknownErrorMessage[];   

    /** modeling library exception */
    typedef OpenmException<4000, modelUnknownErrorMessage> ModelException;

    /** model subsample run base class */
    class ModelBase : public IModel
    {
    public:
        ~ModelBase(void) throw() { }

        /** model factory: create new model subsample run. */
        static ModelBase * create(
            int i_runId,
            int i_subCount,
            int i_subNumber,
            RunController * i_runCtrl,
            const MetaRunHolder * i_metaStore 
            );

        /** number of subsamples */
        int subSampleCount(void) const throw() { return runOptions()->subSampleCount; }
        
        /** subsample number of current modeling process */
        int subSampleNumber(void) const throw() { return runOptions()->subSampleNumber; }

        /** return model run options */
        const RunOptions * runOptions(void) const { return &runOpts; }

        /** update modeling progress */
        int updateProgress(void) { return runState.updateProgress(); }

        /** write result into output table and release accumulators memory. */
        void writeOutputTable(const char * i_name, size_t i_size, forward_list<unique_ptr<double> > & io_accValues);

    private:
        int modelId;                        // model id in database
        int runId;                          // model run id
        RunController * runCtrl;            // run controller interface
        const MetaRunHolder * metaStore;    // metadata tables
        ModelRunState runState;             // model run state
        RunOptions runOpts;                 // model run options

        ModelBase(
            int i_modelId,
            int i_runId,
            int i_subCount,
            int i_subNumber,
            RunController * i_runCtrl,
            const MetaRunHolder * i_metaStore
            );

        // helper struct to store output table save status
        struct TableDoneItem
        {
            int tableId;    // output table id
            bool isDone;    // if true then table is written into db or send to root process

            TableDoneItem(const TableDicRow & i_tableRow) : tableId(i_tableRow.tableId), isDone(false)
            { }
        };

        vector<TableDoneItem> tableDoneVec;     // status for all output tables of subsample

    private:
        ModelBase(const ModelBase & i_model) = delete;
        ModelBase & operator=(const ModelBase & i_model) = delete;
    };
}

#endif  // MODEL_H
