/**
 * @file
 * OpenM++ modeling library: public interface.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_H_MODEL_H
#define OM_H_MODEL_H

#include <memory>
#include <cfloat>
#include <forward_list>
#include <type_traits>
#include <typeinfo>
#include <vector>
using namespace std;

#include "omModelRunState.h"
#include "omLog.h"
#include "omError.h"
#include "omVersion.h"

namespace openm
{
    /** model run options */
    struct RunOptions
    {
        /** number of sub-values */
        int subValueCount;

        /** sub-value index for current modeling thread */
        int subValueId;

        /** if true then use sparse output to database */
        bool useSparse;

        /** if use sparse and abs(value) <= nullValue then value not stored */
        double nullValue;

        /** if positive then used for simulation progress reporting, ex: every 10% */
        int progressPercent;

        /** if positive then used for simulation progress reporting, ex: every 1000 cases or every 0.1 time step */
        double progressStep;

        /** init run options with default values */
        RunOptions(void) :
            subValueCount(1),
            subValueId(0),
            useSparse(false),
            nullValue(FLT_MIN),
            progressPercent(0),
            progressStep(0.0)
        { }
        ~RunOptions(void) noexcept { }
    };

    /** public interface to initialize model run and input parameters */
    struct IRunBase
    {
        virtual ~IRunBase(void) noexcept = 0;

        /** return id of parameter by name */
        virtual int parameterIdByName(const char * i_name) const = 0;

        /** number of parameter sub-values */
        virtual int parameterSubCount(int i_paramId) const = 0;

        /** number of parameter sub-values for current process */
        virtual int parameterSelfSubCount(int i_paramId) const = 0;

        /** return index of parameter sub-value in the storage array of sub-values */
        virtual int parameterSubValueIndex(int i_paramId, int i_subId) const = 0;

        /** return true if sub-value used by current process */
        virtual bool isUseSubValue(int i_subId) const = 0;

        /** read model parameter */
        virtual void readParameter(const char * i_name, int i_subId, const type_info & i_type, size_t i_size, void * io_valueArr) = 0;
    };

    /** model sub-value run public interface */
    struct IModel
    {
        virtual ~IModel(void) noexcept = 0;

        /** number of sub-values */
        virtual int subValueCount(void) const noexcept = 0;

        /** sub-value index of current modeling thread */
        virtual int subValueId(void) const noexcept = 0;

        /** return id of output table by name */
        virtual int tableIdByName(const char * i_name) const = 0;

        /** check by name if output table suppressed. */
        virtual bool isSuppressed(const char * i_name) const = 0;

        /** return model run options */
        virtual const RunOptions * runOptions(void) const = 0;

        /** return index of parameter sub-value in the storage array for current modeling thread */
        virtual int parameterSubValueIndex(const char * i_name) const = 0;

        /** write output result table: sub values */
        virtual void writeOutputTable(const char * i_name, size_t i_size, forward_list<unique_ptr<double> > & io_accValues) = 0;

        /** set modeling progress count and value */
        virtual void updateProgress(int i_count, double i_value = 0.0) = 0;
    };

    /** model input parameter name, type and size */
    struct ParameterNameSizeItem
    {
        /** input parameter name */
        const char * name;

        /** value type */
        const type_info & typeOf;

        /** parameter size (number of parameter values) */
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

    /** read scalar parameter value or all sub-values for the current process */
    template<typename TVal> vector<TVal> read_om_parameter(IRunBase * const i_runBase, const char * i_name)
    {
        int paramId = i_runBase->parameterIdByName(i_name);
        int allCount = i_runBase->parameterSubCount(paramId);       // number of parameter sub-values
        int selfCount = i_runBase->parameterSelfSubCount(paramId);  // number of sub-values for current process

        // storage array: parameter values for current process
        // extra parameter value for exchange between root and child process
        vector<TVal> valueVec(selfCount);
        TVal extraVal;
        bool isStr = typeid(TVal) == typeid(string);    // no default null values for string parameters

        // read sub-values and place into storage array or send to child process
        for (int nSub = 0; nSub < allCount; nSub++) {

            void * pData = &extraVal;
            if (!isStr) {
                extraVal = numeric_limits<TVal>::quiet_NaN();   // set default null value
            }
            i_runBase->readParameter(i_name, nSub, typeid(TVal), 1, pData);

            if (allCount <= 1) {
                valueVec[0] = extraVal;
            }
            else {
                if (i_runBase->isUseSubValue(nSub)) valueVec[i_runBase->parameterSubValueIndex(paramId, nSub)] = extraVal;
            }
        }
        return valueVec;
    }

    /** read array parameter value or all sub-values for the current process */
    template<typename TVal> vector<unique_ptr<TVal[]>> read_om_parameter(IRunBase * const i_runBase, const char * i_name, size_t i_size)
    {
        int paramId = i_runBase->parameterIdByName(i_name);
        int allCount = i_runBase->parameterSubCount(paramId);       // number of parameter sub-values
        int selfCount = i_runBase->parameterSelfSubCount(paramId);  // number of sub-values for current process

        if (i_size <= 0) throw ModelException("invalid size: %zd of parameter %s", i_size, i_name);

        // storage array: parameter values for current process
        vector<unique_ptr<TVal[]>> valueVec(selfCount);
        for (auto & p : valueVec) {
            p.reset(new TVal[i_size]);
        }

        unique_ptr<TVal> extraVal;  // extra parameter value for exchange between root and child process
        if (allCount > 1) {
            extraVal.reset(new TVal[i_size]);
        }
        bool isStr = typeid(TVal) == typeid(string);    // no default null values for string parameters

        // read sub-values and place into storage array or send to child process
        for (int nSub = 0; nSub < allCount; nSub++) {

            void * pData = valueVec[0].get();
            if (allCount > 1) {
                pData =
                    i_runBase->isUseSubValue(nSub) ?
                    valueVec[i_runBase->parameterSubValueIndex(paramId, nSub)].get() :
                    extraVal.get();
            }
            // set default null values and read parameter
            if (!isStr) {
                fill(static_cast<TVal *>(pData), &(static_cast<TVal *>(pData))[i_size], numeric_limits<TVal>::quiet_NaN());
            }
            i_runBase->readParameter(i_name, nSub, typeid(TVal), i_size, pData);
        }
        return valueVec;
    }
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

/** model one-time initialization */
typedef void(*OM_RUN_ONCE_HANDLER)(openm::IRunBase * const i_runBase);
extern OM_RUN_ONCE_HANDLER RunOnceHandler;

/** model run initialization: read input parameters */
typedef void(*OM_RUN_INIT_HANDLER)(openm::IRunBase * const i_runBase);
extern OM_RUN_INIT_HANDLER RunInitHandler;

/** model startup method: initialize sub-value */
typedef void(*OM_STARTUP_HANDLER)(openm::IModel * const i_model);
extern OM_STARTUP_HANDLER ModelStartupHandler;

/** model event loop: user code entry point */
typedef void (*OM_EVENT_LOOP_HANDLER)(openm::IModel * const i_model);
extern OM_EVENT_LOOP_HANDLER RunModelHandler;

/** model shutdown method: save output results */
typedef void (*OM_SHUTDOWN_HANDLER)(openm::IModel * const i_model);
extern OM_SHUTDOWN_HANDLER ModelShutdownHandler;

/** public interface of process-wide model run state: status, progress, update times */
extern openm::IModelRunState * theModelRunState;

#ifdef __cplusplus
    }
#endif

#endif  // OM_H_MODEL_H
