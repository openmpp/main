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
IFinalState::~IFinalState(void) noexcept { }
IModelRunState::~IModelRunState(void) noexcept { }

/** initialize model run state data with default values */
RunState::RunState(void) : theStatus(ModelStatus::init), progressCount(0), progressValue(0.0), startTime(chrono::system_clock::now())
{
    updateTime = startTime;
}

/** check if two run states are equal: state, progress and update times are equal */
bool RunState::operator==(const RunState & i_other) const
{
    return theStatus == i_other.theStatus &&
        progressCount == i_other.progressCount && progressValue == i_other.progressValue &&
        startTime == i_other.startTime && updateTime == i_other.updateTime;
}

/** convert run status to model status code */
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

/** convert model status to run status code */
string RunState::toRunStatus(ModelStatus i_modelStatus)
{
    if (i_modelStatus == ModelStatus::init) return RunStatus::init;
    if (i_modelStatus == ModelStatus::progress) return RunStatus::progress;
    if (i_modelStatus == ModelStatus::waitProgress) return RunStatus::waitProgress;
    if (i_modelStatus == ModelStatus::shutdown) return RunStatus::progress; // shutdown is a progress after modeling completed
    if (i_modelStatus == ModelStatus::done) return RunStatus::done;
    if (i_modelStatus == ModelStatus::exit) return RunStatus::exit;
    if (RunState::isError(i_modelStatus)) return RunStatus::error;

    return "";  // run status undefined
}

/** set modeling progress count and value */
void RunState::setProgress(int i_count, double i_value)
{
    if (!isFinal(theStatus)) {
        theStatus = (theStatus == ModelStatus::waitProgress) ? ModelStatus::waitProgress : ModelStatus::progress;
        progressCount = i_count;
        progressValue = i_value;
        updateTime = chrono::system_clock::now();
    }
    else {
        if (progressCount < i_count) progressCount = i_count;
        if (progressValue < i_value) progressValue = i_value;
    }
}

/** set model status if not already set as one of final status values */
ModelStatus RunState::setStatus(ModelStatus i_status)
{
    if (!isFinal(theStatus)) {
        updateTime = chrono::system_clock::now();
        theStatus = i_status;
    }
    return theStatus;
}

/** get model status */
ModelStatus ModelRunState::status(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return theStatus;
}

/** return true if status is one of exiting: ie done, exit, error */
bool ModelRunState::isFinal(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return RunState::isFinal(theStatus);
}

/** return true if status is an error */
bool ModelRunState::isError(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return RunState::isError(theStatus);
}

/** return true if model in shutdown state: modeling completed or one of exiting */
bool ModelRunState::isShutdownOrFinal(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return RunState::isShutdownOrFinal(theStatus);
}

/** return model run state data */
RunState ModelRunState::get(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    return *this;
}

/** set model status if not already set as one of final status values */
ModelStatus ModelRunState::updateStatus(ModelStatus i_status)
{
    lock_guard<recursive_mutex> lck(theMutex);
    setStatus(i_status);
    return theStatus;
}

/** set modeling progress count and value */
void ModelRunState::updateProgress(int i_count, double i_value)
{
    lock_guard<recursive_mutex> lck(theMutex);
    setProgress(i_count, i_value);
}

/** find sub-value run state, return false and empty sub-value run state if not exist */
tuple<bool, RunState> RunStateHolder::get(int i_runId, int i_subId)
{
    lock_guard<recursive_mutex> lck(theMutex);

    if (auto it = stateMap.find(pair(i_runId, i_subId)); it != stateMap.end()) {
        return { true, it->second };
    }
    return { false, RunState() };
}
/** add new or replace existing sub-value run state */
void RunStateHolder::add(int i_runId, int i_subId, RunState i_state)
{
    lock_guard<recursive_mutex> lck(theMutex);
    stateMap[pair(i_runId, i_subId)] = i_state;
    updateStateMap[pair(i_runId, i_subId)] = i_state;
}

/** update sub-value status if not already set as one of final status values, return actual status or undefined if not found */
ModelStatus RunStateHolder::updateStatus(int i_runId, int i_subId, ModelStatus i_status, bool i_isFinalUpdate)
{
    lock_guard<recursive_mutex> lck(theMutex);

    if (auto it = stateMap.find(pair(i_runId, i_subId)); it != stateMap.end()) {

        ModelStatus mStatus = it->second.setStatus(i_status);
        updateStateMap[pair(i_runId, i_subId)] = it->second;

        if (i_isFinalUpdate) {
            stateMap.erase(it); // this final status update, keep only updated status
        }
        return mStatus;
    }
    return ModelStatus::undefined;
}

/**  set modeling progress count and value, return false if not exist */
bool RunStateHolder::updateProgress(int i_runId, int i_subId, int i_count, double i_value)
{
    lock_guard<recursive_mutex> lck(theMutex);

    if (auto it = stateMap.find(pair(i_runId, i_subId)); it != stateMap.end()) {
        it->second.setProgress(i_count, i_value);
        updateStateMap[pair(i_runId, i_subId)] = it->second;
        return true;
    }
    return false;
}
    
/** return updated sub-values run state since previous call.
* it return non empty results not more often than OM_STATE_SAVE_TIME unless i_isNow is true.
*/
const map<pair<int, int>, RunState> RunStateHolder::saveUpdated(bool i_isNow)
{
    lock_guard<recursive_mutex> lck(theMutex);

    // if no delay requested then return updates now
    if (i_isNow) {
        return map<pair<int, int>, RunState>(move(updateStateMap)); // move updates out: return updated run states and clear it
    }

    // exit if no updates since last save
    if (updateStateMap.size() <= 0) return map<pair<int, int>, RunState>();

    // check interval since last save and exit if it is less than save interval to reduce overhead
    chrono::system_clock::time_point nowTime = chrono::system_clock::now();

    if (nowTime < lastSaveTime + chrono::milliseconds(OM_STATE_SAVE_TIME)) {
        return map<pair<int, int>, RunState>();     // return empty updates: calls are coming too often
    }
    lastSaveTime = nowTime;

    return map<pair<int, int>, RunState>(move(updateStateMap)); // move updates out: return updated run states and clear it
}

/** copy updated run states into output vector of (run id, sub-value id, run state), last element is process run state */
IRowBaseVec RunStateHolder::saveToRowVector(int i_runId)
{
    lock_guard<recursive_mutex> lck(theMutex);

    IRowBaseVec rv;
    rv.reserve(updateStateMap.size() + 1);

    for (const auto & rst : updateStateMap) {
        rv.push_back(
            make_unique<RunStateItem>(RunStateItem{ rst.first.first, rst.first.second, rst.second })
        );
    }
    updateStateMap.clear(); // clear updates after saving

    // append process run state
    rv.push_back(
        make_unique<RunStateItem>(RunStateItem{ i_runId,  0, theModelRunState->get() })
    );
    return rv;
}

/** append or replace existing run states from received vector of (run id, sub-value id, run state) and return child process state */
const RunState RunStateHolder::fromRowVector(const IRowBaseVec & i_src)
{
    if (i_src.size() < 1) return RunState();    // at least one element expectd: process status

    lock_guard<recursive_mutex> lck(theMutex);

    // update sub-values status
    for (size_t k = 0; k < i_src.size() - 1; k++) {
        const RunStateItem * rsi = static_cast<const RunStateItem *>(i_src[k].get());
        stateMap[pair(rsi->runId, rsi->subId)] = rsi->state;
        updateStateMap[pair(rsi->runId, rsi->subId)] = rsi->state;
    }

    // process state is last element of states vector
    const RunStateItem * rsi = static_cast<const RunStateItem *>(i_src[i_src.size() - 1].get());
    return rsi->state;
}
