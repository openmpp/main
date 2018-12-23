/**
* @file
* OpenM++ modeling library: impelementation of controllers for modeling process run
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef RUN_CTRL_IMPL_H
#define RUN_CTRL_IMPL_H

#include <iterator>
#include <unordered_set>
#include "dbParameter.h"
#include "dbOutputTable.h"

using namespace std;

namespace openm
{
    /** controller for single process: create new model run(s), read input parameters and write output tables. */
    class SingleController : public RunController
    {
    public:
        /** create new single process run controller */
        SingleController(const ArgReader & i_argStore, IDbExec * i_dbExec) : 
            RunController(i_argStore),
            runId(0),
            taskId(0),
            taskRunId(0),
            isWaitTaskRun(false),
            dbExec(i_dbExec),
            isSubDone(subValueCount)
        { }

        /** last cleanup */
        virtual ~SingleController(void) throw() { }

        /** create new run and input parameters in database. */
        virtual int nextRun(void) override;

        /** read input parameter values. */
        virtual void readParameter(
            const char * i_name, int i_subId, const type_info & i_type, size_t i_size, void * io_valueArr
            ) override;

        /** write output table accumulators. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLastTable,
            const char * i_name,
            size_t i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(int i_runId) override { doShutdownRun(i_runId, taskRunId, dbExec); }

        /** model process shutdown: cleanup resources. */
        virtual void shutdownWaitAll(void) override { doShutdownAll(taskRunId, dbExec); }

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) override { doShutdownOnExit(i_status, runId, taskRunId, dbExec); }

        /** communicate with child threads to receive status update. */
        virtual bool childExchange(void) override;

    protected:
        /** initialize root modeling process. */
        virtual void init(void) override;

    private:
        int runId;              // if > 0 then model run id
        int taskId;             // if > 0 then modeling task id
        int taskRunId;          // if > 0 then modeling task run id
        bool isWaitTaskRun;     // if true then task run under external supervision
        IDbExec * dbExec;       // db-connection
        DoneVector isSubDone;   // size of [sub-value count], if true then all sub-value accumulators saved in database

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
            const char * i_name, int i_subId, const type_info & i_type, size_t i_size, void * io_valueArr
            ) override;

        /** write output table accumulators. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLastTable,
            const char * i_name,
            size_t i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(int i_runId) override;

        /** model process shutdown: wait for all child to be completed and do final cleanup. */
        virtual void shutdownWaitAll(void) override;

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) override;

        /** exchange between root and child processes and threads.
        *   start new run, send new input, receive accumulators of output tables, send and receive status update.
        *   return true if any: data received, run completed, run started, status update received.
        */
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
        list<RunGroup> runGroupLst;     // process groups run id and run state
        list<AccReceive> accRecvLst;    // list of accumulators to be received

        // return root process run group: last run group
        RunGroup & rootRunGroup(void) { return runGroupLst.back(); }

        /** broadcast metadata tables from root to all modeling processes. */
        void broadcastMetaData(void);

        /** broadcast meta table db rows from root to all modeling processes. */
        template <class MetaTbl>
        void broadcastMetaTable(MsgTag i_msgTag, unique_ptr<MetaTbl> & io_tableUptr);

        /** broadcast run options from root to group of modeling processes. */
        void broadcastRunOptions(void);

        /** broadcast model messages from root to all child processes. */
        void broadcastLanguageMessages(void);

        /** create new run and assign it to modeling group. */
        int makeNextRun(RunGroup & i_runGroup);

        /** read all input parameters by run id and broadcast to child processes. */
        void readAllRunParameters(const RunGroup & i_runGroup) const;

        /** append to list of accumulators to be received from child modeling processes. */
        void appendAccReceiveList(int i_runId, const RunGroup & i_runGroup);

        /** receive accumulators of output tables sub-values and write into database. */
        bool receiveSubValues(void);

        /** update restart sub-value in database and list of accumulators to be received. */
        void updateAccReceiveList(void);

        /** update process status if all run groups completed: done, exit or error.
        *   return true if model process status is not error
        */
        bool updateModelRunStatus(void);

        /** receive status update from all child processes. */
        bool receiveStatusUpdate(long i_waitTime = 0L);

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
            msgExec(i_msgExec),
            lastTimeStatus(chrono::system_clock::now()),
            lastModelStatus(ModelStatus::init)
        {
            processCount = i_processCount; 
        }

        /** last cleanup */
        virtual ~ChildController(void) throw() { }

        /** create new run and input parameters in database. */
        virtual int nextRun(void) override;

        /** read input parameter values. */
        virtual void readParameter(
            const char * i_name, int i_subId, const type_info & i_type, size_t i_size, void * io_valueArr
            ) override;

        /** send output table accumulators to root process. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLastTable,
            const char * i_name,
            size_t i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(int i_runId) override;

        /** model process shutdown: cleanup resources. */
        virtual void shutdownWaitAll(void) override;

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) override;

        /** exchange between root and child process to send and receive status update. */
        virtual bool childExchange(void) override;

    private:
        int runId;                                          // if > 0 then model run id
        ProcessGroupDef groupDef;                           // child process groups size, groups count and process rank in group
        IMsgExec * msgExec;                                 // message passing interface
        chrono::system_clock::time_point lastTimeStatus;    // last status update time sent to root
        ModelStatus lastModelStatus;                        // last model status sent to root

        /** initialize child modeling process. */
        virtual void init(void) override;

        /** receive broadcasted metadata tables from root process. */
        int broadcastMetaData(void);

        /** receive broadcasted meta table db rows from root process. */
        template <class MetaTbl>
        void broadcastMetaTable(MsgTag i_msgTag, unique_ptr<MetaTbl> & io_tableUptr);

        /** receive broadcasted run options from root process. */
        void broadcastRunOptions(void);

        /** receive broadcasted model messages from root process. */
        void broadcastLanguageMessages(void);

        /** send sub-values run status update to root */
        void sendStatusUpdate(void);

    private:
        ChildController(const ChildController & i_runCtrl) = delete;
        ChildController & operator=(const ChildController & i_runCtrl) = delete;
    };

    /** controller for "restart run": calculate outstanding sub-values for existing run */
    class RestartController : public RunController
    {
    public:
        /** create new "restart run" controller */
        RestartController(const ArgReader & i_argStore, IDbExec * i_dbExec) : RunController(i_argStore),
            runId(0),
            setId(0),
            dbExec(i_dbExec),
            isSubDone(subValueCount)
        { }

        /** last cleanup */
        virtual ~RestartController(void) throw() { }

        /** create new run and input parameters in database. */
        virtual int nextRun(void) override;

        /** read input parameter values. */
        virtual void readParameter(
            const char * i_name, int i_subId, const type_info & i_type, size_t i_size, void * io_valueArr
            ) override;

        /** write output table accumulators. */
        virtual void writeAccumulators(
            const RunOptions & i_runOpts,
            bool i_isLastTable,
            const char * i_name,
            size_t i_size,
            forward_list<unique_ptr<double> > & io_accValues
            ) override;

        /** model run shutdown: save results and update run status. */
        virtual void shutdownRun(int i_runId) override { doShutdownRun(i_runId, 0, dbExec); }

        /** model process shutdown: cleanup resources. */
        virtual void shutdownWaitAll(void) override { doShutdownAll(0, dbExec); }

        /** model process shutdown if exiting without completion (ie: exit on error). */
        virtual void shutdownOnExit(ModelStatus i_status) override { doShutdownOnExit(i_status, runId, 0, dbExec); }

        /** communicate with child threads to receive status update. */
        virtual bool childExchange(void) override;

    private:
        int runId;              // if > 0 then model run id
        int setId;              // if > 0 then set id of source input parametes
        IDbExec * dbExec;       // db-connection
        DoneVector isSubDone;   // size of [sub-value count], if true then all sub-value accumulators saved in database

        /** initialize "restart run" modeling process. */
        virtual void init(void) override;

        // find sub-value to restart the run and update run status
        bool cleanupRestartSubValue(void);

    private:
        RestartController(const RestartController & i_runCtrl) = delete;
        RestartController & operator=(const RestartController & i_runCtrl) = delete;
    };
}

#endif  // RUN_CTRL_IMPL_H
