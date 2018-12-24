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
* @param[in,out] argc     main argc to pass to MPI_Init
* @param[in,out] argv     main argv to pass to MPI_Init
* @param[in] IFinalState  final model run state interface
*/
MpiExec::MpiExec(int & argc, char ** & argv, IFinalState * i_final) :
    MsgExecBase(i_final),
    isCleanExit(true),
    worldGroup(MPI_GROUP_NULL), 
    groupComm(MPI_COMM_WORLD)
{
    // cleanup resources on failure
    exit_guard<MpiExec> onExit(this, &MpiExec::cleanup);

    // initialize MPI
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        int provided = MPI_THREAD_SINGLE;
        int mpiRet = MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);     // multiple only because most of implementations provided only that level
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

    isCleanExit = false;    // do MPI_Abort on exit unless this flag is set by main at process shutdown
    onExit.hold();          // initialization completed OK
}

/** cleanup message passing resources by MPI_Finalize. */
MpiExec::~MpiExec(void) noexcept
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);
        cleanup();
    }
    catch (...) { }
}

// Cleanup MPI resources
void MpiExec::cleanup(void) noexcept
{
    try {
        if (!isCleanExit) {
            if (isRoot()) MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            isCleanExit = true;
        }

        for (MPI_Comm & mc : mpiCommVec) {
            if (mc != MPI_COMM_NULL && mc != MPI_COMM_WORLD) MPI_Comm_free(&mc);
        }
        mpiCommVec.clear();
        
        for (MPI_Group & mg : mpiGroupVec) { 
            if (mg != MPI_GROUP_NULL) MPI_Group_free(&mg);
        }
        mpiGroupVec.clear();

        if (worldGroup != MPI_GROUP_NULL) {
            MPI_Group_free(&worldGroup);
            worldGroup = MPI_GROUP_NULL;
        }
        
        MPI_Finalize();
    }
    catch (...) { }
}

/** set clean exit flag for normal shutdown messaging else abort MPI. */
void MpiExec::setCleanExit(bool i_isClean)
{
    lock_guard<recursive_mutex> lck(msgMutex);
    isCleanExit = i_isClean;
}

/** create groups for parallel run of modeling task. */
void MpiExec::createGroups(int i_groupSize, int i_groupCount)
{
    try {
        // if all processes in one group the exit
        if (i_groupCount <= 1 || i_groupSize <= 0 || i_groupSize >= worldCommSize) return;

        lock_guard<recursive_mutex> lck(msgMutex);

        // groups data
        vector<int> ranks;          // process ranks for group
        ranks.push_back(0);         // all groups include world root
        bool isSelf = false;        // if true this is current process group
        int nSize = 0;
        int mpiRet = MPI_SUCCESS;
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

/** wait until MPI request completed.
*
* @param[in]     i_pollTime  dealy in millisends between test MPI request status
* @param[in,out] io_request  MPI request to wait
*/
void MpiExec::waitRequest(int i_pollTime, MPI_Request & io_request) const {

    if (io_request == MPI_REQUEST_NULL) return;     // no active request

    long nErrExchange = 1 + OM_WAIT_SLEEP_TIME / OM_ACTIVE_SLEEP_TIME;
    int isDone = 0;
    do {
        int mpiRet = MPI_Test(&io_request, &isDone, MPI_STATUS_IGNORE);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

        if (!isDone) {
            this_thread::sleep_for(chrono::milliseconds(i_pollTime));
        }
        if (theFinal != nullptr) {
            if (theFinal->isError() && --nErrExchange <= 0) break;  // stop wait attempts if model status is error
        }
    } while (!isDone);
}

/**
 * broadcast value from root to all other processes.
 *
 * @param[in]     i_groupOne  if zero then worldwide else one-based group number
 * @param[in]     i_type      value type
 * @param[in,out] io_value    value to send or output value to receive
 */
void MpiExec::bcastValue(int i_groupOne, const type_info & i_type, void * io_value) {
    try {
        if (io_value == nullptr) throw MsgException("Invalid (null pointer) to value for broadcasting");
        if (i_type == typeid(string)) throw MsgException("Invalid value type to broadcast (string)");

        lock_guard<recursive_mutex> lck(msgMutex);

        // select communicator: group or worldwide
        MPI_Comm mComm = commByGroupOne(i_groupOne);

        // send it by async broadcast
        MPI_Request mpiRq = MPI_REQUEST_NULL;
        int mpiRet = MPI_Ibcast(io_value, 1, MpiPacked::toMpiType(i_type), rootRank, mComm, &mpiRq);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

        // wait until broadcast completed
        waitRequest((isRoot() ? OM_SEND_SLEEP_TIME : OM_SEND_SLEEP_TIME), mpiRq);
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

/** send broadcast value array from root to all other processes.
*
* @param[in]     i_groupOne  if zero then worldwide else one-based group number
* @param[in]     i_type      value type
* @param[in]     i_size      size of array (row count)
* @param[in,out] io_valueArr value array to send or output buffer to receive
*/
void MpiExec::bcastSend(int i_groupOne, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    try {
        if (io_valueArr == nullptr) throw MsgException("Invalid (null) value array to broadcast");
        if (i_size <= 0 || i_size >= INT_MAX) throw MsgException("Invalid size of array to broadcast: %zu", i_size);

        lock_guard<recursive_mutex> lck(msgMutex);

        // select communicator: group or worldwide
        MPI_Comm mComm = commByGroupOne(i_groupOne);

        // send it by async broadcast
        MPI_Request sizeRq = MPI_REQUEST_NULL;
        MPI_Request dataRq = MPI_REQUEST_NULL;
        unique_ptr<char> packedData;

        if (i_type != typeid(string)) {     // if source is not a string array then send it without packing

            // send size of array
            int sendSize = (int)i_size;
            int mpiRet = MPI_Ibcast(&sendSize, 1, MPI_INT, rootRank, mComm, &sizeRq);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

            // send value array
            mpiRet = MPI_Ibcast(io_valueArr, sendSize, MpiPacked::toMpiType(i_type), rootRank, mComm, &dataRq);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);
        }
        else {  // pack string array

            const string * srcArr = reinterpret_cast<const string *>(io_valueArr);
            int sendSize = MpiPacked::packedSize(i_size, srcArr);
            if (sendSize <= 0 || sendSize >= INT_MAX) throw MsgException("Invalid size of data to broadcast: %d", sendSize);

            packedData = MpiPacked::packArray(i_size, srcArr);

            // send byte size of packed string array
            int mpiRet = MPI_Ibcast(&sendSize, 1, MPI_INT, rootRank, mComm, &sizeRq);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

            // send packed string array
            mpiRet = MPI_Ibcast(packedData.get(), sendSize, MPI_PACKED, rootRank, mComm, &dataRq);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);
        }

        // wait until broadcast send completed
        waitRequest(OM_SEND_SLEEP_TIME, sizeRq);
        waitRequest(OM_SEND_SLEEP_TIME, dataRq);
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

/** receive broadcasted value array from root process.
*
* @param[in]     i_groupOne  if zero then worldwide else one-based group number
* @param[in]     i_type      value type
* @param[in]     i_size      size of array (row count)
* @param[in,out] io_valueArr value array to send or output buffer to receive
*/
void MpiExec::bcastReceive(int i_groupOne, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    try {
        if (io_valueArr == nullptr) throw MsgException("Invalid (null) value array to broadcast");
        if (i_size <= 0 || i_size >= INT_MAX) throw MsgException("Invalid size of array to broadcast: %zu", i_size);

        lock_guard<recursive_mutex> lck(msgMutex);

        // select communicator: group or worldwide
        MPI_Comm mComm = commByGroupOne(i_groupOne);

        // receive size of data
        MPI_Request mpiRq = MPI_REQUEST_NULL;
        int recvSize = 0;
        int mpiRet = MPI_Ibcast(&recvSize, 1, MPI_INT, rootRank, mComm, &mpiRq);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

        waitRequest(OM_RECV_SLEEP_TIME, mpiRq);     // wait until receive size completed

        // receive array data: array of primitive type or packed array of strings
        if (i_type != typeid(string)) {

            if (recvSize <= 0 || (size_t)recvSize != i_size) throw MsgException("Invalid size of array broadcasted: %d, expected: %zu", recvSize, i_size);

            // receive value array
            mpiRet = MPI_Ibcast(io_valueArr, recvSize, MpiPacked::toMpiType(i_type), rootRank, mComm, &mpiRq);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

            waitRequest(OM_RECV_SLEEP_TIME, mpiRq);     // wait intil receive data completed
        }
        else {  // packed string array

            if (recvSize <= 0 || recvSize >= INT_MAX) throw MsgException("Invalid size of data broadcasted: %d, ", recvSize);

            // receive packed data
            unique_ptr<char> recvPack(new char[recvSize]);

            mpiRet = MPI_Ibcast(recvPack.get(), recvSize, MPI_PACKED, rootRank, mComm, &mpiRq);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

            waitRequest(OM_RECV_SLEEP_TIME, mpiRq);     // wait intil receive data completed

            // unpack received array of strings
            string * recvData = reinterpret_cast<string *>(io_valueArr);
            MpiPacked::unpackArray(recvSize, recvPack.get(), i_size, recvData);
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

/** send broadcast vector of db rows from root to all other processes. 
*
* @param[in]     i_groupOne  if zero then worldwide else one-based group number
* @param[in,out] io_rowVec   vector of db rows to send or vector to push back received db rows
* @param[in]     i_adapter   adapter to pack and unpack db rows
*/
void MpiExec::bcastSendPacked(int i_groupOne, IRowBaseVec & io_rowVec, const IPackedAdapter & i_adapter)
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        // select communicator: group or worldwide
        MPI_Comm mComm = commByGroupOne(i_groupOne);

        // pack db rows
        vector<char> packedData = i_adapter.pack(io_rowVec);

        if (packedData.size() <= 0 || packedData.size() >= INT_MAX)
            throw MsgException("Invalid size of data to broadcast: %zu", packedData.size());

        // send size of packed data
        MPI_Request sizeRq = MPI_REQUEST_NULL;
        int packedSize = (int)packedData.size();
        int mpiRet = MPI_Ibcast(&packedSize, 1, MPI_INT, rootRank, mComm, &sizeRq);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

        // send packed db rows
        MPI_Request dataRq = MPI_REQUEST_NULL;
        mpiRet = MPI_Ibcast(packedData.data(), packedSize, MPI_PACKED, rootRank, mComm, &dataRq);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

        // wait until broadcast send completed
        waitRequest(OM_RECV_SLEEP_TIME, sizeRq);
        waitRequest(OM_RECV_SLEEP_TIME, dataRq);
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

/** receive broadcasted vector of db rows from root. 
*
* @param[in]     i_groupOne  if zero then worldwide else one-based group number
* @param[in,out] io_rowVec   vector of db rows to send or vector to push back received db rows
* @param[in]     i_adapter   adapter to pack and unpack db rows
*/
void MpiExec::bcastReceivePacked(int i_groupOne, IRowBaseVec & io_rowVec, const IPackedAdapter & i_adapter)
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        // select communicator: group or worldwide
        MPI_Comm mComm = commByGroupOne(i_groupOne);

        // receive size of packed data
        MPI_Request mpiRq = MPI_REQUEST_NULL;
        int packedSize = 0;
        int mpiRet = MPI_Ibcast(&packedSize, 1, MPI_INT, rootRank, mComm, &mpiRq);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

        waitRequest(OM_RECV_SLEEP_TIME, mpiRq);     // wait until size received

        if (packedSize <= 0 || packedSize >= INT_MAX) throw MsgException("Invalid size of data broadcasted: %d, ", packedSize);

        // receive packed db rows
        unique_ptr<char> recvPack(new char[packedSize]);

        mpiRet = MPI_Ibcast(recvPack.get(), packedSize, MPI_PACKED, rootRank, mComm, &mpiRq);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, worldRank);

        waitRequest(OM_RECV_SLEEP_TIME, mpiRq);     // wait until data received

        // unpack received db rows
        i_adapter.unpackTo(packedSize, recvPack.get(), io_rowVec);
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

