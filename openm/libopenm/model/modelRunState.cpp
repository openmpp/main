// OpenM++ modeling library: model run state thread safe class
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "modelRunState.h"
using namespace openm;

// process-wide model run state holder
ModelRunState openm::theModelRunState;

// mutex to lock run state operations
static recursive_mutex mrsMutex;

/** initialize model run state */
ModelRunState::ModelRunState(void) :
    theStatus(ModelStatus::init),
    startTime(chrono::system_clock::now())
{
    updateTime = startTime;
}

/** initialize model run state */
ModelRunState::ModelRunState(const ModelRunState && i_state)
{
    lock_guard<recursive_mutex> lck(mrsMutex);
    theStatus = i_state.theStatus;
    startTime = i_state.startTime;
    updateTime = i_state.updateTime;
}

/** copy model run state */
ModelRunState & ModelRunState::operator=(const ModelRunState & i_state)
{
    if (this != &i_state) {
        lock_guard<recursive_mutex> lck(mrsMutex);
        theStatus = i_state.theStatus;
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
ModelStatus ModelRunState::statusIfNotExit(ModelStatus i_status)
{
    lock_guard<recursive_mutex> lck(mrsMutex);
    if (!isExit(theStatus)) status(i_status);
    return theStatus;
}

/** return true if status is one of exiting: ie done or exit */
bool ModelRunState::isExit(void)
{
    lock_guard<recursive_mutex> lck(mrsMutex);
    return isExit(theStatus);
}

