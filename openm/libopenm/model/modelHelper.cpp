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

/** find index in i_entityArr by entity name and attribute name, return -1 if names not found */
int EntityNameSizeItem::byName(const char * i_entityName, const char * i_attrName)
{
    if (ENTITY_NAME_SIZE_ARR_LEN <= 0) return -1;

    for (int k = 0; k < (int)ENTITY_NAME_SIZE_ARR_LEN; k++)
    {
        if (!strncmp(EntityNameSizeArr[k].entity, i_entityName, OM_STRLEN_MAX) &&
            !strncmp(EntityNameSizeArr[k].attribute, i_attrName, OM_STRLEN_MAX))
        {
            return k;
        }
    }
    return -1;
}

/** return event name by event id or NULL if id not found. */
const char * EventIdNameItem::byId(int i_eventId)
{
    // code below assumes EventIdNameArr first element is a special "no event" entry
    // the rest of EventIdNameArr elements ordered by event id and that id is an index for event name
    // for example:
    // {
    //   -1, "(no event)",
    //    0, "BirthEvent",
    //    1, "RetirementEvent",
    //    2, "DeathEvent"
    // }

    if (!checkId(i_eventId)) return nullptr;

    // check special case "no event" at the first element of EventIdNameArr
    if (EventIdNameArr[0].eventId == i_eventId) return EventIdNameArr[0].eventName;

    // checkId() guarantee: event (id + 1) is an index of EventIdNameArr array
    return (EventIdNameArr[i_eventId + 1].eventId == i_eventId) ? EventIdNameArr[i_eventId + 1].eventName : nullptr;
}

// atomic bool vector to store sub-value done status
void DoneVector::init(size_t i_size)
{
    lock_guard<recursive_mutex> lck(theMutex);
    isDoneVec.assign(i_size, false);
}

// clear all values: set it to false
void DoneVector::reset(void)
{
    lock_guard<recursive_mutex> lck(theMutex);
    std::fill(isDoneVec.begin(), isDoneVec.end(), false);
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
int DoneVector::countFirst(void)
{
    lock_guard<recursive_mutex> lck(theMutex);

    int nPos = 0;
    for (; nPos < (int)isDoneVec.size(); nPos++) {
        if (!isDoneVec[nPos]) return nPos;
    }
    return nPos;
}

// set modeling groups size, group count and process rank in group
ProcessGroupDef::ProcessGroupDef(int i_subValueCount, int i_threadCount, bool i_isRootIdle, int i_worldSize, int i_worldRank) :
    ProcessGroupDef() 
{
    int nWorldSize = (i_isRootIdle && i_worldSize > 1) ? i_worldSize - 1 : i_worldSize;

    groupSize = i_subValueCount / i_threadCount;
    if (i_subValueCount % i_threadCount) groupSize++;
    if (groupSize <= 0) groupSize = 1;
    if (groupSize > nWorldSize) groupSize = nWorldSize;

    groupCount = (i_worldSize - 1) / groupSize;
    if (groupCount <= 0) groupCount = 1;

    // one-based group number, root is in the last group
    groupOne = (i_worldRank > 0) ? 1 + (i_worldRank - 1) / groupSize : groupCount;

    // "active" root: use root process for modeling, else dedicate it for data exchange only
    isRootActive = i_worldSize == 1 || (!i_isRootIdle && groupSize > 1 && i_worldSize <= groupSize * groupCount);

    // get process rank among other active modeling processes in the group
    bool isInLastGroup = groupOne >= groupCount;

    activeRank = 
        (i_worldRank > 0 && (!isInLastGroup || !isRootActive)) ?
        ((i_worldRank - 1) % groupSize) : 
        i_worldRank % groupSize;

    // number of sub-values for modeling process
    subPerProcess = i_subValueCount / groupSize;
    if (i_subValueCount % groupSize) subPerProcess++;
    if (subPerProcess < i_threadCount) subPerProcess = i_threadCount;
    if (subPerProcess > i_subValueCount) subPerProcess = i_subValueCount;

    rootSubCount = 0;
    if (!i_isRootIdle) {
        rootSubCount = i_subValueCount - subPerProcess * (i_worldSize - 1);
        if (rootSubCount < 0) rootSubCount = 0;
    }
    firstSubId = 0;
    selfSubCount = rootSubCount;

    if (i_worldRank > 0) {
        firstSubId = rootSubCount + subPerProcess * (i_worldRank - 1);

        selfSubCount = i_subValueCount - firstSubId;
        if (selfSubCount <= 0) firstSubId = 0;      // not active process
    }
    if (selfSubCount < 0) selfSubCount = 0;         // not active process
    if (selfSubCount > subPerProcess) selfSubCount = subPerProcess;

    // is current process active: 
    // "active" process means it is used for modeling 
    // if process count == group size * group count then all processes used for modeling, including root
    // else root process dedicated to data exchange and children > (group size * group count) unused (not active)
    // isActive = (i_worldRank == 0 && isRootActive) || (0 < i_worldRank && i_worldRank <= groupSize * groupCount);
}

// set initial run group size, assign process ranks and initial state state
RunGroup::RunGroup(int i_groupOne, int i_subValueCount, const ProcessGroupDef & i_rootGroupDef) : 
    groupOne(i_groupOne),
    runId(0), 
    setId(0),
    groupSize(i_rootGroupDef.groupSize),
    subPerProcess(i_rootGroupDef.subPerProcess),
    rootSubCount(i_rootGroupDef.rootSubCount),
    isSubDone(i_subValueCount)
{ 
    firstChildRank = 1 + (i_groupOne - 1) * groupSize;
    childCount = (i_groupOne >= i_rootGroupDef.groupCount && i_rootGroupDef.isRootActive) ? groupSize - 1 : groupSize;
    isUseRoot = (i_groupOne == i_rootGroupDef.groupOne) ? i_rootGroupDef.isRootActive : false;
    childState = vector<ModelRunState>(childCount);
}

// set group state for next run
void RunGroup::nextRun(int i_runId, int i_setId, ModelStatus i_status)
{ 
    runId = i_runId;
    setId = i_setId;
    state.updateStatus(i_status);
    isSubDone.reset();
}

// return child world rank where sub-value is calculated
int RunGroup::rankBySubValueId(int i_subId) const
{
    // if root process is "active" (is used for modeling) then first sub-values calculated at root
    if (isUseRoot && i_subId < rootSubCount) return 0;

    int nSub = !isUseRoot ? i_subId : i_subId - rootSubCount;

    int nProc = nSub / subPerProcess;               // process index in group
    if (nProc >= groupSize) nProc = groupSize - 1;  // last process also calculate the rest

    return firstChildRank + nProc;                  // world rank to calculate sub-value
}
