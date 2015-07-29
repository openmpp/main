/**
 * @file
 * OpenM++: message passing library base class
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MSG_EXEC_BASE_H
#define MSG_EXEC_BASE_H

#include <string>
#include <vector>
using namespace std;

#include "msg.h"

namespace openm
{
    /** message passing base class */
    class MsgExecBase
    {
    public:
        /** cleanup message passing resources. */
        virtual ~MsgExecBase(void) throw() { };

        /** return current process rank. */
        int rank(void) const throw() { return selfRank; }

        /** return total number of processes in world communicator. */
        int worldSize(void) const throw() { return worldCommSize; }

        /** start non-blocking send of value array to i_sendTo process. */
        void startSend(int i_sendTo, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * i_valueArr);

        /** pack and start non-blocking send of vector of db rows to i_sendTo process. */
        void startSendPacked(int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter);

        /** initiate non-blocking recveive of value array into io_valueArr. */
        void startRecv(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr);

        /** initiate non-blocking recveive of vector of db rows into io_rowVec. */
        void startRecvPacked(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter);

        /** try to non-blocking receive value array, return return true if completed. */
        bool tryReceive(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr) const;

        /** try to non-blocking receive and unpack vector of db rows, return return true if completed. */
        bool tryReceive(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter) const;

        /** wait for all non-blocking send to be completed. */
        void waitSendAll(void);

        /** wait for all non-blocking receive to be completed. */
        void waitRecvAll(void);

    protected:
        MsgExecBase(void) : selfRank(0), worldCommSize(0) { }

    protected:
        int selfRank;           // rank of process itself
        int worldCommSize;      // total number of processes in world communicator

        vector<unique_ptr<IMsgSend> > sendVec;     // active send requests vector
        vector<unique_ptr<IMsgRecv> > recvVec;     // active receive requests vector

    private:
        MsgExecBase(const MsgExecBase & i_msgExec);                 // = delete;
        MsgExecBase & operator=(const MsgExecBase & i_msgExec);     // = delete;
    };
}

#endif  // MSG_EXEC_BASE_H
