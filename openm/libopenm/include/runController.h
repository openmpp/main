/**
* @file
* OpenM++ modeling library: public base class for run controller to run modeling process
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef RUN_CTRL_H
#define RUN_CTRL_H

#include <fstream>
#include "metaLoader.h"

using namespace std;

namespace openm
{
    /** run controller: create new model run(s) and support data exchange. */
    class RunController : public MetaLoader, public IRunBase
    {
    public:
        /** sub-value staring index for current modeling process */
        int subFirstId;

        /** number of sub-values for current process */
        int selfSubCount;

        /** number of modeling processes: MPI world size */
        int processCount;

        /** modeling processes rank: MPI rank */
        int processRank;

        /** create run controller, load metadata tables and broadcast it to all modeling processes. */
        static RunController * create(const ArgReader & i_argOpts, bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec);

        /** last cleanup */
        virtual ~RunController(void) noexcept = 0;

        /** return index of parameter by name */
        int parameterIdByName(const char * i_name) const override { return MetaLoader::parameterIdByName(i_name); };

        /** number of parameter sub-values for current model run */
        int parameterSubCount(int i_paramId) const override { return MetaLoader::parameterSubCount(i_paramId); }

        /** number of parameter sub-values for current process */
        int parameterSelfSubCount(int i_paramId) const override
        {
            return parameterSubCount(i_paramId) > 1 ? selfSubCount : 1;
        }
                
        /** return index of parameter sub-value in the storage array of sub-values */
        int parameterSubValueIndex(int i_paramId, int i_subId) const override
        {
            return (parameterSubCount(i_paramId) > 1 && subFirstId <= i_subId && i_subId < subFirstId + selfSubCount) ? 
                i_subId - subFirstId : 
                0;
        }

        /** return true if sub-value used by current process */
        bool isUseSubValue(int i_subId) const override
        {
            return subFirstId <= i_subId && i_subId < subFirstId + selfSubCount;
        }

        /** return holder of all sub-values modeling run states */
        RunStateHolder & runStateStore(void) { return runStateHolder; }

        /** create new run and input parameters in database. */
        virtual int nextRun(void) = 0;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(int i_runId) = 0;

        /** model process shutdown: cleanup resources. */
        virtual void shutdownWaitAll(void) = 0;

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) = 0;

        /** exchange between root and child processes and threads. send new input, receive accumulators of output tables, send and receive status update. */
        virtual bool childExchange(void) = 0;

        /** write output table accumulators or send data to root process. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLastTable,
            const char * i_name,
            size_t i_size,
            forward_list<unique_ptr<double[]> > & io_accValues
            ) = 0;

        /** return true if run option found by i_key in run_option table for the current run id. */
        bool isOptionExist(const char * i_key) const noexcept override {
            return metaStore->runOptionTable ? metaStore->runOptionTable->isExist(currentRunId(), i_key) : false;
        }

        /** return string value of run option by i_key or default value if not found. */
        string strOption(const char * i_key, const string & i_default = "") const noexcept override {
            return metaStore->runOptionTable ? metaStore->runOptionTable->strValue(currentRunId(), i_key, i_default) : i_default;
        }

        /** return boolean value of run option by i_key or false if not found or value is not "yes", "1", "true" or empty "" string. */
        bool boolOption(const char * i_key) const noexcept override {
            return metaStore->runOptionTable ? metaStore->runOptionTable->boolValue(currentRunId(), i_key) : false;
        }

        /** search for boolean value of run option by i_key and return one of:              \n
        * return  1 if key found and value is one of: "yes", "1", "true" or empty value,    \n
        * return  0 if key found and value is one of: "no", "0", "false",                   \n
        * return -1 if key not found,                                                       \n
        * return -2 otherwise.
        */
        int boolOptionToInt(const char * i_key) const noexcept override {
            return metaStore->runOptionTable ? metaStore->runOptionTable->boolValueToInt(currentRunId(), i_key) : -1;
        }

        /** return int value of run option by i_key or default if not found or can not be converted to int. */
        int intOption(const char * i_key, int i_default) const noexcept override {
            return metaStore->runOptionTable ? metaStore->runOptionTable->intValue(currentRunId(), i_key, i_default) : i_default;
        }

        /** return long value of run option by i_key or default if not found or can not be converted to long. */
        long long longOption(const char * i_key, long long i_default) const noexcept override {
            return metaStore->runOptionTable ? metaStore->runOptionTable->longValue(currentRunId(), i_key, i_default) : i_default;
        }

        /** return double value of run option by i_key or default if not found or can not be converted to double. */
        double doubleOption(const char * i_key, double i_default) const noexcept override {
            return metaStore->runOptionTable ? metaStore->runOptionTable->doubleValue(currentRunId(), i_key, i_default) : i_default;
        }

        /** return a copy of all run options as [key, value] pairs, ordered by key. */
        vector<pair<string, string>> allOptions(void) const noexcept override;

        /** entity attribute item to write microdata into database or csv file */
        struct EntityAttrItem
        {
            int attrId;                             /** entity attribute metadata id */
            int idxOf;                              /** attribute index in EntityNameSizeArr */
            unique_ptr<IValueFormatter> fmtValue;   /** attribute value to string converter for csv output */

            EntityAttrItem(int i_attrId, int i_index) : attrId(i_attrId), idxOf(i_index) {}
        };

        /** entity item to write microdata into database or csv file */
        struct EntityItem
        {
            int entityId;                   /** entity metadata id */
            vector<EntityAttrItem> attrs;   /** entity attributes */
            string csvHdr;                  /** microdata csv header line */

            EntityItem(int i_entityId) : entityId(i_entityId) {}
        };

        /** write microdata into the database and/or CSV file.
        *
        * @param   i_entityKind     entity kind id: model metadata entity id in database.
        * @param   i_microdataKey   unique entity instance id.
        * @param   i_entityThis     entity class instance this pointer.
        */
        tuple<bool, const EntityItem &> writeMicrodata(int i_entityKind, uint64_t i_microdataKey, const void * i_entityThis);

        /** return microdata entity csv file header */
        const string getHeaderCsvMicrodata(int i_entityKind) const;

        /** make attributes csv line by converting attribute values into string */
        static void makeCsvLineMicrodata(const EntityItem & i_entityItem, uint64_t i_microdataKey, const void * i_entityThis, string & io_line);

    protected:
        /** create run controller */
        RunController(const ArgReader & i_argStore) : MetaLoader(i_argStore),
            subFirstId(0),
            selfSubCount(0),
            processCount(1),
            processRank(0)
        { }

        /** get number of sub-values, read and broadcast metadata. */
        virtual void init(void) = 0;

        /** get run id of the current model run. */
        virtual int currentRunId(void) const noexcept = 0;

        /** create task run entry in database */
        int createTaskRun(int i_taskId, IDbExec * i_dbExec);

        /** find modeling task, if specified */
        int findTask(IDbExec * i_dbExec);

        /** create new run, create input parameters and run options for input working sets */
        tuple<int, int, ModelStatus> createNewRun(int i_taskRunId, bool i_isWaitTaskRun, IDbExec * i_dbExec);

        /** impelementation of model process shutdown if exiting without completion. */
        void doShutdownOnExit(ModelStatus i_status, int i_runId, int i_taskRunId, IDbExec * i_dbExec);

        /** implementation of model run shutdown. */
        void doShutdownRun(int i_runId, int i_taskRunId, IDbExec * i_dbExec);

        /** implementation model process shutdown. */
        void doShutdownAll(int i_taskRunId, IDbExec * i_dbExec);

        /** write output table accumulators if table is not suppressed. */
        void doWriteAccumulators(
            int i_runId,
            IDbExec * i_dbExec,
            const RunOptions & i_runOpts,
            const char * i_name,
            size_t i_size,
            forward_list<unique_ptr<double[]> > & io_accValues
            ) const;

        /** update sub-value index to restart the run */
        void updateRestartSubValueId(int i_runId, IDbExec * i_dbExec, int i_subRestart) const;

        /** merge updated sub-values run statue into database */
        void updateRunState(IDbExec * i_dbExec, const map<pair<int, int>, RunState> i_updated) const;

        /** create microdata CSV files for new model run. */
        void openCsvMicrodata(int i_runId);

    private:
        /** sub-value run states for all modeling threads */
        RunStateHolder runStateHolder;

        /** find source working set for input parameters */
        tuple<int, bool, bool> findWorkset(int i_setId, IDbExec * i_dbExec);

        /** find base run to get model parameters, if base run option(s) specified */
        int findBaseRun(IDbExec * i_dbExec);

        /** save run options by inserting into run_option table */
        void createRunOptions(int i_runId, IDbExec * i_dbExec) const;

        // copy input parameters from "base" run and working set into new run id
        void createRunParameters(int i_runId, int i_setId, bool i_isWsDefault, bool i_isReadonlyWsDefault, int i_baseRunId, IDbExec * i_dbExec) const;

        // cretate run description and notes using run options or by copy it from workset text
        void createRunText(int i_runId, int i_setId, IDbExec * i_dbExec) const;

        // insert run entity metadata and create run entity database tables
        void createRunEntity(int i_runId, IDbExec* i_dbExec);

        // calculate entity generation digest: based on entity digest, attributes id, name, type digest
        const string makeEntityGenDigest(const EntityDicRow * i_entRow, const vector<EntityAttrRow> i_attrRows) const;

        /** write output tables aggregated values into database, skip suppressed tables */
        void writeOutputValues(int i_runId, IDbExec * i_dbExec) const;

        // check sub-value id's and count: sub id must be zero in range [0, sub count -1] and count must be equal to parameter size
        void checkParamSubCounts(int i_runId, int i_subCount, const ParamDicRow & i_paramRow, IDbExec * i_dbExec) const;

        // make part of where clause to select sub_id's
        const string makeWhereSubId(const MetaLoader::ParamSubOpts & i_subOpts, int i_defaultSubId) const;

        // make part of select columns list to map source sub id's to run parameter sub_id's
        const string mapSelectedSubId(const MetaLoader::ParamSubOpts & i_subOpts) const;

        // return parameter sub-value options by parameter id
        const MetaLoader::ParamSubOpts subOptsById(const ParamDicRow & i_paramRow, int i_subCount, int i_defaultSubId) const;

        /** find import run of upstream model and verify it is completed successfully */
        RunImportOpts findImportRun(const string & i_modelName, const ImportOpts & i_importOpts, const IModelDicTable * i_mainModelTable, IDbExec * i_dbExec) const;

        /** import parameter value from output table expression of upstream model, return true if imported or false if not found */
        bool importOutputTable(
            int i_runId,
            const ParamDicRow & i_paramRow,
            const ParameterNameSizeItem * i_nameSizeItem,
            const RunImportOpts & i_riOpts,
            const ParamImportRow * i_importRow,
            IDbExec * i_dbExec
        ) const;

        /** import parameter from parameter of upstream model, return true if imported or false if not found */
        bool importParameter(
            int i_runId,
            const ParamDicRow & i_paramRow,
            const ParameterNameSizeItem * i_nameSizeItem,
            const RunImportOpts & i_riOpts,
            const ParamImportRow * i_importRow,
            const ParamSubOpts & i_subOpts,
            IDbExec * i_dbExec
        ) const;


        map<int, EntityItem> entityMap;    // microdata entities to write into database or csv

        struct EntityCsvItem
        {
            int entityId;               // entity metadata id
            string filePath;            // if not empty then microdata csv file path
            //
            recursive_mutex theMutex;   // mutex to lock for csv write operations
            bool isReady;               // if true then csv output file open and ready to use
            string csvBuf;              // microdata csv write buffer
            ofstream csvSt;             // csv output stream
        };
        map<int, EntityCsvItem> entityCsvMap;   // microdata entities to write into database or csv

        // initialize microdata entity writing
        void initMicrodata(void);

        /** close microdata CSV files after model run completed. */
        void closeCsvMicrodata(void) noexcept;

        /** write microdata into CSV files. */
        void doCsvMicrodata(const EntityItem & i_entityItem, uint64_t i_microdataKey, const void * i_entityThis);

        /** write line into microdata CSV file, close file and raise exception on error. */
        void doCsvLineMicrodata(EntityCsvItem & io_entityCsvItem, const string & i_line);

    private:
        RunController(const RunController & i_runCtrl) = delete;
        RunController & operator=(const RunController & i_runCtrl) = delete;
    };
}

#endif  // RUN_CTRL_H
