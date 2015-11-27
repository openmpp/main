/**
 * @file
 * OpenM++: message passing library common classes and interfaces
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MSG_MPI_COMMON_H
#define MSG_MPI_COMMON_H

#include <climits>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
using namespace std;

#include "helper.h"
#include "msg.h"

namespace openm
{
    /** public interface for message sender */
    class IMsgSend
    {
    public:
        /** cleanup message sender resources. */
        virtual ~IMsgSend(void) throw() = 0;

        /** check is send completed. */
        virtual bool isCompleted(void) = 0;
    };

    /** public interface to send value array */
    class IMsgSendArray : public IMsgSend
    {
    public:
        /** cleanup message sender resources. */
        virtual ~IMsgSendArray(void) throw() = 0;

        /**
        * create new value array sender.
        *
        * @param[in] i_selfRank sender proccess rank (current process rank)
        * @param[in] i_sendTo   receiver proccess rank
        * @param[in] i_msgTag   tag to identify message content (parameter or output data)
        * @param[in] i_type     type of value array
        * @param[in] i_size     size of value array
        * @param[in] i_valueArr value array to send
        */
        static IMsgSendArray * create(
            int i_selfRank, int i_sendTo, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * i_valueArr
            );
    };

    /** public interface to send packed data. */
    class IMsgSendPacked : public IMsgSend
    {
    public:
        /** cleanup message sender resources. */
        virtual ~IMsgSendPacked(void) throw() = 0;

        /**
        * create new sender for packed data.
        *
        * @param[in] i_selfRank sender proccess rank (current process rank)
        * @param[in] i_sendTo   receiver proccess rank
        * @param[in] i_rowVec   vector of db rows to send
        * @param[in] i_adapter  adapter to pack db rows
        */
        static IMsgSendPacked * create(
            int i_selfRank, int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter
            );
    };

    /** public interface for message receiver */
    class IMsgRecv
    {
    public:
        /** cleanup message receiver resources. */
        virtual ~IMsgRecv(void) throw() = 0;

        /** try to receive the data, return return true if received. */
        virtual bool tryReceive(void) = 0;
    };

    /** public interface to receive value array */
    class IMsgRecvArray : public IMsgRecv
    {
    public:
        /** cleanup value array receiver resources. */
        virtual ~IMsgRecvArray(void) throw() = 0;

        /**
        * create new receiver for value array.
        *
        * @param[in]     i_selfRank  receiver (current process rank)
        * @param[in]     i_recvFrom  sender proccess rank
        * @param[in]     i_msgTag    tag to identify message content (parameter or output data)
        * @param[in]     i_type      type of value array
        * @param[in]     i_size      size of value array
        * @param[in,out] io_valueArr allocated buffer to recieve value array
        */
        static IMsgRecvArray * create(
            int i_selfRank, int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr
            );
    };

    /** public interface to receive packed data */
    class IMsgRecvPacked : public IMsgRecv
    {
    public:
        /** cleanup message receiver resources. */
        virtual ~IMsgRecvPacked(void) throw() = 0;

        /**
        * create new receiver for packed data.
        *
        * @param[in]     i_selfRank      receiver (current process rank)
        * @param[in]     i_recvFrom      sender proccess rank
        * @param[in,out] io_resultRowVec vector to push back received db rows
        * @param[in]     i_adapter       adapter to unpack db rows
        */
        static IMsgRecvPacked * create(
            int i_selfRank, int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter
            );
    };
}

// define OM_MSG_MPI   to use message passing library based on MPI
// define OM_MSG_EMPTY to use empty version message passing library
//      empty version does nothing but don't require MPI installed

#ifdef OM_MSG_EMPTY
    #include "msgEmpty.h"
#elif OM_MSG_MPI
    #include <mpi.h>
    #include "msgMpi.h"
#else       // not defined any of OM_MSG_*
    #error No message passing providers defined
#endif      // OM_MSG_MPI

#endif  // MSG_MPI_COMMON_H
