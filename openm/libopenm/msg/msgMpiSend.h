/**
 * @file
 * OpenM++ message passing library: sender class for MPI-based implementation
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MSG_MPI_SEND_H
#define MSG_MPI_SEND_H

using namespace std;

#include "msgCommon.h"
#include "msgMpiPacked.h"

namespace openm
{
    /** base class for non-blocking MPI send */
    class MpiSendBase
    {
    public:
        /** create non-blocking MPI sender. */
        MpiSendBase(int i_selfRank, int i_sendTo, MsgTag i_msgTag);

        /** cleanup message sender resources by MPI_Request_free. */
        virtual ~MpiSendBase(void) noexcept;

        /** check is send completed by MPI_Test. */
        bool isCompleted(void);

    protected:
        int msgTag;         // MPI message tag
        int selfRank;       // source (ourself) MPI process rank
        int sendToRank;     // destination MPI process rank
        MPI_Request mpiRq;  // send request

    protected:
        // cleanup MPI send resources
        void cleanup(void) noexcept;

    private:
        MpiSendBase(const MpiSendBase & i_send) = delete;
        MpiSendBase & operator=(const MpiSendBase & i_send) = delete;
    };

    /** non-blocking MPI send of value array */
    class MpiSendArray : public IMsgSendArray, public MpiSendBase
    {
    public:
        /** create non-blocking MPI sender for value array and call MPI_Isend. */
        MpiSendArray(
            int i_selfRank, int i_sendTo, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * i_valueArr
            );

        /** cleanup message sender resources. */
        ~MpiSendArray(void) noexcept { }

        /** check is send completed by MPI_Test. */
        bool isCompleted(void) { return MpiSendBase::isCompleted(); }

    private:
        unique_ptr<uint8_t> sendValueArr;     // copy of data to send

    private:
        MpiSendArray(const MpiSendArray & i_send) = delete;
        MpiSendArray & operator=(const MpiSendArray & i_send) = delete;
    };

    /** non-blocking MPI send of packed data */
    class MpiSendPacked : public MpiSendBase, public IMsgSendPacked
    {
    public:
        /** create non-blocking MPI sender pack the data and call MPI_Isend. */
        MpiSendPacked(
            int i_selfRank, int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter
            );

        /** cleanup message sender resources. */
        ~MpiSendPacked(void) noexcept { }

        /** check is send completed by MPI_Test. */
        bool isCompleted(void) { return MpiSendBase::isCompleted(); }

    private:
        vector<uint8_t> packedData;   // data to send

    private:
        MpiSendPacked(const MpiSendPacked & i_send) = delete;
        MpiSendPacked & operator=(const MpiSendPacked & i_send) = delete;
    };
}

#endif  // MSG_MPI_SEND_H
