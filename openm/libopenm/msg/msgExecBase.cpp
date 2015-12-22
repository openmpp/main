/**
 * @file
 * OpenM++: message passing library base class
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

using namespace std;

#include "msgCommon.h"
using namespace openm;

/**
* start non-blocking send of value array to i_sendTo process.
*
* @param[in] i_sendTo    receiver proccess rank
* @param[in] i_msgTag    tag to identify message content (parameter or output data)
* @param[in] i_type      type of value array
* @param[in] i_size      size of value array
* @param[in] i_valueArr  value array to send
*/
void MsgExecBase::startSend(int i_sendTo, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * i_valueArr)
{
    try {
        sendVec.push_back(unique_ptr<IMsgSend>(
            IMsgSendArray::create(selfRank, i_sendTo, i_msgTag, i_type, i_size, i_valueArr)
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
        sendVec.push_back(unique_ptr<IMsgSend>(
            IMsgSendPacked::create(selfRank, i_sendTo, i_rowVec, i_adapter)
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
* @param[in]     i_type      type of value array
* @param[in]     i_size      size of value array
* @param[in,out] io_valueArr allocated buffer to recieve value array
*/
void MsgExecBase::startRecv(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr)
{
    try {
        recvVec.push_back(unique_ptr<IMsgRecv>(
            IMsgRecvArray::create(selfRank, i_recvFrom, i_msgTag, i_type, i_size, io_valueArr)
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
        recvVec.push_back(unique_ptr<IMsgRecv>(
            IMsgRecvPacked::create(selfRank, i_recvFrom, io_resultRowVec, i_adapter)
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
* @param[in]     i_type      type of value array
* @param[in]     i_size      size of value array
* @param[in,out] io_valueArr allocated buffer to recieve value array
*/
bool MsgExecBase::tryReceive(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr) const
{
    try {
        unique_ptr<IMsgRecv> rcv(IMsgRecvArray::create(selfRank, i_recvFrom, i_msgTag, i_type, i_size, io_valueArr));
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
        unique_ptr<IMsgRecv> rcv(IMsgRecvPacked::create(selfRank, i_recvFrom, io_resultRowVec, i_adapter));
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
        bool isAllDone = true;
        do {
            isAllDone = true;

            // check for any active send request
            for (size_t nSend = 0; nSend < sendVec.size(); nSend++) {
                bool isDone = sendVec[nSend].get()->isCompleted();
                isAllDone = isAllDone && isDone;
            }

            // sleep if any outstanding request exist
            if (!isAllDone) sleepMilli(OM_SEND_SLEEP_TIME);
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
        bool isAllDone = true;
        do {
            isAllDone = true;

            // check for any active receive request
            for (size_t nRecv = 0; nRecv < recvVec.size(); nRecv++) {
                bool isDone = recvVec[nRecv].get()->tryReceive();
                isAllDone = isAllDone && isDone;
            }

            // sleep if any outstanding request exist
            if (!isAllDone) sleepMilli(OM_RECV_SLEEP_TIME);
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

