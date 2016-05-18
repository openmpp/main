/**
 * @file
 * OpenM++ message passing library: sender classes for MPI-based implementation
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifdef OM_MSG_MPI

using namespace std;

#include "msgCommon.h"
using namespace openm;

/**
* create non-blocking MPI sender.
*
* @param[in] i_selfRank sender proccess rank (current process rank)
* @param[in] i_sendTo   receiver proccess rank
* @param[in] i_msgTag   tag to identify message content
*/
MpiSendBase::MpiSendBase(int i_selfRank, int i_sendTo, MsgTag i_msgTag) :
    msgTag((int)i_msgTag),
    selfRank(i_selfRank),
    sendToRank(i_sendTo),
    mpiRq(MPI_REQUEST_NULL)
{ }

/** cleanup message sender resources by MPI_Request_free. */
MpiSendBase::~MpiSendBase(void) throw() 
{ 
    try { 
        cleanup(); 
    } 
    catch (...) { } 
}

/** check is send completed by MPI_Test. */
bool MpiSendBase::isCompleted(void)
{
    if (mpiRq == MPI_REQUEST_NULL) return true;     // no active request

    int isDone = 0;
    int mpiRet = MPI_Test(&mpiRq, &isDone, MPI_STATUS_IGNORE);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);

    return isDone != 0;
}

// cleanup MPI send resources
void MpiSendBase::cleanup(void) throw()
{
    try {
        // request must be released by MPI_Test prior the cleanup
        if (mpiRq != MPI_REQUEST_NULL) {
            theLog->logFormatted("MPI error: send %d unfinished from %d to %d", msgTag, selfRank, sendToRank);

            // release request
            MPI_Request_free(&mpiRq);
            mpiRq = MPI_REQUEST_NULL;
        }
    }
    catch (...) { }
}

/**
* create non-blocking MPI sender for value array and call MPI_Isend.
* 
* @param[in] i_selfRank sender proccess rank (current process rank)
* @param[in] i_sendTo   receiver proccess rank
* @param[in] i_msgTag   tag to identify message content (parameter or output data)
* @param[in] i_type     value type
* @param[in] i_size     size of array
* @param[in] i_valueArr value array to send
 */
MpiSendArray::MpiSendArray(
    int i_selfRank, int i_sendTo, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * i_valueArr
    ) :
    MpiSendBase(i_selfRank, i_sendTo, i_msgTag)
{
    // cleanup resources on failure
    exit_guard<MpiSendArray> onExit(this, &MpiSendArray::cleanup);

    if (i_size <= 0 || i_size >= INT_MAX) throw MsgException("Invalid size of array to send: %d", i_size);

    int sendSize = (int)i_size;
    sendValueArr.reset(static_cast<char *>(i_valueArr));

    int mpiRet = MPI_Isend(sendValueArr.get(), sendSize, MpiPacked::toMpiType(i_type), sendToRank, msgTag, MPI_COMM_WORLD, &mpiRq);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);

    onExit.hold();  // initialization completed OK
}

/**
* create non-blocking MPI sender pack the data and call MPI_Isend.
*
* @param[in] i_selfRank sender proccess rank (current process rank)
* @param[in] i_sendTo   receiver proccess rank
* @param[in] i_rowVec   vector of db rows to send
* @param[in] i_adapter  adapter to pack db rows
*/
MpiSendPacked::MpiSendPacked(
    int i_selfRank, int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter
    ) :
    MpiSendBase(i_selfRank, i_sendTo, i_adapter.tag())
{
    // cleanup resources on failure
    exit_guard<MpiSendPacked> onExit(this, &MpiSendPacked::cleanup);

    // start MPI non-blocking send
    packedData = i_adapter.pack(i_rowVec);

    int mpiRet = MPI_Isend(packedData.data(), (int)packedData.size(), MPI_PACKED, sendToRank, msgTag, MPI_COMM_WORLD, &mpiRq);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);

    onExit.hold();  // initialization completed OK
}

#endif  // OM_MSG_MPI
