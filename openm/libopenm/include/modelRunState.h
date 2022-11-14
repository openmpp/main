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
#include "helper.h"

namespace openm
{
    /** model run state: thread safe */
    class ModelRunState : public virtual IModelRunState, private RunState
    {
    public:
        /** initialize model run state */
        ModelRunState(void) : RunState() { };

        /** get model status */
        ModelStatus status(void) override;

        /** return true if status is one of exiting: ie done, exit, error */
        bool isFinal(void) override;

        /** return true if status is an error */
        bool isError(void) override;

        /** return true if model in shutdown state: modeling completed or one of exiting */
        bool isShutdownOrFinal(void) override;

        /** return model run state data */
        RunState get(void) override;

        /** set model status if not already set as one of final status values */
        ModelStatus updateStatus(ModelStatus i_status) override;

        /** set modeling progress count and value */
        void updateProgress(int i_count, double i_value = 0.0) override;

    private:
        recursive_mutex theMutex;                       // mutex to lock status upadte operations

    private:
        ModelRunState(const ModelRunState & i_state) = delete;
        ModelRunState & operator=(const ModelRunState & i_state) = delete;

    };

    /** sub-value run state for all modeling threads, identified by (run id, sub-value id) pair */
    class RunStateHolder
    {
    public:
        RunStateHolder(void) { }

        /** find sub-value run state, return false and empty sub-value run state if not exist */
        tuple<bool, RunState> get(int i_runId, int i_subId);

        /** add new or replace existing sub-value run state */
        void add(int i_runId, int i_subId) { add(i_runId, i_subId, RunState()); };

        /** add new or replace existing sub-value run state */
        void add(int i_runId, int i_subId, RunState i_state);

        /** update sub-value status if not already set as one of final status values, return actual status or undefined if not found */
        ModelStatus updateStatus(int i_runId, int i_subId, ModelStatus i_status, bool i_isFinalUpdate = true);

        /**  set modeling progress count and value, return false if not exist */
        bool updateProgress(int i_runId, int i_subId, int i_count, double i_value);

        /** return updated sub-values run state since previous call.
        * it return non empty results not more often than OM_STATE_SAVE_TIME unless i_isNow is true.
        */
        const map<pair<int, int>, RunState> saveUpdated(bool i_isNow = false);

        /** copy updated run states into output vector of (run id, sub-value id, run state), last element is process run state */
        IRowBaseVec saveToRowVector(int i_runId);

        /** append or replace existing run states from received vector of (run id, sub-value id, run state) and return child process state */
        const RunState fromRowVector(const IRowBaseVec & i_src);

    private:
        recursive_mutex theMutex;                           // mutex to lock access operations
        map<pair<int, int>, RunState> stateMap;             // store of current runs state for all sub-values, key: (run id, sub-value id) pair
        map<pair<int, int>, RunState> updateStateMap;       // run states updated since last save call
        chrono::system_clock::time_point lastSaveTime = chrono::system_clock::time_point::min();   // last time of save

    private:
        RunStateHolder(const RunStateHolder & i_state) = delete;
        RunStateHolder & operator=(const RunStateHolder & i_state) = delete;
    };
}

#endif  // MODEL_RUN_STATE_H
