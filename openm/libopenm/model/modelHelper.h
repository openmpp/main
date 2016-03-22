/**
* @file
* OpenM++ modeling library: helper classes for process run controller.
*/
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MODEL_HELPER_H
#define MODEL_HELPER_H

using namespace std;

namespace openm
{
    // atomic bool vector to store subsample done status
    class DoneVector
    {
    public:
        DoneVector(size_t i_size) { init(i_size); };

        // create new vector of bool values
        void init(size_t i_size);

        // clear all values: set it to false
        void reset(void);

        // return true if all values are set to true
        bool isAll(void);

        // set to true value at specified position
        void setAt(size_t i_pos);

        // return count of contiguous true values staring from zero position
        size_t countFirst(void);

    private:
        recursive_mutex theMutex;   // mutex to lock access operations
        vector<bool> isDoneVec;     // if true then all subsample accumulators saved in database

    private:
        DoneVector(const DoneVector & i_doneVec) = delete;
        DoneVector & operator=(const DoneVector & i_doneVec) = delete;
    };

    // helper struct to define modeling groups size and count: groups of processes for parallel run of modeling task
    struct ProcessGroupDef
    {
        int groupSize;          // size of modeling group
        int groupCount;         // number of modeling group
        int activeRank;         // active rank in group: process index among other modeling processes in the group
        int groupOne;           // current process modeling group number, one based, not a zero based
        bool isRootActive;      // if true then root process used for modeling else dedicated for data exchange
        int subPerProcess;      // number of subsamples per modeling process, except of last process where rest is calculated
        int selfSubCount;       // number of subsamples for current process

        static const int all = 0;   // worldwide group, all processes

        ProcessGroupDef(void) : 
            groupSize(1), groupCount(0), activeRank(0), groupOne(0), isRootActive(true), subPerProcess(1), selfSubCount(1)
        { }

        ProcessGroupDef(int i_subSampleCount, int i_threadCount, int i_worldSize, int i_worldRank);
    };

    // helper struct to hold modeling group run id and run state
    struct RunGroup 
    {
        int groupOne;           // modeling group number, one based, not a zero based
        int runId;              // if > 0 then model run id
        int setId;              // if > 0 then set id of source input parametes
        int firstChildRank;     // world rank of first child process
        int childCount;         // number of child processes in group
        bool isUseRoot;         // if true then root process used for modeling else dedicated for data exchange
        int groupSize;          // size of modeling group
        int subPerProcess;      // number of subsamples per modeling process, except of last process where rest is calculated
        ModelRunState state;    // group status and modeling progress
        DoneVector isSubDone;   // size of [subsample count], if true then all subsample accumulators saved in database

        // set initial run group size, assign process ranks and initial state state
        RunGroup(int i_groupOne, int i_subSampleCount, const ProcessGroupDef & i_rootGroupDef);

        // set group state for next run
        void nextRun(int i_runId, int i_setId, ModelStatus i_status);

        // clear group state after run
        void reset(void) { nextRun(0, 0, ModelStatus::init); }

        // return child world rank where subsample is calculated
        int rankBySubsampleNumber(int i_subNumber) const;

    private:
        RunGroup(const RunGroup & i_runGroup) = delete;
        RunGroup & operator=(const RunGroup & i_runGroup) = delete;
    };

    // helper struct to receive output table values for each accumulator
    struct AccReceive
    {
        int runId;              // run id to receive
        int subNumber;          // subsample number to receive
        int tableId;            // output table id
        int accId;              // accumulator id
        long long valueCount;   // size of accumulator data
        bool isReceived;        // if true then data received
        int senderRank;         // sender rank: process where accumulator calculated
        int msgTag;             // accumulator message tag

        AccReceive(
            int i_runId,
            int i_subNumber,
            int i_subSampleCount,
            int i_senderRank,
            int i_tableId,
            int i_accId,
            int i_accIndex,
            long long i_valueCount
            ) :
            runId(i_runId),
            subNumber(i_subNumber),
            tableId(i_tableId),
            accId(i_accId),
            valueCount(i_valueCount),
            isReceived(false),
            senderRank(i_senderRank),
            msgTag(accMsgTag(i_subNumber, i_subSampleCount, i_accIndex))
        { }

        /** return accumulator message tag */
        static int accMsgTag(int i_subNumber, int i_subSampleCount, int i_accIndex)
        {
            return ((int)MsgTag::outSubsampleBase + i_accIndex) * i_subSampleCount + i_subNumber;
        }
    };
}

#endif  // MODEL_HELPER_H
