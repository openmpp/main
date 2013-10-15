/**
 * @file
 * OpenM++ modeling library: public interface.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef OM_MODEL_H
#define OM_MODEL_H

#include <typeinfo>
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

        /** starting seed for random number generator */
        int startingSeed;

        /** if true then use sparse output to database */
        bool useSparse;

        /** if use sparse and abs(value) <= nullValue then value not stored */
        double nullValue;

        /** init run options with default values */
        RunOptions(void) :
            subSampleCount(1),
            subSampleNumber(0),
            startingSeed(0),
            useSparse(false),
            nullValue(DBL_EPSILON)
        { }
        ~RunOptions(void) throw() { }
    };

    /** case-based model run options */
    struct CaseRunOptions : public RunOptions
    {
        /** number of cases */
        long long cases;

        /** init case-based model run options */
        CaseRunOptions(void) : cases(1)
        { }
        ~CaseRunOptions(void) throw() { }
    };

    /** time-based model run options */
    struct TimeRunOptions : public RunOptions
    {
        /** simulation end time */
        long long simulationEnd;

        /** init time-based model run options */
        TimeRunOptions(void) : simulationEnd(1)
        { }
        ~TimeRunOptions(void) throw() { }
    };

    /** model public interface */
    struct IModel
    {
        virtual ~IModel(void) throw() = 0;

        /** model shutdown: save results and cleanup resources. */
        virtual void shutdown(void) = 0;

        /** number of subsamples */
        virtual int subSampleCount(void) const throw() = 0;
        
        /** subsample number of current modeling process */
        virtual int subSampleNumber(void) const throw() = 0;

        /** return model run options */
        virtual const RunOptions * runOptions(void) const = 0;

        /** update modeling progress */
        virtual int updateProgress(void) = 0;

        /** read model parameter */
        virtual void readParameter(const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr) = 0;

        /** write output result table: subsample value */
        virtual void writeOutputTable(const char * i_name, int i_accCount, long long i_size, const double * i_valueArr[]) = 0;
    };

    /** case-based model public interface */
    struct ICaseModel
    {
        virtual ~ICaseModel(void) throw() = 0;

        /** return model run options */
        virtual const CaseRunOptions * runOptions(void) const = 0;
    };

    /** time-based model public interface */
    struct ITimeModel
    {
        virtual ~ITimeModel(void) throw() = 0;

        /** return model run options */
        virtual const TimeRunOptions * runOptions(void) const = 0;
    };
}

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

/** model startup method: initialize parameters */
typedef void (*OM_STARTUP_HANDLER)(openm::IModel * const i_model);
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
