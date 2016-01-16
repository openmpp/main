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
#include <forward_list>
using namespace std;

#include "libopenm/omModel.h"

namespace openm
{
    /** model run state holder */
    extern class ModelRunState theModelRunState;

    /** model run state: thread safe */
    class ModelRunState
    {
    public:
        /** initialize model run state */
        ModelRunState(void);

        /** initialize model run state */
        explicit ModelRunState(const ModelRunState && i_state);

        /** copy model run state */
        ModelRunState & operator=(const ModelRunState & i_state);

        /** get model status */
        ModelStatus status(void);

        /** set model status if not already set as one of exit status values */
        ModelStatus updateStatus(ModelStatus i_status);

        /** return true if status is one of exiting: ie done, exit, error */
        bool isExit(void);

        /** return true if status is one of exiting: ie done, exit, error */
        static bool isExit(ModelStatus i_status) { return i_status >= ModelStatus::done; }

        /** return true if status is an error */
        bool isError(void);

        /** return true if status is an error */
        static bool isError(ModelStatus i_status) { return i_status >= ModelStatus::error; }

        /** update modeling progress */
        int updateProgress(void);

    private:
        ModelStatus theStatus;                          // current status
        int progressCount;                              // progress count
        chrono::system_clock::time_point startTime;     // process start time
        chrono::system_clock::time_point updateTime;    // last update time

        /** set model status and return it */
        ModelStatus status(ModelStatus i_status);

    private:
        ModelRunState(const ModelRunState & i_state) = delete;
    };
}

#endif  // MODEL_RUN_STATE_H
