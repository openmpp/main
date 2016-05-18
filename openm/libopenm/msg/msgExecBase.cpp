/**
 * @file
 * OpenM++: message passing library base class
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

using namespace std;

#include "msgCommon.h"
using namespace openm;

// mutex to lock messaging operations
recursive_mutex openm::msgMutex;

/** return total number of processes in MPI world communicator. */
int MsgExecBase::worldSize(void) const
{ 
    lock_guard<recursive_mutex> lck(msgMutex);
    return worldCommSize;
}

/** return current process rank. */
int MsgExecBase::rank(void) const
{ 
    lock_guard<recursive_mutex> lck(msgMutex);
    return worldRank; 
}

/** return rank in modeling group. */
int MsgExecBase::groupRank(void) const
{ 
    lock_guard<recursive_mutex> lck(msgMutex);
    return group_rank; 
}

/**
* start non-blocking send of value array to i_sendTo process.
*
* @param[in] i_sendTo    receiver proccess rank
* @param[in] i_msgTag    tag to identify message content (parameter or output data)
* @param[in] i_type      value type
* @param[in] i_size      size of array
* @param[in] i_valueArr  value array to send
*/
void MsgExecBase::startSend(int i_sendTo, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * i_valueArr)
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        sendVec.push_back(unique_ptr<IMsgSend>(
            IMsgSendArray::create(rank(), i_sendTo, i_msgTag, i_type, i_size, i_valueArr)
            ));
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
* pack and start non-blocking send of vector of db rows to i_sendTo process.
*
* @param[in] i_sendTo   receiver proccess rank
* @param[in] i_rowVec   vector of db rows to send
* @param[in] i_adapter  adapter to pack db rows
*/
void MsgExecBase::startSendPacked(int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter)
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        sendVec.push_back(unique_ptr<IMsgSend>(
            IMsgSendPacked::create(rank(), i_sendTo, i_rowVec, i_adapter)
            ));
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
* initiate non-blocking recveive of value array into io_valueArr.
*
* @param[in]     i_recvFrom  sender proccess rank
* @param[in]     i_msgTag    tag to identify message content (parameter or output data)
* @param[in]     i_type      value type
* @param[in]     i_size      size of array
* @param[in,out] io_valueArr allocated buffer to recieve value array
*/
void MsgExecBase::startRecv(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr)
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        recvVec.push_back(unique_ptr<IMsgRecv>(
            IMsgRecvArray::create(rank(), i_recvFrom, i_msgTag, i_type, i_size, io_valueArr)
            ));
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
* initiate non-blocking recveive of vector of db rows into io_rowVec.
*
* @param[in]     i_recvFrom      sender proccess rank
* @param[in,out] io_resultRowVec vector to push back received db rows
* @param[in]     i_adapter       adapter to unpack db rows
*/
void MsgExecBase::startRecvPacked(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter)
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        recvVec.push_back(unique_ptr<IMsgRecv>(
            IMsgRecvPacked::create(rank(), i_recvFrom, io_resultRowVec, i_adapter)
            ));
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
* try to non-blocking receive value array, return true if completed.
*
* @param[in]     i_recvFrom  sender proccess rank
* @param[in]     i_msgTag    tag to identify message content (parameter or output data)
* @param[in]     i_type      value type
* @param[in]     i_size      size of array
* @param[in,out] io_valueArr allocated buffer to recieve value array
*/
bool MsgExecBase::tryReceive(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr) const
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        unique_ptr<IMsgRecv> rcv(IMsgRecvArray::create(rank(), i_recvFrom, i_msgTag, i_type, i_size, io_valueArr));
        return rcv->tryReceive();
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
* try to non-blocking receive and unpack vector of db rows, return true if completed.
*
* @param[in]     i_recvFrom      sender proccess rank
* @param[in,out] io_resultRowVec vector to push back received db rows
* @param[in]     i_adapter       adapter to unpack db rows
*/
bool MsgExecBase::tryReceive(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter) const
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        unique_ptr<IMsgRecv> rcv(IMsgRecvPacked::create(rank(), i_recvFrom, io_resultRowVec, i_adapter));
        return rcv->tryReceive();
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

/** wait for all non-blocking send to be completed. */
void MsgExecBase::waitSendAll(void)
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        bool isAllDone = true;
        do {
            isAllDone = true;

            // check for any active send request
            for (size_t nSend = 0; nSend < sendVec.size(); nSend++) {
                bool isDone = sendVec[nSend].get()->isCompleted();
                isAllDone = isAllDone && isDone;
            }

            // sleep if any outstanding request exist
            if (!isAllDone) this_thread::sleep_for(chrono::milliseconds(OM_SEND_SLEEP_TIME));
        }
        while (!isAllDone);
        
        sendVec.clear();    // all request completed, clear the list
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

/** wait for all non-blocking receive to be completed. */
void MsgExecBase::waitRecvAll(void)
{
    try {
        lock_guard<recursive_mutex> lck(msgMutex);

        bool isAllDone = true;
        do {
            isAllDone = true;

            // check for any active receive request
            for (size_t nRecv = 0; nRecv < recvVec.size(); nRecv++) {
                bool isDone = recvVec[nRecv].get()->tryReceive();
                isAllDone = isAllDone && isDone;
            }

            // sleep if any outstanding request exist
            if (!isAllDone) this_thread::sleep_for(chrono::milliseconds(OM_RECV_SLEEP_TIME));
        }
        while (!isAllDone);
        
        recvVec.clear();    // all request completed, clear the list
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

