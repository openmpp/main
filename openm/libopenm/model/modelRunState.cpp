// OpenM++ modeling library: model run state thread safe class
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "modelRunState.h"
using namespace openm;

namespace openm
{
    /** i = initial status */
    const char * RunStatus::init = "s";

    /** p = run in progress */
    const char * RunStatus::progress = "p";

    /** s = completed successfully */
    const char * RunStatus::done = "s";

    /** x = exit and not completed (reserved) */
    const char * RunStatus::exit = "x";

    /** e = error failure */
    const char * RunStatus::error = "e";
}

// process-wide model run state holder
ModelRunState openm::theModelRunState;

// mutex to lock run state operations
static recursive_mutex mrsMutex;

/** initialize model run state */
ModelRunState::ModelRunState(void) :
    theStatus(ModelStatus::init),
    progressCount(0),
    startTime(chrono::system_clock::now())
{
    updateTime = startTime;
}

/** initialize model run state */
ModelRunState::ModelRunState(const ModelRunState && i_state)
{
    lock_guard<recursive_mutex> lck(mrsMutex);
    theStatus = i_state.theStatus;
    progressCount = i_state.progressCount;
    startTime = i_state.startTime;
    updateTime = i_state.updateTime;
}

/** copy model run state */
ModelRunState & ModelRunState::operator=(const ModelRunState & i_state)
{
    if (this != &i_state) {
        lock_guard<recursive_mutex> lck(mrsMutex);
        theStatus = i_state.theStatus;
        progressCount = i_state.progressCount;
        startTime = i_state.startTime;
        updateTime = i_state.updateTime;
    }
    return *this;
}

/** get model status */
ModelStatus ModelRunState::status(void)
{
    lock_guard<recursive_mutex> lck(mrsMutex);
    return theStatus;
}

/** set model status */
ModelStatus ModelRunState::status(ModelStatus i_status)
{
    lock_guard<recursive_mutex> lck(mrsMutex);
    updateTime = chrono::system_clock::now();
    theStatus = i_status;
    return theStatus;
}

/** set model status if not already set as one of exit status values */
ModelStatus ModelRunState::updateStatus(ModelStatus i_status)
{
    lock_guard<recursive_mutex> lck(mrsMutex);
    if (!isExit(theStatus)) status(i_status);
    return theStatus;
}

/** return true if status is one of exiting: ie done, exit, error */
bool ModelRunState::isExit(void)
{
    lock_guard<recursive_mutex> lck(mrsMutex);
    return isExit(theStatus);
}

/** return true if status is an error */
bool ModelRunState::isError(void)
{
    lock_guard<recursive_mutex> lck(mrsMutex);
    return isError(theStatus);
}

/** update modeling progress */
int ModelRunState::updateProgress(void) 
{ 
    lock_guard<recursive_mutex> lck(mrsMutex);
    if (!isExit(theStatus)) {
        theStatus = ModelStatus::progress;
        progressCount++;
        updateTime = chrono::system_clock::now();
    }
    return progressCount;
}
