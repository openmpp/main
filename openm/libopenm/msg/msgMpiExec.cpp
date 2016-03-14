/**
 * @file
 * OpenM++: message passing library main class for MPI-based implementation
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifdef OM_MSG_MPI

using namespace std;

#include "msgCommon.h"
using namespace openm;

/**
* create message passing interface by MPI_Init.
*
* @param[in,out] argc  main argc to pass to MPI_Init
* @param[in,out] argv  main argv to pass to MPI_Init
*/
MpiExec::MpiExec(int & argc, char ** & argv) :
    worldGroup(MPI_GROUP_NULL), 
    groupComm(MPI_COMM_WORLD),
    MsgExecBase()
{
    // cleanup resources on failure
    exit_guard<MpiExec> onExit(this, &MpiExec::cleanup);

    // initialize MPI
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        int mpiRet = MPI_Init(&argc, &argv);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

        // return error code instead of aborting process
        mpiRet = MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

        // get ourself rank and number of modeling processes
        mpiRet = MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

        group_rank = worldRank;

        mpiRet = MPI_Comm_size(MPI_COMM_WORLD, &worldCommSize);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

        mpiRet = MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw MsgException(ex.what());
    }

    onExit.hold();  // initialization completed OK
}

/** cleanup message passing resources by MPI_Finalize. */
MpiExec::~MpiExec(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);
        cleanup();
    }
    catch (...) { }
}

// Cleanup MPI resources
void MpiExec::cleanup(void) throw()
{
    try {
        for (MPI_Comm & mc : mpiCommVec) {
            if (mc != MPI_COMM_NULL && mc != MPI_COMM_WORLD) MPI_Comm_free(&mc);
        }
        mpiCommVec.clear();

        for (MPI_Group & mg : mpiGroupVec) { 
            if (mg != MPI_GROUP_NULL) MPI_Group_free(&mg);
        }
        mpiGroupVec.clear();

        if (groupComm != MPI_COMM_NULL && groupComm != MPI_COMM_WORLD) {
            MPI_Comm_free(&groupComm);
            groupComm = MPI_COMM_NULL;
        }
        if (worldGroup != MPI_GROUP_NULL) {
            MPI_Group_free(&worldGroup);
            worldGroup = MPI_GROUP_NULL;
        }
        
        MPI_Finalize();
    }
    catch (...) { }
}

/** create groups for parallel run of modeling task. */
void MpiExec::createGroups(int i_groupSize, int i_groupCount)
{
    try {
        // if all processes in one group the exit
        if (i_groupCount <= 1 || i_groupSize <= 0 || i_groupSize >= worldCommSize) return;

        lock_guard<recursive_mutex> lck(msgMutex);

        // get global group as base for all other sub-groups
        int mpiRet = MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

        // groups data
        vector<int> ranks;          // process ranks for group
        ranks.push_back(0);         // all groups include world root
        bool isSelf = false;        // if true this is current process group
        int nSize = 0;
        MPI_Comm mComm = MPI_COMM_NULL;

        int nProc = 1;
        do {
            ranks.push_back(nProc);
            if (nProc == worldRank) isSelf = true;

            // if this is end of current group or last group
            if (++nSize >= i_groupSize || nProc >= worldCommSize - 1) {

                // create group
                MPI_Group mGrp = MPI_GROUP_NULL;

                mpiRet = MPI_Group_incl(worldGroup, (int)ranks.size(), ranks.data(), &mGrp);
                if (mpiRet != MPI_SUCCESS || mGrp == MPI_GROUP_NULL) throw MpiException(mpiRet, worldRank);

                mpiGroupVec.push_back(mGrp);

                // create commumicator
                mpiRet = MPI_Comm_create(MPI_COMM_WORLD, mGrp, &mComm);
                if (mpiRet != MPI_SUCCESS || (isSelf && mComm == MPI_COMM_NULL)) throw MpiException(mpiRet, worldRank);

                mpiCommVec.push_back(mComm);

                if (isSelf || isRoot()) groupComm = mComm;  // group communicator, use last for root process

                // clear group data for next group
                ranks.clear();
                ranks.push_back(0);
                isSelf = false;
                nSize = 0;
                mComm = MPI_COMM_NULL;
            }
        }
        while (++nProc < worldCommSize && (int)mpiCommVec.size() < i_groupCount);

        // validate results: new group communicator expected to be created to all "active" child processes
        if (worldRank <= i_groupSize * i_groupCount) {

            if (groupComm == MPI_COMM_NULL || groupComm == MPI_COMM_WORLD) throw MsgException("Invalid group communicator");

            // get process rank in the group
            mpiRet = MPI_Comm_rank(groupComm, &group_rank);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);
        }
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw MsgException(ex.what());
    }
}

/**
* return group communicator by one-based group number or worldwide communicator.
*
* @param[in] i_groupOne     if zero then worldwide else one-based group number
*/
MPI_Comm MpiExec::commByGroupOne(int i_groupOne)
{
    if (i_groupOne < 0 || (i_groupOne > (int)mpiCommVec.size() && mpiCommVec.size() != 0)) throw MsgException("Invalid modeling group number: %d", i_groupOne);
        
    return (i_groupOne > 0 && mpiCommVec.size() != 0) ? mpiCommVec[i_groupOne - 1] : MPI_COMM_WORLD;
}

/**
* broadcast value array from root to all other processes.
*
* @param[in]     i_groupOne  if zero then worldwide else one-based group number
* @param[in]     i_type      value type
* @param[in]     i_size      size of array
* @param[in,out] io_valueArr value array to send or output buffer to receive
*/
void MpiExec::bcast(int i_groupOne, const type_info & i_type, long long i_size, void * io_valueArr)
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        // select communicator: group or worldwide
        MPI_Comm mComm = commByGroupOne(i_groupOne);

        // if this is a sender then send it by broadcast
        if (isRoot()) {

            // send size of array
            if (i_size <= 0 || i_size >= INT_MAX) 
                throw MsgException("Invalid size of array to broadcast: %d", i_size);

            int sendSize = (int)i_size;
            int mpiRet = MPI_Bcast(&sendSize, 1, MPI_INT, rootRank, mComm);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

            // send value array
            mpiRet = MPI_Bcast(io_valueArr, sendSize, MpiPacked::toMpiType(i_type), rootRank, mComm);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);
        }
        else {  // receive the data

            // receive size of array
            int recvSize = 0;
            int mpiRet = MPI_Bcast(&recvSize, 1, MPI_INT, rootRank, mComm);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);
            if (recvSize <= 0 || recvSize != i_size) 
                throw MsgException("Invalid size of array broadcasted: %d, expected: %d", recvSize, i_size);

            // receive value array
            mpiRet = MPI_Bcast(io_valueArr, recvSize, MpiPacked::toMpiType(i_type), rootRank, mComm);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);
        }
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw MsgException(ex.what());
    }
}

/**
* broadcast vector of db rows from root to all other processes.
*
* @param[in]     i_groupOne  if zero then worldwide else one-based group number
* @param[in,out] io_rowVec   vector of db rows to send or vector to push back received db rows
* @param[in]     i_adapter   adapter to pack and unpack db rows
*/
void MpiExec::bcastPacked(int i_groupOne, IRowBaseVec & io_rowVec, const IPackedAdapter & i_adapter)
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        // select communicator: group or worldwide
        MPI_Comm mComm = commByGroupOne(i_groupOne);

        // if this is a sender then pack db rows and send it by broadcast
        if (isRoot()) {

            // pack db rows
            vector<char> packedData = i_adapter.pack(io_rowVec);

            // send size of packed data
            if (packedData.size() <= 0 || packedData.size() >= INT_MAX) 
                throw MsgException("Invalid size of data to broadcast: %d", packedData.size());

            int packedSize = (int)packedData.size();
            int mpiRet = MPI_Bcast(&packedSize, 1, MPI_INT, rootRank, mComm);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

            // send packed db rows
            mpiRet = MPI_Bcast(packedData.data(), packedSize, MPI_PACKED, rootRank, mComm);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);
        }
        else {  // receive and unpack db rows

            // receive size of packed data
            int packedSize = 0;
            int mpiRet = MPI_Bcast(&packedSize, 1, MPI_INT, rootRank, mComm);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);
            if (packedSize <= 0 || packedSize >= INT_MAX) throw MsgException("Invalid size of data broadcasted: %d, ", packedSize);

            // receive packed db rows
            unique_ptr<char[]> recvPack(new char[packedSize]);

            mpiRet = MPI_Bcast(recvPack.get(), packedSize, MPI_PACKED, rootRank, mComm);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

            // unpack received db rows
            i_adapter.unpackTo(packedSize, recvPack.get(), io_rowVec);
        }
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw MsgException(ex.what());
    }
}

#endif  // OM_MSG_MPI

