// OpenM++: message passing library common classes
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "msgCommon.h"

using namespace std;
using namespace openm;

// messaging library default error message
const char openm::msgUnknownErrorMessage[] = "unknown messaging error";

// cleanup message passing resources.
IMsgExec::~IMsgExec(void) noexcept { }

// cleanup message sender resources.
IMsgSend::~IMsgSend(void) noexcept { }
IMsgSendArray::~IMsgSendArray(void) noexcept { }
IMsgSendPacked::~IMsgSendPacked(void) noexcept { }

// cleanup message receiver resources.
IMsgRecv::~IMsgRecv(void) noexcept { }
IMsgRecvArray::~IMsgRecvArray(void) noexcept { }
IMsgRecvPacked::~IMsgRecvPacked(void) noexcept { }

#ifdef OM_MSG_MPI

// create new message passing interface.
IMsgExec * IMsgExec::create(int argc, char **argv, IFinalState * i_final)
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return new MpiExec(argc, argv, i_final);
}

// return byte size to pack source array.
size_t IPackedAdapter::packedSize(const type_info & i_type, size_t i_size)
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return MpiPacked::packedSize(i_type, i_size);
}

// return new allocated and packed copy of source array.
unique_ptr<uint8_t[]> IPackedAdapter::packArray(const type_info & i_type, size_t i_size, void * i_valueArr)
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return MpiPacked::packArray(i_type, i_size, i_valueArr);
}

// create new value array sender.
IMsgSendArray * IMsgSendArray::create(
    int i_selfRank, int i_sendTo, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * i_valueArr
    )
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return new MpiSendArray(i_selfRank, i_sendTo, i_msgTag, i_type, i_size, i_valueArr);
}

// create new sender for packed data.
IMsgSendPacked * IMsgSendPacked::create(
    int i_selfRank, int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter
    )
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return new MpiSendPacked(i_selfRank, i_sendTo, i_rowVec, i_adapter);
}

// create new receiver for value array.
IMsgRecvArray * IMsgRecvArray::create(
    int i_selfRank, int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr
    )
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return new MpiRecvArray(i_selfRank, i_recvFrom, i_msgTag, i_type, i_size, io_valueArr);
}

// create new receiver for packed data.
IMsgRecvPacked * IMsgRecvPacked::create(
    int i_selfRank, int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter
    )
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return new MpiRecvPacked(i_selfRank, i_recvFrom, io_resultRowVec, i_adapter);
}

// build error message by MPI error code and rank
const string MpiException::makeErrorMsg(int i_mpiReturn, int i_rank) 
{
    try {
        return string("[") + to_string(i_rank) + string("] ") + makeErrorMsg(i_mpiReturn);
    }
    catch (...) {
        return "MPI call failed";
    }
}

// build error message by MPI error code
const string MpiException::makeErrorMsg(int i_mpiReturn) 
{
    try {
        if (i_mpiReturn == MPI_SUCCESS) return "MPI_SUCCESS";    // not an error

        int msgLen = 0;
        char errMsg[MPI_MAX_ERROR_STRING + 1];

        if (MPI_Error_string(i_mpiReturn, errMsg, &msgLen) != MPI_SUCCESS) 
            return "MPI error: " + to_string(i_mpiReturn);

        if (msgLen < 0 || msgLen >= MPI_MAX_ERROR_STRING) 
            return "MPI error: " + to_string(i_mpiReturn);

        errMsg[msgLen] = '\0';
        return 
            to_string(i_mpiReturn) + ": " + errMsg;
    }
    catch (...) {
        return "MPI call failed";
    }
}

#endif // OM_MSG_MPI

#ifdef OM_MSG_EMPTY

// create new pack and unpack adapter for metadata table db rows
IPackedAdapter * IPackedAdapter::create(MsgTag i_msgTag) 
{ 
    lock_guard<recursive_mutex> lck(msgMutex);
    return new EmptyPackedAdapter(i_msgTag);
}

/** return byte size to pack source array: return always zero */
size_t IPackedAdapter::packedSize(const type_info & /*i_type*/, size_t /*i_size*/)
{
    return 0;
}

/** return pack'ed copy of source array: return always empty. */
unique_ptr<uint8_t[]> IPackedAdapter::packArray(const type_info & /*i_type*/, size_t /*i_size*/, void * /*i_valueArr*/)
{
    unique_ptr<uint8_t[]> packedData(new uint8_t[0]);
    return packedData;
}

// create new message passing interface.
IMsgExec * IMsgExec::create(int argc, char **argv, IFinalState * i_final)
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return new MsgEmptyExec(argc, argv, i_final);
}

// create new value array sender.
IMsgSendArray * IMsgSendArray::create(
    int i_selfRank, int i_sendTo, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * i_valueArr
    )
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return new MsgEmptySendArray(i_selfRank, i_sendTo, i_msgTag, i_type, i_size, i_valueArr);
}

// create new sender for packed data.
IMsgSendPacked * IMsgSendPacked::create(
    int i_selfRank, int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter
    )
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return new MsgEmptySendPacked(i_selfRank, i_sendTo, i_rowVec, i_adapter);
}

// create new receiver for value array.
IMsgRecvArray * IMsgRecvArray::create(
    int i_selfRank, int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr
    )
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return new MsgEmptyRecvArray(i_selfRank, i_recvFrom, i_msgTag, i_type, i_size, io_valueArr);
}

// create new receiver for packed data.
IMsgRecvPacked * IMsgRecvPacked::create(
    int i_selfRank, int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter
    )
{
    lock_guard<recursive_mutex> lck(msgMutex);
    return new MsgEmptyRecvPacked(i_selfRank, i_recvFrom, io_resultRowVec, i_adapter);
}

#endif  // OM_MSG_EMPTY
