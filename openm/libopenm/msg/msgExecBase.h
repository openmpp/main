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
#include <mutex>
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

        /** return total number of processes in MPI world communicator. */
        int worldSize(void) const;

        /** return current process rank. */
        int rank(void) const;

        /** return rank in modeling group. */
        int groupRank(void) const;

        /** start non-blocking send of value array to i_sendTo process. */
        void startSend(int i_sendTo, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * i_valueArr);

        /** pack and start non-blocking send of vector of db rows to i_sendTo process. */
        void startSendPacked(int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter);

        /** try to non-blocking receive value array, return return true if completed. */
        bool tryReceive(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr) const;

        /** try to non-blocking receive and unpack vector of db rows, return return true if completed. */
        bool tryReceive(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter) const;

        /** wait for non-blocking send to be completed.
         *
         * @param[in] i_isOnce  if true then check send list only once else wait until all requests completed
         */
        void waitSendAll(bool i_isOnce);

    protected:
        MsgExecBase(void) : worldCommSize(1), worldRank(0), group_rank(0) { }

    protected:
        int worldCommSize;      // total number of processes in world communicator
        int worldRank;          // rank of process in world communicator
        int group_rank;         // rank of process in group communicator

        list<unique_ptr<IMsgSend> > sendLst;    // list of active send requests

    private:
        MsgExecBase(const MsgExecBase & i_msgExec) = delete;
        MsgExecBase & operator=(const MsgExecBase & i_msgExec) = delete;
    };

    /** mutex to lock messaging operations */
    extern recursive_mutex msgMutex;
}

#endif  // MSG_EXEC_BASE_H
