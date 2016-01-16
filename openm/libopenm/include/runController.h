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

        /** number of subsamples for each child modeling process */
        int subPerProcess;

        /** number of subsamples for current process */
        int selfSubCount;

        /** number of modeling processes */
        int processCount;

        /** create run controller, load metadata tables and broadcast it to all modeling processes. */
        static RunController * create(const ArgReader & i_argOpts, bool i_isMpiUsed, IDbExec * i_dbExec, IMsgExec * i_msgExec);

        /** last cleanup */
        virtual ~RunController(void) throw() = 0;

        /** create new run and input parameters in database. */
        virtual int nextRun(void) = 0;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(void) = 0;

        /** model process shutdown: cleanup resources. */
        virtual void shutdown(void) = 0;

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) = 0;

        /** receive accumulators of output tables subsamples and write into database. */
        virtual bool receiveSubSamples(void) = 0;

        /** write output table accumulators or send data to root process. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLast,
            const char * i_name,
            long long i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) = 0;

    protected:
        static const int processInitMsgSize = 4;    // initial message size at process startup

        /** create run controller */
        RunController(int i_processCount, const ArgReader & i_argStore) : MetaLoader(i_argStore),
            subFirstNumber(0),
            subPerProcess(1),
            selfSubCount(0),
            processCount(i_processCount)
        { }

        /** get number of subsamples, read and broadcast metadata. */
        virtual void init(void) = 0;

        // task item: workset, result run and status
        struct TaskItem
        {
            int setId;              // set id of input parameters
            int runId;              // if >0 then run id
            ModelRunState state;    // run status

            TaskItem(int i_setId) : setId(i_setId), runId(0) { }

        private:
            TaskItem(void) = delete;
        };

        /** read modeling task definition */
        list<RunController::TaskItem> readTask(int i_taskId, const ModelDicRow * i_modelRow, IDbExec * i_dbExec);

        /** find working set to run the model */
        int nextSetId(int i_taskId, IDbExec * i_dbExec, const list<RunController::TaskItem> & i_taskRunLst);

        /** create new run, create input parameters and run options for input working sets */
        int createNewRun(
            int i_setId, int i_taskId, int i_taskRunId, IDbExec * i_dbExec, list<RunController::TaskItem> & io_taskRunLst
            );

        /** impelementation of model process shutdown if exiting without completion. */
        void doShutdownOnExit(ModelStatus i_status, int i_runId, int i_taskRunId, IDbExec * i_dbExec);

        /** implementation of model run shutdown. */
        void doShutdownRun(int i_runId, int i_taskRunId, IDbExec * i_dbExec);

        /** implementation model process shutdown. */
        void doShutdown(int i_taskRunId, IDbExec * i_dbExec);

        /** receive outstanding accumulators and wait until outstanding send completed. */
        virtual void receiveSendLast(void) = 0;

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
            );

        /** update subsample nubre to restart the run */
        void updateRestartSubsample(int i_runId, IDbExec * i_dbExec, const vector<bool> & i_subDoneVec);

    private:
        // create run options in run_option table
        void createRunOptions(int i_runId, IDbExec * i_dbExec);

        // copy input parameters from "base" run and working set into new run id
        void createRunParameters(int i_runId, int i_setId, IDbExec * i_dbExec, const ModelDicRow * i_modelRow);

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
        SingleController(const ArgReader & i_argStore, IDbExec * i_dbExec) : RunController(1, i_argStore),
            runId(0),
            setId(0),
            taskId(0),
            taskRunId(0),
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
            bool i_isLast,
            const char * i_name,
            long long i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(void) override { doShutdownRun(runId, taskRunId, dbExec); }

        /** model process shutdown: cleanup resources. */
        virtual void shutdown(void) override { doShutdown(taskRunId, dbExec); }

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) override { doShutdownOnExit(i_status, runId, taskRunId, dbExec); }

        /** receive accumulators of output tables subsamples and write into database. */
        virtual bool receiveSubSamples(void) override { return false; }

    protected:
        /** initialize root modeling process. */
        virtual void init(void) override;

        /** receive outstanding accumulators and wait until outstanding send completed. */
        virtual void receiveSendLast(void) override { }

    private:
        int runId;                  // if > 0 then model run id
        int setId;                  // if > 0 then set id of source input parametes
        int taskId;                 // if > 0 then modeling task id
        int taskRunId;              // if > 0 then modeling task run id
        IDbExec * dbExec;           // db-connection
        vector<bool> subDoneVec;    // if true then all subsample accumulators save in database
        list<TaskItem> taskRunLst;  // pairs of (set, run) for modeling task

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
            RunController(i_processCount, i_argStore),
            runId(0),
            setId(0),
            taskId(0),
            taskRunId(0),
            dbExec(i_dbExec),
            msgExec(i_msgExec)
        { }

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
            bool i_isLast,
            const char * i_name,
            long long i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(void) override { doShutdownRun(runId, taskRunId, dbExec); }

        /** model process shutdown: cleanup resources. */
        virtual void shutdown(void) override { doShutdown(taskRunId, dbExec); }

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) override { doShutdownOnExit(i_status, runId, taskRunId, dbExec); }

        /** receive accumulators of output tables subsamples and write into database. */
        virtual bool receiveSubSamples(void) override;

    protected:
        /** initialize root modeling process. */
        virtual void init(void) override;

        /** receive outstanding accumulators and wait until outstanding send completed. */
        virtual void receiveSendLast(void) override;

    private:
        int runId;                  // if > 0 then model run id
        int setId;                  // if > 0 then set id of source input parametes
        int taskId;                 // if > 0 then modeling task id
        int taskRunId;              // if > 0 then modeling task run id
        IDbExec * dbExec;           // db-connection
        IMsgExec * msgExec;         // message passing interface
        vector<bool> subDoneVec;    // if true then all subsample accumulators save in database
        list<TaskItem> taskRunLst;  // pairs of (set, run) for modeling task

        // helper struct to receive output table values for each accumulator
        struct AccReceiveItem
        {
            int subNumber;          // subsample number to receive
            int tableId;            // output table id
            int accId;              // accumulator id
            long long valueSize;    // size of accumulator data
            bool isReceived;        // if true then data received
            int senderRank;         // sender rank: process where accumulator calculated
            int msgTag;             // accumulator message tag

            AccReceiveItem(
                int i_subNumber,
                int i_subSampleCount,
                int i_subPerProcess,
                int i_tableId,
                int i_accId,
                int i_accIndex,
                long long i_valueSize
                ) :
                subNumber(i_subNumber),
                tableId(i_tableId),
                accId(i_accId),
                valueSize(i_valueSize),
                isReceived(false),
                senderRank(accFromRank(i_subNumber, i_subPerProcess)),
                msgTag(accMsgTag(i_subNumber, i_subSampleCount, i_accIndex))
            { }

            // return accumulator sender rank
            static int accFromRank(int i_subNumber, int i_subPerProcess)
            {
                return 1 + (i_subNumber / i_subPerProcess);
            }
        };

        vector<AccReceiveItem> accRecvVec;  // list of accumulators to be received

        // send initial state and main control values to all child processes: subsample count, thread count
        void sendInitialState(void);

        // create list of accumulators to be received from child modeling processes
        void initAccReceiveList(void);

        /** receive outstanding accumulators. */
        void receiveLast(void);

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
            RunController(i_processCount, i_argStore),
            runId(0),
            msgExec(i_msgExec)
        { }

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
            bool i_isLast,
            const char * i_name,
            long long i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        void shutdownRun(void) override { receiveSendLast(); }

        /** model process shutdown: cleanup resources. */
        void shutdown(void) override { }

        /** model process shutdown if exiting without completion (ie: exit on error). */
        void shutdownOnExit(ModelStatus /*i_status*/) override { }

        /** receive accumulators of output tables subsamples and write into database. */
        virtual bool receiveSubSamples(void) override { return false; }

    private:
        int runId;                  // if > 0 then model run id
        IMsgExec * msgExec;         // message passing interface

        /** initialize child modeling process. */
        virtual void init(void) override;

        /** wait until outstanding send completed. */
        virtual void receiveSendLast(void) override;

        // receive initial state and main control values from root process: subsample count, thread count
        ModelStatus receiveInitialState(void);

    private:
        ChildController(const ChildController & i_runCtrl) = delete;
        ChildController & operator=(const ChildController & i_runCtrl) = delete;
    };

    /** controller for "restart run": calculate outstanding subsamples for existing run */
    class RestartController : public RunController
    {
    public:
        /** create new "restart run" controller */
        RestartController(const ArgReader & i_argStore, IDbExec * i_dbExec) : RunController(1, i_argStore),
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
            bool i_isLast,
            const char * i_name,
            long long i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        void shutdownRun(void) override { doShutdownRun(runId, 0, dbExec); }

        /** model process shutdown: cleanup resources. */
        void shutdown(void) override { doShutdown(0, dbExec); }

        /** model process shutdown if exiting without completion (ie: exit on error). */
        void shutdownOnExit(ModelStatus i_status) override { doShutdownOnExit(i_status, runId, 0, dbExec); }

        /** receive accumulators of output tables subsamples and write into database. */
        virtual bool receiveSubSamples(void) override { return false; }

    private:
        int runId;                  // if > 0 then model run id
        int setId;                  // if > 0 then set id of source input parametes
        IDbExec * dbExec;           // db-connection
        vector<bool> subDoneVec;    // if true then all subsample accumulators save in database

        /** initialize "restart run" modeling process. */
        virtual void init(void) override;

        // find subsample to restart the run and update run status
        bool cleanupRestartSubsample(void);

        /** receive outstanding accumulators and wait until outstanding send completed. */
        virtual void receiveSendLast(void) override { }

    private:
        RestartController(const RestartController & i_runCtrl) = delete;
        RestartController & operator=(const RestartController & i_runCtrl) = delete;
    };
}
#endif  // RUN_CTRL_H
