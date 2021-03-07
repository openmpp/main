/**
 * @file
 * OpenM++ modeling library: model run state public interface.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_H_MODEL_RUN_STATE_H
#define OM_H_MODEL_RUN_STATE_H

#include <chrono>
#include "common/omHelper.h"   // define IRowBase

namespace openm
{
    /** model exit status */
    enum class ExitStatus : int
    {
        /** completed successfully */
        OK = 0,

        /** unknown error */
        FAIL = 1,

        /** helper exception: low level error from openM++ runtime */
        HELPER_ERROR = 4,

        /** messaging exception: message passing (MPI) error */
        MSG_ERROR = 8,

        /** database exception: database error from openM++ runtime or database vendor */
        DB_ERROR = 12,

        /** model exception: top level error from openM++ runtime */
        MODEL_ERROR = 16,

        /** simulation exception raised in model code */
        SIMULATION_ERROR = 32
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

        /** run in progress, under external supervision */
        waitProgress,

        /** shutdown model process */
        shutdown,

        /** completed successfully */
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

        /** w = wait: run in progress, under external supervision */
        static const char * waitProgress;

        /** s = completed successfully */
        static const char * done;

        /** x = exit and not completed (reserved) */
        static const char * exit;

        /** e = error failure */
        static const char * error;
    };
    
    /** model run state data: status, progress, update times */
    struct RunState
    {
        /** current status */
        ModelStatus theStatus;

        /** progress count, usually percent completed */
        int progressCount;

        /** progress value: number of cases or time completed */
        double progressValue;

        /** process start time */
        std::chrono::system_clock::time_point startTime;

        /** last update time */
        std::chrono::system_clock::time_point updateTime;

        /** initialize model run state data with default values */
        RunState(void);

        /** check if two run states are equal: state, progress and update times are equal */
        bool operator==(const RunState & i_other) const;

        /** return true if status is one of exiting: ie done, exit, error */
        static bool isFinal(ModelStatus i_status) { return i_status >= ModelStatus::done; }

        /** return true if status is an error */
        static bool isError(ModelStatus i_status) { return i_status >= ModelStatus::error; }

        /** return true if model in shutdown state: modeling completed or one of exiting */
        static bool isShutdownOrFinal(ModelStatus i_status) { return i_status == ModelStatus::shutdown || isFinal(i_status); }

        /** convert run status to model status code */
        static ModelStatus fromRunStatus(const std::string & i_runStatus);

        /** convert model status to run status code */
        static std::string toRunStatus(ModelStatus i_modelStatus);

        /** set model status if not already set as one of final status values */
        ModelStatus setStatus(ModelStatus i_status);

        /** set modeling progress count and value */
        void setProgress(int i_count, double i_value);
    };

    /** model run state public interface: thread safe */
    struct IModelRunState : public IFinalState
    {
        virtual ~IModelRunState(void) noexcept = 0;

        /** get model status */
        virtual ModelStatus status(void) = 0;

        /** return true if model in shutdown state: modeling completed or one of exiting */
        virtual bool isShutdownOrFinal(void) = 0;

        /** retrun model run state data */
        virtual RunState get(void) = 0;

        /** set model status if not already set as one of final status values */
        virtual ModelStatus updateStatus(ModelStatus i_status) = 0;

        /** set modeling progress count and value */
        virtual void updateProgress(int i_count, double i_value = 0.0) = 0;
    };

    /** run state data for sub-value identified by run id and sub-value id */
    struct RunStateItem : public IRowBase
    {
        /** if >0 then model run id */
        int runId;

        /** sub-value id */
        int subId;

        /** run state data for the sub-value */
        RunState state;

        /** initialize run state data for sub-value */
        RunStateItem(int i_runId, int i_subId, const RunState i_state) : runId(i_runId), subId(i_subId), state(i_state) {}
        RunStateItem(void) : RunStateItem(0, 0, RunState()) {}
    };
}

#endif  // OM_H_MODEL_RUN_STATE_H
