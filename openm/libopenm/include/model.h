/**
 * @file
 * OpenM++ modeling library: implementation.
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
#include "metaRunHolder.h"
#include "msg.h"
#include "modelRunState.h"
#include "metaLoader.h"
#include "runController.h"

namespace openm
{
    /** default error message: "unknown model error" */
    extern const char modelUnknownErrorMessage[];   

    /** modeling library exception */
    typedef OpenmException<4000, modelUnknownErrorMessage> ModelException;

    /** model run basic support: initialze, read input parameters and write output values */
    class RunBase : public IRunBase
    {
    public:
        ~RunBase(void) throw() { }

        /** model run basis factory. */
        static RunBase * create(
            bool i_isMpiUsed,
            int i_modelId,
            int i_runId,
            int i_subSampleCount,
            IDbExec * i_dbExec,
            IMsgExec * i_msgExec,
            const MetaRunHolder * i_metaStore
            );

        /** read input parameter values. */
        void readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr);
        
        /** model shutdown: save results and cleanup resources. */
        void shutdown(int i_processSubCount);

        /** model run shutdown if exiting without completion (ie: exit on error). */
        void shutdownOnExit(ModelStatus i_status);

    private:
        bool isMpiUsed;                     // if true then use MPI messaging library to pass the data
        int modelId;                        // model id in database
        int runId;                          // model run id
        int subCount;                       // number of subsamples
        IDbExec * dbExec;                   // db-connection
        IMsgExec * msgExec;                 // message passing interface
        const MetaRunHolder * metaStore;    // metadata tables

        RunBase(
            bool i_isMpiUsed,
            int i_modelId,
            int i_runId,
            int i_subCount,
            IDbExec * i_dbExec,
            IMsgExec * i_msgExec,
            const MetaRunHolder * i_metaStore
            );

        /** write output tables aggregated values into database */
        void writeOutputValues(void);

    private:
        RunBase(const RunBase & i_runBase) = delete;
        RunBase & operator=(const RunBase & i_runBase) = delete;
    };

    /** model base class */
    class ModelBase : public IModel
    {
    public:
        ~ModelBase(void) throw() { }

        /** model factory: create new model. */
        static ModelBase * create(
            bool i_isMpiUsed,
            int i_runId,
            int i_subCount,
            int i_subNumber,
            IDbExec * i_dbExec,
            IMsgExec * i_msgExec, 
            const MetaRunHolder * i_metaStore 
            );

        /** number of subsamples */
        int subSampleCount(void) const throw() { return runOptions()->subSampleCount; }
        
        /** subsample number of current modeling process */
        int subSampleNumber(void) const throw() { return runOptions()->subSampleNumber; }

        /** return model run options */
        const RunOptions * runOptions(void) const { return &runOpts; }

        /** update modeling progress */
        int updateProgress(void) { return ++progressCount; /* prototype only */ }

        /** write output result table and release accumulators memory. */
        void writeOutputTable(const char * i_name, long long i_size, forward_list<unique_ptr<double> > & io_accValues);

    private:
        bool isMpiUsed;                     // if true then use MPI messaging library to pass the data
        int modelId;                        // model id in database
        int runId;                          // model run id
        IDbExec * dbExec;                   // db-connection
        IMsgExec * msgExec;                 // message passing interface
        const MetaRunHolder * metaStore;    // metadata tables
        RunOptions runOpts;                 // model run options
        int progressCount;                  // model progress count

        ModelBase(
            bool i_isMpiUsed,
            int i_modelId,
            int i_runId,
            int i_subCount,
            int i_subNumber,
            IDbExec * i_dbExec,
            IMsgExec * i_msgExec,
            const MetaRunHolder * i_metaStore
            );
    private:
        ModelBase(const ModelBase & i_model) = delete;
        ModelBase & operator=(const ModelBase & i_model) = delete;
    };
}

#endif  // MODEL_H
