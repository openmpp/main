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
        /** subsample staring number for current modeling process */
        int subFirstNumber;

        /** number of subsamples for current process */
        int selfSubCount;

        /** number of modeling processes: MPI world size */
        int processCount;

        /** create run controller, load metadata tables and broadcast it to all modeling processes. */
        static RunController * create(const ArgReader & i_argOpts, bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec);

        /** last cleanup */
        virtual ~RunController(void) throw() = 0;

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

    protected:
        /** create run controller */
        RunController(const ArgReader & i_argStore) : MetaLoader(i_argStore),
            subFirstNumber(0),
            selfSubCount(0),
            processCount(1)
        { }

        /** get number of subsamples, read and broadcast metadata. */
        virtual void init(void) = 0;

        // input set id, result run id and status
        struct SetRunItem
        {
            int setId;              // set id of input parameters
            int runId;              // if >0 then run id
            ModelRunState state;    // run status

            SetRunItem(void) : setId(0), runId(0) { }
            SetRunItem(int i_setId, int i_runId, ModelStatus i_status) : setId(i_setId), runId(i_runId)
                { state.updateStatus(i_status); }

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

        /** update subsample number to restart the run */
        void updateRestartSubsample(int i_runId, IDbExec * i_dbExec, size_t i_subRestart) const;

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
