/**
 * @file
 * OpenM++ modeling library: implementation.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef MODEL_H
#define MODEL_H

#include <memory>
#include <stdexcept>
#include <mutex>
#include <future>
using namespace std;

#include "libopenm/omCommon.h"
#include "libopenm/omModel.h"
#include "libopenm/db/dbMetaRow.h"
#include "dbExec.h"
#include "dbMetaTable.h"
#include "dbParameter.h"
#include "dbOutputTable.h"
#include "metaRunHolder.h"
#include "msg.h"
#include "runController.h"

namespace openm
{
    /** default error message: "unknown model error" */
    extern const char modelUnknownErrorMessage[];   

    /** modeling library exception */
    typedef OpenmException<4000, modelUnknownErrorMessage> ModelException;

    /** model base class */
    class ModelBase : public IModel
    {
    public:
        ~ModelBase(void) throw();

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

        /** model shutdown: save results and cleanup resources. */
        static void shutdown(
            bool i_isMpiUsed,
            int i_runId,
            int i_subCount,
            IDbExec * i_dbExec,
            IMsgExec * i_msgExec,
            const MetaRunHolder * i_metaStore
            );

        /** model shutdown on error: mark run as failure. */
        static void shutdownOnFail(
            bool i_isMpiUsed, 
            int i_runId, 
            IDbExec * i_dbExec, 
            IMsgExec * i_msgExec
            );

        /** number of subsamples */
        int subSampleCount(void) const throw() { return runOptions()->subSampleCount; }
        
        /** subsample number of current modeling process */
        int subSampleNumber(void) const throw() { return runOptions()->subSampleNumber; }

        /** return model run options */
        const RunOptions * runOptions(void) const { return &runOpts; }

        /** update modeling progress */
        int updateProgress(void) { return ++progressCount; /* prototype only */ }

        /** read input parameter values. */
        void readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr);

        /** write output result table: array of accumulator values. */
        void writeOutputTable(const char * i_name, int i_accCount, long long i_size, const double * i_valueArr[]);

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
        /** write output tables aggregated values into database */
        static void writeOutputValues(
            int i_modelId, 
            int i_runId, 
            int i_subCount, 
            IDbExec * i_dbExec, 
            const MetaRunHolder * i_metaStore
            );

        /** receive all output tables subsamples and write into database */
        static void receiveSubSamples(
            int i_modelId,
            int i_runId,
            int i_subCount,
            IDbExec * i_dbExec,
            IMsgExec * i_msgExec,
            const MetaRunHolder * i_metaStore
            );

    private:
        ModelBase(const ModelBase & i_model);               // = delete;
        ModelBase & operator=(const ModelBase & i_model);   // = delete;
    };
}

#endif  // MODEL_H
