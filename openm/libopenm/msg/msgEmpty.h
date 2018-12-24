/**
 * @file
 * OpenM++: empty implementation of message passing library
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MSG_EMPTY_H
#define MSG_EMPTY_H

using namespace std;

#include "msgExecBase.h"

namespace openm
{
    /** pack and unpack adapter for metadata table db rows (empty implementation). */
    struct EmptyPackedAdapter : public IPackedAdapter
    {
    public:
        /** create pack and unpack adapter for metadata table db rows */
        EmptyPackedAdapter(MsgTag i_tag) : msgTag(i_tag) { }

        /** return message tag */
        MsgTag tag(void) const noexcept override { return msgTag; }

        /** pack vector of db rows into char vector: return empty vector. */
        const vector<char> pack(const IRowBaseVec & /* i_rowVec */) const override
        { 
            lock_guard<recursive_mutex> lck(msgMutex);
            vector<char> packedData;
            return packedData;
        }

        /** unpack from char[] into vector of rows (does nothing). */
        void unpackTo(int /* i_packSize */, void * /* i_packedData */, IRowBaseVec & /* io_rowVec */) const override { }

    private:
        MsgTag msgTag;  // message tag
    };

    /** message passing main class.    
    * 
    * empty implementation of message passing library does nothing \n
    * it does NOT pass any data between sub-value processes but not required MPI to be installed \n
    * it can be used for: \n
    *   model debug and development to quickly test model code with single sub-value \n
    *   or if each sub-value process directly connected to database \n
    */
    class MsgEmptyExec : public IMsgExec, public MsgExecBase
    {
    public:
        /** create new message passing interface (empty constructor) */
        MsgEmptyExec(int & /* argc */, char ** & /* argv */, IFinalState * i_final) : MsgExecBase(i_final) { }

        /** cleanup message passing resources (empty destructor). */
        ~MsgEmptyExec(void) noexcept { }

        /** return total number of processes in world communicator: always = 1. */
        int worldSize(void) const override { return MsgExecBase::worldSize(); }

        /** return current process rank: always = 0. */
        int rank(void) const override { return MsgExecBase::rank(); }

        /** return rank in modeling group: always = 0. */
        int groupRank(void) const noexcept { return MsgExecBase::groupRank(); }

        /** set clean exit flag for normal shutdown of message passing interface. */
        void setCleanExit(bool /*i_isClean*/) override { }

        /** create groups for parallel run of modeling task (does nothing). */
        void createGroups(int /*i_groupSize*/, int /*i_groupCount*/) override { }

        /** broadcast value from root to all other processes (does nothing). */
        void bcastValue(
            int /*i_groupOne*/, const type_info & /*i_type*/, void * /*io_value*/
            ) override { }

        /** send broadcast value array from root to all other processes (does nothing). */
        void bcastSend(
            int /*i_groupOne*/, const type_info & /*i_type*/, size_t /*i_size*/, void * /*io_valueArr*/
            ) override { }

        /** receive broadcasted value array from root process (does nothing). */
        void bcastReceive(
            int /*i_groupOne*/, const type_info & /*i_type*/, size_t /*i_size*/, void * /*io_valueArr*/
            ) override { }

        /** send broadcast vector of db rows from root to all other processes (does nothing). */
        void bcastSendPacked(
            int /*i_groupOne*/, IRowBaseVec & /*io_rowVec*/, const IPackedAdapter & /*i_adapter*/
            ) override { }

        /** receive broadcasted vector of db rows from root (does nothing). */
        void bcastReceivePacked(
            int /*i_groupOne*/, IRowBaseVec & /*io_rowVec*/, const IPackedAdapter & /*i_adapter*/
            ) override { }

        /** start non-blocking send of value array to i_sendTo process. */
        void startSend(int i_sendTo, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * i_valueArr) override
        { MsgExecBase::startSend(i_sendTo, i_msgTag, i_type, i_size, i_valueArr); }

        /** pack and start non-blocking send of vector of db rows to i_sendTo process. */
        void startSendPacked(int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter) override
        { MsgExecBase::startSendPacked(i_sendTo, i_rowVec, i_adapter); }

        /** try to non-blocking receive and unpack vector of db rows, return true if completed. */
        bool tryReceive(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter) const override
        { return MsgExecBase::tryReceive(i_recvFrom, io_resultRowVec, i_adapter); }

        /** try to non-blocking receive value array, return true if completed. */
        bool tryReceive(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr) const override
        { return MsgExecBase::tryReceive(i_recvFrom, i_msgTag, i_type, i_size, io_valueArr); }

        /** wait for non-blocking send to be completed. */
        void waitSendAll(void) override { MsgExecBase::waitSendAll(); }

    private:
        MsgEmptyExec(const MsgEmptyExec & i_exec) = delete;
        MsgEmptyExec & operator=(const MsgEmptyExec & i_exec) = delete;
    };

    /** base class for non-blocking send (empty implementation) */
    class MsgEmptySendBase
    {
    public:
        /** create sender and initiate non-blocking send (does nothing). */
        MsgEmptySendBase(int i_selfRank, int i_sendTo, MsgTag i_msgTag) :
            msgTag((int)i_msgTag),
            selfRank(i_selfRank),
            sendToRank(i_sendTo)
        { }

        /** cleanup message sender resources (does nothing). */
        virtual ~MsgEmptySendBase(void) noexcept { }

    protected:
        int msgTag;         // MPI message tag
        int selfRank;       // source (ourself) MPI process rank
        int sendToRank;     // destination MPI process rank

    private:
        MsgEmptySendBase(const MsgEmptySendBase & i_send) = delete;
        MsgEmptySendBase & operator=(const MsgEmptySendBase & i_send) = delete;
    };

    /** non-blocking send of value array (empty implementation) */
    class MsgEmptySendArray : public IMsgSendArray, public MsgEmptySendBase
    {
    public:
        /** create sender and initiate non-blocking send of value array (does nothing). */
        MsgEmptySendArray(
            int i_selfRank, int i_sendTo, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * i_valueArr
            ) : 
            MsgEmptySendBase(i_selfRank, i_sendTo, i_msgTag),
            valueType(i_type),
            arrSize(i_size),
            valueArr(i_valueArr)
        { }

        /** cleanup message sender resources (does nothing). */
        ~MsgEmptySendArray(void) noexcept { }

        /** return true if send completed (always true). */
        bool isCompleted(void) { return true; }

    private:
        const type_info & valueType;    // type of value in array
        size_t arrSize;                 // size of array
        void * valueArr;                // value array

    private:
        MsgEmptySendArray(const MsgEmptySendArray & i_send) = delete;
        MsgEmptySendArray & operator=(const MsgEmptySendArray & i_send) = delete;
    };

    /** non-blocking send of packed data (empty implementation) */
    class MsgEmptySendPacked : public IMsgSendPacked, public MsgEmptySendBase
    {
    public:
        /** create sender and initiate non-blocking send of packed data (does nothing). */
        MsgEmptySendPacked(
            int i_selfRank, int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter
            ) : 
            MsgEmptySendBase(i_selfRank, i_sendTo, i_adapter.tag()),
            rowVec(i_rowVec),
            packAdp(i_adapter)
        { }

        /** cleanup message sender resources (does nothing). */
        ~MsgEmptySendPacked(void) noexcept { }

        /** return true if send completed (always true). */
        bool isCompleted(void) { return true; }

    private:
        const IRowBaseVec & rowVec;     // vector of db rows
        const IPackedAdapter & packAdp; // adapter to pack db rows data

    private:
        MsgEmptySendPacked(const MsgEmptySendPacked & i_send) = delete;
        MsgEmptySendPacked & operator=(const MsgEmptySendPacked & i_send) = delete;
    };

    /** non-blocking receive of value array (empty implementation) */
    class MsgEmptyRecvArray : public IMsgRecvArray
    {
    public:
        /** create sender and initiate non-blocking receive of value array (does nothing). */
        MsgEmptyRecvArray(
            int i_selfRank, int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr
            ) :
            msgTag((int)i_msgTag),
            selfRank(i_selfRank),
            recvFromRank(i_recvFrom),
            valueType(i_type),
            resultSize(i_size),
            resultValueArr(io_valueArr)
        { }

        /** cleanup message receiver resources (does nothing). */
        ~MsgEmptyRecvArray(void) noexcept { }

        /** try to non-blocking receive value array, return true if completed (always true). */
        bool tryReceive(void) { return true; }

    private:
        int msgTag;                     // message tag
        int selfRank;                   // destination (ourself) process rank
        int recvFromRank;               // source process rank
        const type_info & valueType;    // type of value in array
        size_t resultSize;              // expected size of array
        void * resultValueArr;          // received data

    private:
        MsgEmptyRecvArray(const MsgEmptyRecvArray & i_recv) = delete;
        MsgEmptyRecvArray & operator=(const MsgEmptyRecvArray & i_recv) = delete;
    };

    /** non-blocking receive of packed data (empty implementation) */
    class MsgEmptyRecvPacked : public IMsgRecvPacked
    {
    public:
        /** create sender and initiate non-blocking receive of packed data (does nothing). */
        MsgEmptyRecvPacked(
            int i_selfRank, int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter
            ) :
            selfRank(i_selfRank),
            recvFromRank(i_recvFrom),
            resultRowVec(io_resultRowVec),
            packAdp(i_adapter)
        { }

        /** cleanup message receiver resources (does nothing). */
        ~MsgEmptyRecvPacked(void) noexcept { }

        /** try to non-blocking receive and unpack the data, return true if completed (always true). */
        bool tryReceive(void) { return true; }

    private:
        int selfRank;                   // destination (ourself) process rank
        int recvFromRank;               // source process rank
        IRowBaseVec & resultRowVec;     // received data
        const IPackedAdapter & packAdp; // adapter to unpack received data

    private:
        MsgEmptyRecvPacked(const MsgEmptyRecvPacked & i_recv) = delete;
        MsgEmptyRecvPacked & operator=(const MsgEmptyRecvPacked & i_recv) = delete;
    };
}

#endif  // MSG_EMPTY_H
