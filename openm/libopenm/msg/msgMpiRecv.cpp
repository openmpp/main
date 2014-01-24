/**
 * @file
 * OpenM++ message passing library: receiver classes for MPI-based implementation
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifdef OM_MSG_MPI

using namespace std;

#include "msgCommon.h"
using namespace openm;

/** try to non-blocking receive value array, return true if completed. */
bool MpiRecvArray::tryReceive(void)
{
    if (isReceived) return true;    // message already received

    // probe for incoming message
    int isRecv = 0;
    MPI_Status mpiStatus;

    int mpiRet = MPI_Iprobe(recvFromRank, msgTag, MPI_COMM_WORLD, &isRecv, &mpiStatus);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);

    if (isRecv == 0) return false;  // message not received

    // check message size
    int recvSize;
    mpiRet = MPI_Get_count(&mpiStatus, mpiType, &recvSize);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);
    if (recvSize <= 0 || recvSize != resultSize) 
        throw MsgException("Invalid size of array received: %d, expected: %d", recvSize, resultSize);

    // receive the message
    mpiRet = MPI_Recv(resultValueArr, recvSize,  mpiType, recvFromRank, msgTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);

    // receive completed
    isReceived = true;
    return isReceived;
}

/** try to receive and unpack the data, return return true if completed. */
bool MpiRecvPacked::tryReceive(void)
{
    if (isReceived) return true;    // message already received

    // probe for incoming message
    int isRecv = 0;
    MPI_Status mpiStatus;

    int mpiRet = MPI_Iprobe(recvFromRank, (int)packAdp.tag(), MPI_COMM_WORLD, &isRecv, &mpiStatus);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);

    if (isRecv == 0) return false;  // message not received

    // check message size
    int recvSize;
    mpiRet = MPI_Get_count(&mpiStatus, MPI_PACKED, &recvSize);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);
    if (recvSize <= 0 || recvSize >= INT_MAX) throw MsgException("Invalid size of data received: %d", recvSize);

    // receive the message
    unique_ptr<char[]> recvPack(new char[recvSize]);

    mpiRet = MPI_Recv(recvPack.get(), recvSize, MPI_PACKED, recvFromRank, (int)packAdp.tag(), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);

    // unpack received message
    packAdp.unpackTo(recvSize, recvPack.get(), resultRowVec);

    // receive completed
    isReceived = true;
    return isReceived;
}

#endif  // OM_MSG_MPI
