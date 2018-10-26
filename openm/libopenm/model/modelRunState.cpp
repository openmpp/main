// OpenM++ modeling library: model run state thread safe class
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "modelRunState.h"
using namespace openm;

namespace openm
{
    /** i = initial status */
    const char * RunStatus::init = "i";

    /** p = run in progress */
    const char * RunStatus::progress = "p";

    /** w = wait: run in progress, under external supervision */
    const char * RunStatus::waitProgress = "w";

    /** s = completed successfully */
    const char * RunStatus::done = "s";

    /** x = exit and not completed (reserved) */
    const char * RunStatus::exit = "x";

    /** e = error failure */
    const char * RunStatus::error = "e";
}

/** process-wide model run state: status, progress, update times */
static ModelRunState processModelRunState;

/** public interface of process-wide model run state: status, progress, update times */
IModelRunState * theModelRunState = &processModelRunState;

//  model run state public interface
IModelRunState::~IModelRunState(void) throw() { }

/** initialize model run state data with default values */
RunState::RunState(void) : theStatus(ModelStatus::init), progressCount(0), startTime(chrono::system_clock::now())
{
    updateTime = startTime;
}

/** convert run status to model status */
ModelStatus RunState::fromRunStatus(const string & i_runStatus)
{
    if (i_runStatus == RunStatus::init) return ModelStatus::init;
    if (i_runStatus == RunStatus::progress) return ModelStatus::progress;
    if (i_runStatus == RunStatus::waitProgress) return ModelStatus::waitProgress;
    if (i_runStatus == RunStatus::done) return ModelStatus::done;
    if (i_runStatus == RunStatus::exit) return ModelStatus::exit;
    if (i_runStatus == RunStatus::error) return ModelStatus::error;

    return ModelStatus::undefined;
}

/** set modeling progress count */
int RunState::setProgress(int i_progress)
{
    if (!isExit(theStatus)) {
        theStatus = (theStatus == ModelStatus::waitProgress) ? ModelStatus::waitProgress : ModelStatus::progress;
        progressCount = i_progress;
        updateTime = chrono::system_clock::now();
    }
    return progressCount;
}

/** set model status if not already set as one of exit status values */
ModelStatus RunState::setStatus(ModelStatus i_status)
{
    if (!isExit(theStatus)) {
        updateTime = chrono::system_clock::now();
        theStatus = i_status;
    }
    return theStatus;
}

/** initialize model run state */
ModelRunState::ModelRunState(const RunState & i_state)
{
    lock_guard<recursive_mutex> lck(theMutex);
    theStatus = i_state.theStatus;
    progressCount = i_state.progressCount;
    startTime = i_state.startTime;
    updateTime = i_state.updateTime;
}

/** set model run state */
ModelRunState & ModelRunState::operator=(const RunState & i_state)
{
    lock_guard<recursive_mutex> lck(theMutex);
    theStatus = i_state.theStatus;
    progressCount = i_state.progressCount;
    startTime = i_state.startTime;
    updateTime = i_state.updateTime;
    return *this;
}

/** get model status */
ModelStatus ModelRunState::status(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return theStatus;
}

/** return true if status is one of exiting: ie done, exit, error */
bool ModelRunState::isExit(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return RunState::isExit(theStatus);
}

/** return true if status is an error */
bool ModelRunState::isError(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return RunState::isError(theStatus);
}

/** return true if model in shutdown state: modeling completed or one of exiting */
bool ModelRunState::isShutdownOrExit(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return RunState::isShutdownOrExit(theStatus);
}

/** retrun model run state data */
RunState ModelRunState::get(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return *this;
}

/** set model status if not already set as one of exit status values */
ModelStatus ModelRunState::updateStatus(ModelStatus i_status)
{
    lock_guard<recursive_mutex> lck(theMutex);
    setStatus(i_status);
    return theStatus;
}

/** set modeling progress count */
int ModelRunState::updateProgress(int i_progress)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return setProgress(i_progress);
}

/** find model run state, return false and empty model run state if not exist */
tuple<bool, RunState> RunStateHolder::get(int i_runId, int i_subId)
{
    lock_guard<recursive_mutex> lck(theMutex);

    if (auto it = stateMap.find(pair(i_runId, i_subId)); it != stateMap.end()) {
        return{ true, it->second };
    }
    return { false, RunState() };
}

/** add new or replace existing model run state, return true if not already exist */
void RunStateHolder::add(int i_runId, int i_subId, RunState i_state)
{
    lock_guard<recursive_mutex> lck(theMutex);
    stateMap[pair(i_runId, i_subId)] = i_state;
}

/** remove model run state from the store, return false if not exist */
void RunStateHolder::remove(int i_runId, int i_subId)
{
    lock_guard<recursive_mutex> lck(theMutex);
    stateMap.erase(pair(i_runId, i_subId));
}

/** update model run state, return false if not exist */
bool RunStateHolder::update(int i_runId, int i_subId, RunState i_state)
{
    lock_guard<recursive_mutex> lck(theMutex);

    if (auto it = stateMap.find(pair(i_runId, i_subId)); it != stateMap.end()) {
        it->second = i_state;
        return true;
    }
    return false;
}

/** update model status if not already set as one of exit status values, if found then return true and actual status */
ModelStatus RunStateHolder::updateStatus(int i_runId, int i_subId, ModelStatus i_status)
{
    lock_guard<recursive_mutex> lck(theMutex);

    if (auto it = stateMap.find(pair(i_runId, i_subId)); it != stateMap.end()) {
        return it->second.setStatus(i_status);
    }
    return ModelStatus::undefined;
}

/** set modeling progress count, return false if not exist */
bool RunStateHolder::updateProgress(int i_runId, int i_subId, int i_progress)
{
    lock_guard<recursive_mutex> lck(theMutex);

    if (auto it = stateMap.find(pair(i_runId, i_subId)); it != stateMap.end()) {
        it->second.setProgress(i_progress);
        return true;
    }
    return false;
}
    
/** remove from store all run states where status is completed (shutdown, done, exit, error) */
void RunStateHolder::removeShutdownOrExit(void)
{
    lock_guard<recursive_mutex> lck(theMutex);

    for (auto it = stateMap.cbegin(); it != stateMap.cend();) {
        if (it->second.isShutdownOrExit(it->second.theStatus)) {
            it = stateMap.erase(it);
        }
        else {
            ++it;
        }
    }
}

/** copy all run states into output vector of (run id, sub-value id, run state) */
IRowBaseVec RunStateHolder::toRowVector(void)
{
    lock_guard<recursive_mutex> lck(theMutex);

    IRowBaseVec rv;
    rv.reserve(stateMap.size() + 1);

    for (auto it = stateMap.cbegin(); it != stateMap.cend(); ++it) {
        rv.push_back(
            make_unique<RunStateItem>(RunStateItem{ it->first.first, it->first.second, it->second })
        );
    }
    return rv;
}

/** append or replace existing run states from output vector of (run id, sub-value id, run state) */
void RunStateHolder::fromRowVector(const IRowBaseVec & i_src)
{
    lock_guard<recursive_mutex> lck(theMutex);

    for (const auto & rsiUp : i_src) {
        const RunStateItem * rsi = static_cast<const RunStateItem *>(rsiUp.get());
        stateMap[pair(rsi->runId, rsi->subId)] = rsi->state;
    }
}
