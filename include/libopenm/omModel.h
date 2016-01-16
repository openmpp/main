/**
 * @file
 * OpenM++ modeling library: public interface.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_MODEL_H
#define OM_MODEL_H

#include <typeinfo>
#include <memory>
using namespace std;

#include "omCommon.h"

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

        /** waiting (reserved) */
        wait,

        /** completed */
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
        virtual void readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr) = 0;
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
        virtual void writeOutputTable(const char * i_name, long long i_size, forward_list<unique_ptr<double> > & io_accValues) = 0;
    };
}

// VC++ 2013: define thread_local to comply with c++11 standard
#ifdef _WIN32
    #define thread_local __declspec(thread)
#endif  // _WIN32

//
// modeling library export
//
#ifdef __cplusplus
    extern "C" {
#endif

/** model name */
extern const char * OM_MODEL_NAME;

/** model timestamp: compilation date-time */
extern const char * OM_MODEL_TIMESTAMP;

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

#endif  // OM_MODEL_H
