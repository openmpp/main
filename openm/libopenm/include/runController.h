/**
* @file
* OpenM++ modeling library: controllers for modeling process run
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef RUN_CTRL_H
#define RUN_CTRL_H

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
            long long i_size,
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

        /** return accumulator message tag */
        static int accMsgTag(int i_subNumber, int i_subSampleCount, int i_accIndex)
        {
            return ((int)MsgTag::outSubsampleBase + i_accIndex) * i_subSampleCount + i_subNumber;
        }

        /** write output table accumulators. */
        void doWriteAccumulators(
            int i_runId,
            IDbExec * i_dbExec,
            const RunOptions & i_runOpts,
            const char * i_name,
            long long i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) const;

        /** update subsample number to restart the run */
        void updateRestartSubsample(int i_runId, IDbExec * i_dbExec, const vector<bool> & i_isSubDone);

    private:
        // create run options in run_option table
        void createRunOptions(int i_runId, int i_setId, IDbExec * i_dbExec) const;

        // copy input parameters from "base" run and working set into new run id
        void createRunParameters(int i_runId, int i_setId, IDbExec * i_dbExec, const ModelDicRow * i_modelRow) const;

        /** write output tables aggregated values into database */
        void writeOutputValues(int i_runId, IDbExec * i_dbExec) const;

    private:
        RunController(const RunController & i_runCtrl) = delete;
        RunController & operator=(const RunController & i_runCtrl) = delete;
    };

    /** controller for single process: create new model run(s), read input parameters and write output tables. */
    class SingleController : public RunController
    {
    public:
        /** create new single process run controller */
        SingleController(const ArgReader & i_argStore, IDbExec * i_dbExec) : RunController(i_argStore),
            taskId(0),
            taskRunId(0),
            isWaitTaskRun(false),
            dbExec(i_dbExec)
        { }

        /** last cleanup */
        virtual ~SingleController(void) throw() { }

        /** create new run and input parameters in database. */
        virtual int nextRun(void) override;

        /** read input parameter values. */
        virtual void readParameter(
            const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr
            ) override;

        /** write output table accumulators. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLastTable,
            const char * i_name,
            long long i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(int i_runId) override { doShutdownRun(i_runId, taskRunId, dbExec); }

        /** model process shutdown: cleanup resources. */
        virtual void shutdownWaitAll(void) override { doShutdownAll(taskRunId, dbExec); }

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) override 
            { doShutdownOnExit(i_status, nowSetRun.runId, taskRunId, dbExec); }

        /** communicate with child processes to send new input and receive accumulators of output tables. */
        virtual bool childExchange(void) override { return false; }

    protected:
        /** initialize root modeling process. */
        virtual void init(void) override;

    private:
        int taskId;                 // if > 0 then modeling task id
        int taskRunId;              // if > 0 then modeling task run id
        bool isWaitTaskRun;         // if true then task run under external supervision
        SetRunItem nowSetRun;       // current set id, run id and status
        IDbExec * dbExec;           // db-connection
        vector<bool> isSubDone;     // if true then all subsample accumulators saved in database

    private:
        SingleController(const SingleController & i_runCtrl) = delete;
        SingleController & operator=(const SingleController & i_runCtrl) = delete;
    };

    /** controller for root process: create new model run(s), send input parameters to children and receieve output tables. */
    class RootController : public RunController
    {
    public:
        /** create new root run controller */
        RootController(int i_processCount, const ArgReader & i_argStore, IDbExec * i_dbExec, IMsgExec * i_msgExec) :
            RunController(i_argStore),
            taskId(0),
            taskRunId(0),
            isWaitTaskRun(false),
            dbExec(i_dbExec),
            msgExec(i_msgExec)
        {
            processCount = i_processCount; 
        }

        /** last cleanup */
        virtual ~RootController(void) throw() { }

        /** create new run and input parameters in database. */
        virtual int nextRun(void) override;

        /** read input parameter values. */
        virtual void readParameter(
            const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr
            ) override;

        /** write output table accumulators. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLastTable,
            const char * i_name,
            long long i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(int i_runId) override;

        /** model process shutdown: wait for all child to be completed and do final cleanup. */
        virtual void shutdownWaitAll(void) override;

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) override 
            { doShutdownOnExit(i_status, rootRunGroup().runId, taskRunId, dbExec); }

        /** communicate with child processes to send new input and receive accumulators of output tables. */
        virtual bool childExchange(void) override;

    protected:
        /** initialize root modeling process. */
        virtual void init(void) override;

    private:
        int taskId;                     // if > 0 then modeling task id
        int taskRunId;                  // if > 0 then modeling task run id
        bool isWaitTaskRun;             // if true then task run under external supervision
        IDbExec * dbExec;               // db-connection
        IMsgExec * msgExec;             // message passing interface
        ProcessGroupDef rootGroupDef;   // root process groups size, groups count and process rank in group
        vector<RunGroup> runGroupVec;   // process groups run id and run state
        
        // helper struct to receive output table values for each accumulator
        struct AccReceiveItem
        {
            int runId;              // run id to receive
            int subNumber;          // subsample number to receive
            int tableId;            // output table id
            int accId;              // accumulator id
            long long valueCount;   // size of accumulator data
            bool isReceived;        // if true then data received
            int senderRank;         // sender rank: process where accumulator calculated
            int msgTag;             // accumulator message tag

            AccReceiveItem(
                int i_runId,
                int i_subNumber,
                int i_subSampleCount,
                int i_senderRank,
                int i_tableId,
                int i_accId,
                int i_accIndex,
                long long i_valueCount
                ) :
                runId(i_runId),
                subNumber(i_subNumber),
                tableId(i_tableId),
                accId(i_accId),
                valueCount(i_valueCount),
                isReceived(false),
                senderRank(i_senderRank),
                msgTag(accMsgTag(i_subNumber, i_subSampleCount, i_accIndex))
            { }
        };

        list<AccReceiveItem> accRecvLst;  // list of accumulators to be received

        // return root process run group: last run group
        RunGroup & rootRunGroup(void) { return runGroupVec.back(); }

        /** create new run and assign it to modeling group. */
        int makeNextRun(RunGroup & i_runGroup);

        /** receive accumulators of output tables subsamples and write into database. */
        void appendAccReceiveList(int i_runId, const RunGroup & i_runGroup);

        /** read all input parameters by run id and broadcast to child processes. */
        void readAllRunParameters(const RunGroup & i_runGroup) const;

        /** receive accumulators of output tables subsamples and write into database. */
        bool receiveSubSamples(void);

        /** update restart subsample in database and list of accumulators to be received. */
        void updateAccReceiveList(void);

    private:
        RootController(const RootController & i_runCtrl) = delete;
        RootController & operator=(const RootController & i_runCtrl) = delete;
    };

    /** controller for child process: receive input parameters from root and send output tables. */
    class ChildController : public RunController
    {
    public:
        /** create new child run controller */
        ChildController(int i_processCount, const ArgReader & i_argStore, IMsgExec * i_msgExec) :
            RunController(i_argStore),
            runId(0),
            msgExec(i_msgExec)
        {
            processCount = i_processCount; 
        }

        /** last cleanup */
        virtual ~ChildController(void) throw() { }

        /** create new run and input parameters in database. */
        virtual int nextRun(void) override;

        /** read input parameter values. */
        virtual void readParameter(
            const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr
            ) override;

        /** send output table accumulators to root process. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLastTable,
            const char * i_name,
            long long i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(int i_runId) override;

        /** model process shutdown: cleanup resources. */
        virtual void shutdownWaitAll(void) override { theModelRunState.updateStatus(ModelStatus::done); }

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) override { theModelRunState.updateStatus(i_status); }

        /** communicate with child processes to send new input and receive accumulators of output tables. */
        virtual bool childExchange(void) override { return false; }

    private:
        int runId;                  // if > 0 then model run id
        ProcessGroupDef groupDef;   // child process groups size, groups count and process rank in group
        IMsgExec * msgExec;         // message passing interface

        /** initialize child modeling process. */
        virtual void init(void) override;

    private:
        ChildController(const ChildController & i_runCtrl) = delete;
        ChildController & operator=(const ChildController & i_runCtrl) = delete;
    };

    /** controller for "restart run": calculate outstanding subsamples for existing run */
    class RestartController : public RunController
    {
    public:
        /** create new "restart run" controller */
        RestartController(const ArgReader & i_argStore, IDbExec * i_dbExec) : RunController(i_argStore),
            runId(0),
            setId(0),
            dbExec(i_dbExec)
        { }

        /** last cleanup */
        virtual ~RestartController(void) throw() { }

        /** create new run and input parameters in database. */
        virtual int nextRun(void) override;

        /** read input parameter values. */
        virtual void readParameter(
            const char * i_name, const type_info & i_type, long long i_size, void * io_valueArr
            ) override;

        /** write output table accumulators. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLastTable,
            const char * i_name,
            long long i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(int i_runId) override { doShutdownRun(i_runId, 0, dbExec); }

        /** model process shutdown: cleanup resources. */
        virtual void shutdownWaitAll(void) override { doShutdownAll(0, dbExec); }

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) override { doShutdownOnExit(i_status, runId, 0, dbExec); }

        /** communicate with child processes to send new input and receive accumulators of output tables. */
        virtual bool childExchange(void) override { return false; }

    private:
        int runId;                  // if > 0 then model run id
        int setId;                  // if > 0 then set id of source input parametes
        IDbExec * dbExec;           // db-connection
        vector<bool> isSubDone;     // if true then all subsample accumulators saved in database

        /** initialize "restart run" modeling process. */
        virtual void init(void) override;

        // find subsample to restart the run and update run status
        bool cleanupRestartSubsample(void);

    private:
        RestartController(const RestartController & i_runCtrl) = delete;
        RestartController & operator=(const RestartController & i_runCtrl) = delete;
    };
}
#endif  // RUN_CTRL_H
