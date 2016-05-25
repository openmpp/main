/**
 * @file
 * OpenM++ message passing library: receiver class for MPI-based implementation
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MSG_MPI_RECV_H
#define MSG_MPI_RECV_H

using namespace std;

#include "msgCommon.h"

namespace openm
{
    /** non-blocking receive of value array */
    class MpiRecvArray : public IMsgRecvArray
    {
    public:
        /**
         * create receiver for value array. 
         *
         * @param[in]     i_selfRank  receiver (current process MPI rank)
         * @param[in]     i_recvFrom  sender proccess MPI rank
         * @param[in]     i_msgTag    tag to identify message content (parameter or output data)
         * @param[in]     i_type      value type
         * @param[in]     i_size      size of array
         * @param[in,out] io_valueArr allocated buffer to recieve value array
         */
        MpiRecvArray(
            int i_selfRank, int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr
            ) :
            msgTag((int)i_msgTag),
            isReceived(false),
            selfRank(i_selfRank),
            recvFromRank(i_recvFrom),
            resultSize(i_size),
            resultValueArr(io_valueArr),
            mpiType(MpiPacked::toMpiType(i_type))
        { }

        ~MpiRecvArray(void) throw() { }

        /** try to non-blocking receive value array, return true if completed. */
        bool tryReceive(void);

    private:
        int msgTag;             // MPI message tag
        bool isReceived;        // if true the data received and unpacked
        int selfRank;           // destination (ourself) MPI process rank
        int recvFromRank;       // source MPI process rank
        size_t resultSize;      // expected size of array
        void * resultValueArr;  // received data
        MPI_Datatype mpiType;   // MPI type to receive

    private:
        MpiRecvArray(const MpiRecvArray & i_recv) = delete;
        MpiRecvArray & operator=(const MpiRecvArray & i_recv) = delete;
    };

    /** non-blocking receive of packed data */
    class MpiRecvPacked : public IMsgRecvPacked
    {
    public:
        /**
         * create receiver for packed data.
         *
         * @param[in]     i_selfRank      receiver (current process MPI rank)
         * @param[in]     i_recvFrom      sender proccess MPI rank
         * @param[in,out] io_resultRowVec vector to push back received db rows
         * @param[in]     i_adapter       adapter to unpack db rows
         */
        MpiRecvPacked(
            int i_selfRank, int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter
            ) :
            isReceived(false),
            selfRank(i_selfRank),
            recvFromRank(i_recvFrom),
            resultRowVec(io_resultRowVec),
            packAdp(i_adapter)
        { }

        ~MpiRecvPacked(void) throw() { }

        /** try to receive and unpack the data, return return true if completed. */
        bool tryReceive(void);

    private:
        bool isReceived;                // if true the data received and unpacked
        int selfRank;                   // destination (ourself) MPI process rank
        int recvFromRank;               // source MPI process rank
        IRowBaseVec & resultRowVec;     // received data
        const IPackedAdapter & packAdp; // adapter to unpack received data

    private:
        MpiRecvPacked(const MpiRecvPacked & i_recv) = delete;
        MpiRecvPacked & operator=(const MpiRecvPacked & i_recv) = delete;
    };
}

#endif  // MSG_MPI_RECV_H
