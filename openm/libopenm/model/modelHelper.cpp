/**
* @file
* OpenM++ modeling library: helper classes for process run controller.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "model.h"
#include "modelHelper.h"

using namespace std;
using namespace openm;

// atomic bool vector to store subsample done status
void DoneVector::init(size_t i_size)
{
    lock_guard<recursive_mutex> lck(theMutex);
    isDoneVec.assign(i_size, false);
}

// clear all values: set it to false
void DoneVector::reset(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    for (auto && isV : isDoneVec) {
        isV = false;
    }
}

// return true if all values are set to true
bool DoneVector::isAll(void) 
{
    lock_guard<recursive_mutex> lck(theMutex);
    return std::all_of(
        isDoneVec.cbegin(),
        isDoneVec.cend(),
        [](bool i_isDone) -> bool { return i_isDone; }
    );
}

// set to true value at specified position
void DoneVector::setAt(size_t i_pos)
{
    lock_guard<recursive_mutex> lck(theMutex);
    isDoneVec.at(i_pos) = true;
}

// return count of contiguous true values staring from zero position
size_t DoneVector::countFirst(void)
{
    lock_guard<recursive_mutex> lck(theMutex);

    size_t nPos = 0;
    for (; nPos < isDoneVec.size(); nPos++) {
        if (!isDoneVec[nPos]) return nPos;
    }
    return nPos;
}

// set modeling groups size, group count and process rank in group
ProcessGroupDef::ProcessGroupDef(int i_subSampleCount, int i_threadCount, int i_worldSize, int i_worldRank) :
    ProcessGroupDef() 
{
    groupSize = i_subSampleCount / i_threadCount;
    if (groupSize <= 0) groupSize = 1;
    if (groupSize > i_worldSize) groupSize = i_worldSize;

    groupCount = (groupSize > 1) ? i_worldSize / groupSize : (i_worldSize - 1) / groupSize;
    if (groupCount <= 0) groupCount = 1;

    // one-based group number, root is in the last group
    groupOne = (i_worldRank > 0) ? 1 + (i_worldRank - 1) / groupSize : groupCount;

    // "active" root: use root process for modeling, else dedicate it for data exchange only
    isRootActive = groupSize > 1 && i_worldSize <= groupSize * groupCount;

    // get process rank among other active modeling processes in the group
    bool isInLastGroup = groupOne >= groupCount;

    activeRank = 
        (i_worldRank > 0 && (!isInLastGroup || !isRootActive)) ?
        ((i_worldRank - 1) % groupSize) : 
        i_worldRank % groupSize;

    // number of subsamples for modeling process, last process calculate the rest of subsamples
    subPerProcess = i_subSampleCount / groupSize;   // subsamples per modeling process
    selfSubCount = (activeRank < groupSize - 1) ? subPerProcess : i_subSampleCount - ((groupSize - 1) * subPerProcess);

    // is current process active: 
    // "active" process means it is used for modeling 
    // if process count == group size * group count then all processes used for modeliung, including root
    // else root process dedicated to data exchange and children > (group size * group count) unused (not active)
    // isActive = (i_worldRank == 0 && isRootActive) || (0 < i_worldRank && i_worldRank <= groupSize * groupCount);
}

// set initial run group size, assign process ranks and initial state state
RunGroup::RunGroup(int i_groupOne, int i_subSampleCount, const ProcessGroupDef & i_rootGroupDef) : 
    groupOne(i_groupOne),
    runId(0), 
    setId(0),
    groupSize(i_rootGroupDef.groupSize),
    subPerProcess(i_rootGroupDef.subPerProcess),
    isSubDone(i_subSampleCount)
{ 
    firstChildRank = 1 + (i_groupOne - 1) * groupSize;
    childCount = (i_groupOne >= i_rootGroupDef.groupCount && i_rootGroupDef.isRootActive) ? groupSize - 1 : groupSize;
    isUseRoot = (i_groupOne == i_rootGroupDef.groupOne) ? i_rootGroupDef.isRootActive : false;
}

// set group state for next run
void RunGroup::nextRun(int i_runId, int i_setId, ModelStatus i_status)
{ 
    runId = i_runId;
    setId = i_setId;
    state.updateStatus(i_status);
    isSubDone.reset();
}

// return child world rank where subsample is calculated
int RunGroup::rankBySubsampleNumber(int i_subNumber) const
{
    int nProc = i_subNumber / subPerProcess;        // process index in group
    if (nProc >= groupSize) nProc = groupSize - 1;  // last process also calculate the rest

    int nRank = firstChildRank + nProc;             // world rank to calculate subsample

    // if root process is "active" (is used for modeling) then first subsamples calculated at root 
    if (isUseRoot) {
        nRank = i_subNumber < subPerProcess ? 0 : firstChildRank + nProc - 1;
    }
    return nRank;
}