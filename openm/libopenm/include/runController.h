/**
* @file
* OpenM++ modeling library: public base class for run controller to run modeling process
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef RUN_CTRL_H
#define RUN_CTRL_H

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

        /** create run controller, load metadata tables and broadcast it to all modeling processes. */
        static RunController * create(const ArgReader & i_argOpts, bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec);

        /** last cleanup */
        virtual ~RunController(void) throw() = 0;

        /** return index of parameter by name */
        int parameterIdByName(const char * i_name) const override;

        /** number of parameter sub-values for current process */
        int parameterSubCount(int i_paramId) const override
        {
            return MetaLoader::parameterSubCount(i_paramId);
        }

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

        /** create new run and input parameters in database. */
        virtual int nextRun(void) = 0;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(int i_runId) = 0;

        /** model process shutdown: cleanup resources. */
        virtual void shutdownWaitAll(void) = 0;

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) = 0;

        /** communicate with child processes to send new input and receive accumulators of output tables. */
        virtual bool childExchange(void) = 0;

        /** write output table accumulators or send data to root process. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLastTable,
            const char * i_name,
            size_t i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) = 0;

        /** add new run state on modeling thread start */
        void addModelRunState(int i_runId, int i_subId) { return runStateStore.add(i_runId, i_subId); }

        /** remove run state on modeling thread exit */
        void removeModelRunState(int i_runId, int i_subId) { return runStateStore.remove(i_runId, i_subId); }

        /** set sub-value modeling progress count */
        bool updateProgress(int i_runId, int i_subId, int i_progress) {
            return runStateStore.updateProgress(i_runId, i_subId, i_progress);
        }

        /** update model status in the list if not already set as one of exit status values, if found then return true and actual status */
        ModelStatus updateStatus(int i_runId, int i_subId, ModelStatus i_status) {
            return runStateStore.updateStatus(i_runId, i_subId, i_status);
        }

    protected:
        /** run states for all modeling threads */
        RunStateHolder runStateStore;

        /** create run controller */
        RunController(const ArgReader & i_argStore) : MetaLoader(i_argStore),
            subFirstId(0),
            selfSubCount(0),
            processCount(1)
        { }

        /** get number of sub-values, read and broadcast metadata. */
        virtual void init(void) = 0;

        // input set id, result run id and status
        struct SetRunItem
        {
            int setId;              // set id of input parameters
            int runId;              // if >0 then run id
            ModelStatus status;     // run status

            SetRunItem(void) : setId(0), runId(0) { }
            SetRunItem(int i_setId, int i_runId, ModelStatus i_status) : setId(i_setId), runId(i_runId), status(i_status) { }

            // return true if set or run undefined
            bool isEmpty(void) { return setId <= 0 || runId <= 0; }
        };

        /** create new run, create input parameters and run options for input working sets */
        SetRunItem createNewRun(int i_taskRunId, bool i_isWaitTaskRun, IDbExec * i_dbExec) const;

        /** impelementation of model process shutdown if exiting without completion. */
        void doShutdownOnExit(ModelStatus i_status, int i_runId, int i_taskRunId, IDbExec * i_dbExec);

        /** implementation of model run shutdown. */
        void doShutdownRun(int i_runId, int i_taskRunId, IDbExec * i_dbExec);

        /** implementation model process shutdown. */
        void doShutdownAll(int i_taskRunId, IDbExec * i_dbExec);

        /** write output table accumulators. */
        void doWriteAccumulators(
            int i_runId,
            IDbExec * i_dbExec,
            const RunOptions & i_runOpts,
            const char * i_name,
            size_t i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) const;

        /** update sub-value index to restart the run */
        void updateRestartSubValueId(int i_runId, IDbExec * i_dbExec, size_t i_subRestart) const;

    private:
        // create run options in run_option table
        void createRunOptions(int i_runId, int i_setId, IDbExec * i_dbExec) const;

        // copy input parameters from "base" run and working set into new run id
        void createRunParameters(int i_runId, int i_setId, IDbExec * i_dbExec) const;

        /** write output tables aggregated values into database */
        void writeOutputValues(int i_runId, IDbExec * i_dbExec) const;

    private:
        RunController(const RunController & i_runCtrl) = delete;
        RunController & operator=(const RunController & i_runCtrl) = delete;
    };
}

#endif  // RUN_CTRL_H
