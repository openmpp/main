/**
 * @file
 * OpenM++: message passing library main class for MPI-based implementation
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MSG_MPI_EXEC_H
#define MSG_MPI_EXEC_H

using namespace std;

#include "msgCommon.h"
#include "msgExecBase.h"
#include "msgMpiPacked.h"
#include "msgMpiSend.h"
#include "msgMpiRecv.h"

namespace openm
{
    /** message passing library main class for MPI-based implementation */
    class MpiExec : public IMsgExec, public MsgExecBase
    {
    public:
        /** create message passing interface by MPI_Init. */
        MpiExec(int & argc, char ** & argv);

        /** cleanup message passing resources by MPI_Finalize. */
        ~MpiExec(void) throw();

        /** return total number of processes in MPI world communicator. */
        int worldSize(void) const throw() override { return MsgExecBase::worldSize(); }

        /** return current process MPI rank. */
        int rank(void) const throw() override { return MsgExecBase::rank(); }

        /** return rank in modeling group. */
        int groupRank(void) const throw() override { return MsgExecBase::groupRank(); }

        /** create groups for parallel run of modeling task. */
        void createGroups(int i_groupSize, int i_groupCount) override;

        /** broadcast value array from root to all other processes. */
        void bcast(int i_groupOne, const type_info & i_type, size_t i_size, void * io_valueArr) override;

        /** broadcast vector of db rows from root to all other processes. */
        void bcastPacked(int i_groupOne, IRowBaseVec & io_rowVec, const IPackedAdapter & i_adapter) override;

        /** start non-blocking send of value array to i_sendTo process. */
        void startSend(int i_sendTo, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * i_valueArr) override
        { MsgExecBase::startSend(i_sendTo, i_msgTag, i_type, i_size, i_valueArr); }

        /** pack and start non-blocking send of vector of db rows to i_sendTo process. */
        void startSendPacked(int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter) override
        { MsgExecBase::startSendPacked(i_sendTo, i_rowVec, i_adapter); }

        /** initiate non-blocking recveive of value array into io_valueArr. */
        void startRecv(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr) override
        { MsgExecBase::startRecv(i_recvFrom, i_msgTag, i_type, i_size, io_valueArr); }

        /** initiate non-blocking recveive of vector of db rows into io_rowVec. */
        void startRecvPacked(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter) override
        { MsgExecBase::startRecvPacked(i_recvFrom, io_resultRowVec, i_adapter); }

        /** try to non-blocking receive and unpack vector of db rows, return true if completed. */
        bool tryReceive(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter) const override
        { return MsgExecBase::tryReceive(i_recvFrom, io_resultRowVec, i_adapter); }

        /** try to non-blocking receive value array, return true if completed. */
        bool tryReceive(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr) const override
        { return MsgExecBase::tryReceive(i_recvFrom, i_msgTag, i_type, i_size, io_valueArr); }

        /** wait for all non-blocking send to be completed. */
        void waitSendAll(void) override { MsgExecBase::waitSendAll(); }

        /** wait for all non-blocking receive to be completed. */
        void waitRecvAll(void) override { MsgExecBase::waitRecvAll(); }

    private:
        MPI_Group worldGroup;           // MPI world global group
        MPI_Comm groupComm;             // modeling group communicator for current process
        vector<MPI_Group> mpiGroupVec;  // handles of modeling groups
        vector<MPI_Comm> mpiCommVec;    // handles of communicators

        // cleanup MPI resources
        void cleanup(void) throw();

        /** return group communicator by one-based group number or worldwide communicator. */
        MPI_Comm commByGroupOne(int i_groupOne);

    private:
        MpiExec(const MpiExec & i_exec) = delete;
        MpiExec & operator=(const MpiExec & i_exec) = delete;
    };
}

#endif  // MSG_MPI_EXEC_H
