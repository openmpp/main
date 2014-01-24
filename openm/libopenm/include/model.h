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
            int i_subCount, 
            int i_subNumber,
            IDbExec * i_dbExec, 
            IMsgExec * i_msgExec, 
            const MetaRunHolder * i_metaStore 
            );

        /** return model type */
        virtual ModelType type(void) const = 0;

        /** number of subsamples */
        int subSampleCount(void) const throw() { return runOptions()->subSampleCount; }
        
        /** subsample number of current modeling process */
        int subSampleNumber(void) const throw() { return runOptions()->subSampleNumber; }

        /** return model run options */
        virtual const RunOptions * runOptions(void) const = 0;

        /** update modeling progress */
        int updateProgress(void) { return ++progressCount; /* prototype only */ }

        /** model shutdown: save results and cleanup resources. */
        void shutdown(void);

        /** read input parameter values. */
        void readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr);

        /** write output result table: array of accumulator values. */
        void writeOutputTable(const char * i_name, int i_accCount, long long i_size, const double * i_valueArr[]);

    protected:
        int modelId;                        // model id in database
        bool isMpiUsed;                     // if true then use MPI messaging library to pass the data
        IDbExec * dbExec;                   // db-connection
        IMsgExec * msgExec;                 // message passing interface
        const MetaRunHolder * metaStore;    // metadata tables
        int progressCount;                  // model progress count

        ModelBase(
            bool i_isMpiUsed, int i_modelId, IDbExec * i_dbExec, IMsgExec * i_msgExec, const MetaRunHolder * i_metaStore
            );

        /** set default values of run options common for all model types. */
        static void defaultBaseOptions(RunOptions & io_options);

        /** get model default run options as list of (key, value) strings. */
        static NoCaseMap defaultBaseOptions(const RunOptions & i_options);

        /** set run id and run options common for all model types. */
        void setBaseOptions(int i_subCount, int i_subNumber, RunOptions & io_options);

    private:
        /** receive all output tables subsamples and write into database */
        void receiveSubSamples(void);

        /** write output tables aggregated values into database */
        void writeOutputValues(void);

    private:
        ModelBase(const ModelBase & i_model);               // = delete;
        ModelBase & operator=(const ModelBase & i_model);   // = delete;
    };

    /** case-based model */
    class CaseModel : public ModelBase, public ICaseModel
    {
    public:
        /** create case-based model. */
        CaseModel(
            bool i_isMpiUsed,
            int i_modelId,
            int i_subCount, 
            int i_subNumber,
            IDbExec * i_dbExec, 
            IMsgExec * i_msgExec, 
            const MetaRunHolder * i_metaStore
            );
        ~CaseModel(void) throw();

        /** return model type: always caseBased. */
        ModelType type(void) const { return ModelType::caseBased; }

        /** return model run options */
        const CaseRunOptions * runOptions() const { return &runOptions_; }

        /** get case-based model default run options as list of (key, value) strings. */
        static NoCaseMap defaultOptions(void);

    private:
        CaseRunOptions runOptions_;     // model run options

        /** set default run options for case-based models. */
        static void defaultOptions(CaseRunOptions & io_options);

        /** set run id and run options for case-based model. */
        void setOptions(int i_subCount, int i_subNumber);

    private:
        CaseModel(const CaseModel & i_model);               // = delete;
        CaseModel & operator=(const CaseModel & i_model);   // = delete;
    };

    /** time-based model */
    class TimeModel : public ModelBase, public ITimeModel
    {
    public:
        /** create time-based model. */
        TimeModel(
            bool i_isMpiUsed,
            int i_modelId,
            int i_subCount, 
            int i_subNumber,
            IDbExec * i_dbExec, 
            IMsgExec * i_msgExec, 
            const MetaRunHolder * i_metaStore
            );
        ~TimeModel(void) throw();

        /** return model type: always timeBased. */
        ModelType type(void) const { return ModelType::timeBased; };

        /** return model run options */
        const TimeRunOptions * runOptions(void) const { return &runOptions_; }

        /** get time-based model default run options as list of (key, value) strings. */
        static NoCaseMap defaultOptions(void);

    private:
        TimeRunOptions runOptions_;     // model run options

        /** set default run options for time-based models. */
        static void defaultOptions(TimeRunOptions & io_options);

        /** set run id and run options for case-based model. */
        void setOptions(int i_subCount, int i_subNumber);

    private:
        TimeModel(const TimeModel & i_model);              // = delete;
        TimeModel & operator=(const TimeModel & i_model);   // = delete;
    };
}

#endif  // MODEL_H
