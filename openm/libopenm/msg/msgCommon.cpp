// OpenM++: message passing library common classes
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

using namespace std;

#include "msgCommon.h"
using namespace openm;

// messaging library default error message
const char openm::msgUnknownErrorMessage[] = "unknown messaging error";

// cleanup message passing resources.
IMsgExec::~IMsgExec(void) throw() { }

// cleanup message sender resources.
IMsgSend::~IMsgSend(void) throw() { }
IMsgSendArray::~IMsgSendArray(void) throw() { }
IMsgSendPacked::~IMsgSendPacked(void) throw() { }

// cleanup message receiver resources.
IMsgRecv::~IMsgRecv(void) throw() { }
IMsgRecvArray::~IMsgRecvArray(void) throw() { }
IMsgRecvPacked::~IMsgRecvPacked(void) throw() { }

#ifdef OM_MSG_MPI

// create new message passing interface.
IMsgExec * IMsgExec::create(int argc, char **argv)
{
    lock_guard<recursive_mutex> lck(rtMutex);
    return new MpiExec(argc, argv);
}

// create new value array sender.
IMsgSendArray * IMsgSendArray::create(
    int i_selfRank, int i_sendTo, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * i_valueArr
    )
{
    return new MpiSendArray(i_selfRank, i_sendTo, i_msgTag, i_type, i_size, i_valueArr);
}

// create new sender for packed data.
IMsgSendPacked * IMsgSendPacked::create(
    int i_selfRank, int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter
    )
{
    return new MpiSendPacked(i_selfRank, i_sendTo, i_rowVec, i_adapter);
}

// create new receiver for value array.
IMsgRecvArray * IMsgRecvArray::create(
    int i_selfRank, int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr
    )
{
    return new MpiRecvArray(i_selfRank, i_recvFrom, i_msgTag, i_type, i_size, io_valueArr);
}

// create new receiver for packed data.
IMsgRecvPacked * IMsgRecvPacked::create(
    int i_selfRank, int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter
    )
{
    return new MpiRecvPacked(i_selfRank, i_recvFrom, io_resultRowVec, i_adapter);
}

// build error message by MPI error code and rank
const string MpiException::makeErrorMsg(int i_mpiReturn, int i_rank) 
{
    try {
        return "[" + to_string(i_rank) + "] " + makeErrorMsg(i_mpiReturn);
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
    lock_guard<recursive_mutex> lck(rtMutex);
    return new EmptyPackedAdapter(i_msgTag);
}

// create new message passing interface.
IMsgExec * IMsgExec::create(int argc, char **argv)
{
    lock_guard<recursive_mutex> lck(rtMutex);
    return new MsgEmptyExec(argc, argv);
}

// create new value array sender.
IMsgSendArray * IMsgSendArray::create(
    int i_selfRank, int i_sendTo, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * i_valueArr
    )
{
    return new MsgEmptySendArray(i_selfRank, i_sendTo, i_msgTag, i_type, i_size, i_valueArr);
}

// create new sender for packed data.
IMsgSendPacked * IMsgSendPacked::create(
    int i_selfRank, int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter
    )
{
    return new MsgEmptySendPacked(i_selfRank, i_sendTo, i_rowVec, i_adapter);
}

// create new receiver for value array.
IMsgRecvArray * IMsgRecvArray::create(
    int i_selfRank, int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr
    )
{
    return new MsgEmptyRecvArray(i_selfRank, i_recvFrom, i_msgTag, i_type, i_size, io_valueArr);
}

// create new receiver for packed data.
IMsgRecvPacked * IMsgRecvPacked::create(
    int i_selfRank, int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter
    )
{
    return new MsgEmptyRecvPacked(i_selfRank, i_recvFrom, io_resultRowVec, i_adapter);
}

#endif  // OM_MSG_EMPTY
