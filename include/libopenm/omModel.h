/**
 * @file
 * OpenM++ modeling library: public interface.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_H_MODEL_H
#define OM_H_MODEL_H

#include <typeinfo>
#include <memory>
#include <cfloat>
using namespace std;

#include "omLog.h"
#include "omError.h"

namespace openm
{
    /** model run options */
    struct RunOptions
    {
        /** number of subsamples */
        int subSampleCount;

        /** subsample number for current modeling process */
        int subSampleNumber;

        /** if true then use sparse output to database */
        bool useSparse;

        /** if use sparse and abs(value) <= nullValue then value not stored */
        double nullValue;

        /** init run options with default values */
        RunOptions(void) :
            subSampleCount(1),
            subSampleNumber(0),
            useSparse(false),
            nullValue(DBL_EPSILON)
        { }
        ~RunOptions(void) throw() { }
    };

    /** modeling job status */
    enum class ModelStatus : int
    {
        /** unknown status */
        undefined = 0,

        /** initial status */
        init = 1,

        /** run in progress */
        progress,

        /** run in progress, under external supervision */
        waitProgress,

        /** shutdown model process */
        shutdown,

        /** completed successfully */
        done = 64,

        /** exit and not completed (reserved) */
        exit,

        /** error failure */
        error = 128
    };

    /** model run status codes */
    struct RunStatus
    {
        /** i = initial status */
        static const char * init;

        /** p = run in progress */
        static const char * progress;

        /** w = wait: run in progress, under external supervision */
        static const char * waitProgress;

        /** s = completed successfully */
        static const char * done;

        /** x = exit and not completed (reserved) */
        static const char * exit;

        /** e = error failure */
        static const char * error;
    };

    /** public interface to initialize model run and input parameters */
    struct IRunBase
    {
        virtual ~IRunBase(void) throw() = 0;

        /** read model parameter */
        virtual void readParameter(const char * i_name, const type_info & i_type, size_t i_size, void * io_valueArr) = 0;
    };

    /** model subsample run public interface */
    struct IModel
    {
        virtual ~IModel(void) throw() = 0;

        /** number of subsamples */
        virtual int subSampleCount(void) const throw() = 0;
        
        /** subsample number of current modeling process */
        virtual int subSampleNumber(void) const throw() = 0;

        /** return model run options */
        virtual const RunOptions * runOptions(void) const = 0;

        /** update modeling progress */
        virtual int updateProgress(void) = 0;

        /** write output result table: subsample value */
        virtual void writeOutputTable(const char * i_name, size_t i_size, forward_list<unique_ptr<double> > & io_accValues) = 0;
    };

    /** model input parameter name, type and size */
    struct ParameterNameSizeItem
    {
        /** input parameter name */
        const char * name;

        /** value type */
        const type_info & typeOf;

        /** parameter size(number of parameter values) */
        const size_t size;
    };

    /** size of parameters list: number of model input parameters */
    extern const size_t PARAMETER_NAME_ARR_LEN;

    /** model input parameters name, type and size */
    extern const ParameterNameSizeItem parameterNameSizeArr[];

    /** default error message: "unknown model error" */
    extern const char modelUnknownErrorMessage[];   

    /** modeling library exception */
    typedef OpenmException<4000, modelUnknownErrorMessage> ModelException;

    /** simulation exception default error message: "unknown error in simulation" */
    extern const char simulationUnknownErrorMessage[];   

    /** simulation exception */
    typedef OpenmException<4000, simulationUnknownErrorMessage> SimulationException;
}

//
// modeling library import and export
//
#ifdef __cplusplus
    extern "C" {
#endif

/** model name */
extern const char * OM_MODEL_NAME;

/** model metadata digest: unique model key */
extern const char * OM_MODEL_DIGEST;

/** main entry point */
int main(int argc, char ** argv);

/** model run initialization: read input parameters */
typedef void(*OM_RUN_INIT_HANDLER)(openm::IRunBase * const i_runBase);
extern OM_RUN_INIT_HANDLER RunInitHandler;

/** model startup method: initialize subsample */
typedef void(*OM_STARTUP_HANDLER)(openm::IModel * const i_model);
extern OM_STARTUP_HANDLER ModelStartupHandler;

/** model event loop: user code entry point */
typedef void (*OM_EVENT_LOOP_HANDLER)(openm::IModel * const i_model);
extern OM_EVENT_LOOP_HANDLER RunModelHandler;

/** model shutdown method: save output results */
typedef void (*OM_SHUTDOWN_HANDLER)(openm::IModel * const i_model);
extern OM_SHUTDOWN_HANDLER ModelShutdownHandler;

#ifdef __cplusplus
    }
#endif

#endif  // OM_H_MODEL_H
