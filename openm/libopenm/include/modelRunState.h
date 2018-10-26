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
    /** model run state: thread safe */
    class ModelRunState : public virtual IModelRunState, private RunState
    {
    public:
        /** initialize model run state */
        ModelRunState(void) : RunState() { };

        /** initialize model run state */
        ModelRunState(const RunState & i_state);

        /** set model run state */
        ModelRunState & operator=(const RunState & i_state);

        /** get model status */
        ModelStatus status(void) override;

        /** return true if status is one of exiting: ie done, exit, error */
        bool isExit(void) override;

        /** return true if status is an error */
        bool isError(void) override;

        /** return true if model in shutdown state: modeling completed or one of exiting */
        bool isShutdownOrExit(void) override;

        /** retrun model run state data */
        RunState get(void) override;

        /** set model status if not already set as one of exit status values */
        ModelStatus updateStatus(ModelStatus i_status) override;

        /** set modeling progress count */
        int updateProgress(int i_progress) override;

    private:
        recursive_mutex theMutex;                       // mutex to lock status upadte operations

    private:
        ModelRunState(const ModelRunState & i_state) = delete;
        ModelRunState(const ModelRunState && i_state) = delete;
        ModelRunState & operator=(const ModelRunState & i_state) = delete;

    };

    /** modeling run state for all modeling threads, identified by (run id, sub-value id) pair */
    class RunStateHolder
    {
    public:
        RunStateHolder(void) {}

        /** find model run state, return false and empty model run state if not exist */
        tuple<bool, RunState> get(int i_runId, int i_subId);

        /** add new or replace existing model run state, return true if not already exist */
        void add(int i_runId, int i_subId) { add(i_runId, i_subId, RunState()); };

        /** add new or replace existing model run state, return true if not already exist */
        void add(int i_runId, int i_subId, RunState i_state);

        /** remove model run state from the store, return false if not exist */
        void remove(int i_runId, int i_subId);

        /** update model run state, return false if not exist */
        bool update(int i_runId, int i_subId, RunState i_state);

        /** update model status if not already set as one of exit status values, if found then return true and actual status */
        ModelStatus updateStatus(int i_runId, int i_subId, ModelStatus i_status);

        /** set modeling progress count, return false if not exist */
        bool updateProgress(int i_runId, int i_subId, int i_progress);

        /** remove from store all run states where status is completed (shutdown, done, exit, error) */
        void removeShutdownOrExit(void);

        /** copy all run states into output vector of (run id, sub-value id, run state) */
        IRowBaseVec toRowVector(void);

        /** append or replace existing run states from output vector of (run id, sub-value id, run state) */
        void fromRowVector(const IRowBaseVec & i_src);

    private:
        recursive_mutex theMutex;               // mutex to lock access operations
        map<pair<int, int>, RunState> stateMap; // store of run state for each modeling thread, key: (run id, sub-value id) pair

    private:
        RunStateHolder(const RunStateHolder & i_state) = delete;
        RunStateHolder & operator=(const RunStateHolder & i_state) = delete;
    };
}

#endif  // MODEL_RUN_STATE_H
