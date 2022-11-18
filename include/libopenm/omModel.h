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

#include "omModelRunState.h"
#include "omLog.h"
#include "omError.h"

namespace openm
{
    /** basic model run options */
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

        /** if true then model run is storing microdata in database. */
        bool isDbMicrodata;

        /** if true then model run is writing microdata into CSV. */
        bool isCsvMicrodata;

        /** if true then model run is writing microdata into trace output. */
        bool isTraceMicrodata;

        /** if true then model run is writing microdata events. */
        bool isMicrodataEvents;

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
            isDbMicrodata(false),
            isCsvMicrodata(false),
            isTraceMicrodata(false),
            isMicrodataEvents(false),
            progressPercent(0),
            progressStep(0.0)
        { }
        ~RunOptions(void) noexcept { }
    };

    /** public interface to get model run options */
    struct IRunOptions
    {
        virtual ~IRunOptions(void) noexcept = 0;

        /** return true if run option found by i_key in run_option table for the current run id. */
        virtual bool isOptionExist(const char * i_key) const noexcept = 0;

        /** return string value of run option by i_key or default value if not found. */
        virtual std::string strOption(const char * i_key, const std::string & i_default = "") const noexcept = 0;

        /** return boolean value of run option by i_key or false if not found or value is not "yes", "1", "true" or empty "" string. */
        virtual bool boolOption(const char * i_key) const noexcept = 0;

        /** search for boolean value of run option by i_key and return one of:              \n
        * return  1 if key found and value is one of: "yes", "1", "true" or empty value,    \n
        * return  0 if key found and value is one of: "no", "0", "false",                   \n
        * return -1 if key not found,                                                       \n
        * return -2 otherwise.
        */
        virtual int boolOptionToInt(const char * i_key) const noexcept = 0;

        /** return int value of run option by i_key or default if not found or can not be converted to int. */
        virtual int intOption(const char * i_key, int i_default) const noexcept = 0;

        /** return long value of run option by i_key or default if not found or can not be converted to long. */
        virtual long long longOption(const char * i_key, long long i_default) const noexcept = 0;

        /** return double value of run option by i_key or default if not found or can not be converted to double. */
        virtual double doubleOption(const char * i_key, double i_default) const noexcept = 0;

        /** return a copy of all run options as [key, value] pairs, ordered by key. */
        virtual std::vector<std::pair<std::string, std::string>> allOptions(void) const noexcept = 0;
    };

    /** public interface to initialize model run and input parameters */
    struct IRunBase : public IRunOptions
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
        virtual void readParameter(const char * i_name, int i_subId, const std::type_info & i_type, size_t i_size, void * io_valueArr) = 0;
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

        /** return basic model run options */
        virtual const RunOptions * runOptions(void) const = 0;

        /** return index of parameter sub-value in the storage array for current modeling thread */
        virtual int parameterSubValueIndex(const char * i_name) const = 0;

        /** write output result table: sub values */
        virtual void writeOutputTable(const char * i_name, size_t i_size, std::forward_list<std::unique_ptr<double[]> > & io_accValues) = 0;

        /** set modeling progress count and value */
        virtual void updateProgress(int i_count, double i_value = 0.0) = 0;

        /** return true if model store microdata in database or CSV file. */
        virtual const bool isMicrodata(void) const = 0;

        /** write microdata into the database and/or CSV file.
        *
        * @param   i_entityKind     entity kind id: model metadata entity id in database.
        * @param   i_microdataKey   unique entity instance id.
        * @param   i_eventId        event id, if microdata events enabled.
        * @param   i_isSameEntity   if true then event entity the same as microdata entity.
        * @param   i_entityThis     entity class instance this pointer.
        */
        virtual void writeMicrodata(int i_entityKind, uint64_t i_microdataKey, int i_eventId, bool i_isSameEntity, const void * i_entityThis) = 0;
    };

    /** model input parameter name, type and size */
    struct ParameterNameSizeItem
    {
        /** input parameter name */
        const char * name;

        /** value type */
        const std::type_info & typeOf;

        /** parameter size (number of parameter values) */
        const size_t size;
    };

    /** size of parameters list: number of model input parameters */
    extern const size_t PARAMETER_NAME_ARR_LEN;

    /** model input parameters name, type and size */
    extern const ParameterNameSizeItem parameterNameSizeArr[];

    /** model entity attributes name, type, size and member offset */
    struct EntityNameSizeItem
    {
        /** entity metadata id in database */
        int entityId;

        /** entity name */
        const char * entity;

        /** attribute metadata id in database */
        int attributeId;

        /** attribute name */
        const char * attribute;

        /** attribute value type */
        const std::type_info & typeOf;

        /** attribute value bytes size */
        const size_t size;

        /** attribute value offset in entity this */
        const ptrdiff_t offset;

        /** find index in i_entityArr by entity name and attribute name, return -1 if names not found */
        static int byName(const char * i_entityName, const char * i_attrName);
    };

    /** size of entity attributes list: all attributes of all entities */
    extern const size_t ENTITY_NAME_SIZE_ARR_LEN;

    /** list of entity attributes name, type, size and member offset */
    extern const EntityNameSizeItem EntityNameSizeArr[];

    /** model events id, name */
    struct EventIdNameItem
    {
        /** event id */
        int eventId;

        /** event name */
        const char * eventName;

        /** return event name by event id or NULL if id not found */
        static const char * byId(int i_eventId);
    };

    /** size of event list: all events in all entities */
    extern const size_t EVENT_ID_NAME_ARR_LEN;

    /** list of events id, name */
    extern const EventIdNameItem EventIdNameArr[];

    /** if true then write microdata events into database and/or CSV */
    #define OM_USE_MICRODATA_EVENTS (EVENT_ID_NAME_ARR_LEN > 1)

    /** default error message: "unknown model error" */
    extern const char modelUnknownErrorMessage[];

    /** modeling library exception */
    typedef OpenmException<4000, modelUnknownErrorMessage> ModelException;

    /** simulation exception default error message: "unknown error in simulation" */
    extern const char simulationUnknownErrorMessage[];

    /** simulation exception */
    typedef OpenmException<4000, simulationUnknownErrorMessage> SimulationException;

    /** read scalar parameter value or all sub-values for the current process */
    template<typename TVal> std::vector<TVal> read_om_parameter(IRunBase * const i_runBase, const char * i_name)
    {
        int paramId = i_runBase->parameterIdByName(i_name);
        int allCount = i_runBase->parameterSubCount(paramId);       // number of parameter sub-values
        int selfCount = i_runBase->parameterSelfSubCount(paramId);  // number of sub-values for current process

        // storage array: parameter values for current process
        // extra parameter value for exchange between root and child process
        std::vector<TVal> valueVec(selfCount);
        TVal extraVal;
        bool isStr = typeid(TVal) == typeid(std::string);    // no default null values for string parameters

        // read sub-values and place into storage array or send to child process
        for (int nSub = 0; nSub < allCount; nSub++) {

            void * pData = &extraVal;
            if (!isStr) {
                extraVal = std::numeric_limits<TVal>::quiet_NaN();   // set default null value
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
    template<typename TVal> std::vector<std::unique_ptr<TVal[]>> read_om_parameter(IRunBase * const i_runBase, const char * i_name, size_t i_size)
    {
        int paramId = i_runBase->parameterIdByName(i_name);
        int allCount = i_runBase->parameterSubCount(paramId);       // number of parameter sub-values
        int selfCount = i_runBase->parameterSelfSubCount(paramId);  // number of sub-values for current process

        if (i_size <= 0) throw ModelException("invalid size: %zd of parameter %s", i_size, i_name);

        // storage array: parameter values for current process
        std::vector<std::unique_ptr<TVal[]>> valueVec(selfCount);
        for (auto & p : valueVec) {
            p.reset(new TVal[i_size]);
        }

        std::unique_ptr<TVal> extraVal;  // extra parameter value for exchange between root and child process
        if (allCount > 1) {
            extraVal.reset(new TVal[i_size]);
        }
        bool isStr = typeid(TVal) == typeid(std::string);    // no default null values for string parameters

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
                std::fill(static_cast<TVal *>(pData), &(static_cast<TVal *>(pData))[i_size], std::numeric_limits<TVal>::quiet_NaN());
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
