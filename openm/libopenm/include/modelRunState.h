/**
 * @file
 * OpenM++ modeling library: implementation.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MODEL_RUN_STATE_H
#define MODEL_RUN_STATE_H

#include <chrono>
#include <list>
#include <mutex>
using namespace std;

namespace openm
{
    /** model run state holder */
    extern class ModelRunState theModelRunState;

    /** modelling job status */
    enum class ModelStatus
    {
        /** initial state */
        init = 0,

        /** run in progress */
        progress,

        /** waiting */
        wait,

        /** completed */
        done = 64,

        /** exit and not completed */
        exit,

        /** error failure */
        error = 128
    };

    /** model run state: thread safe */
    class ModelRunState
    {
    public:
        /** initialize model run state */
        ModelRunState(void);

        /** initialize model run state */
        explicit ModelRunState(const ModelRunState & i_state);

        /** copy model run state */
        ModelRunState & operator=(const ModelRunState & i_state);

        /** get model status */
        ModelStatus status(void);

        /** set model status and return it */
        ModelStatus status(ModelStatus i_status);

        /** set model status if not already set as one of exit status values */
        ModelStatus statusIfNotExit(ModelStatus i_status);

        /** return true if status is one of exiting: ie done or exit */
        bool isExit(void);

        /** return true if status is one of exiting: ie done or exit */
        static bool isExit(ModelStatus i_status) { return i_status >= ModelStatus::done; }

    private:
        ModelStatus theStatus;                          // current status
        chrono::system_clock::time_point startTime;     // process start time
        chrono::system_clock::time_point updateTime;    // last update time
    };
}

#endif  // MODEL_RUN_STATE_H
