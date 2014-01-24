/**
 * @file
 * OpenM++: message passing library main class for MPI-based implementation
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

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

        /** return current process MPI rank. */
        int rank(void) const throw() { return MsgExecBase::rank(); }

        /** return total number of processes in MPI world communicator. */
        int worldSize(void) const throw() { return MsgExecBase::worldSize(); }

        /** wait until all processes come to the barrier. */
        void waitOnBarrier(void) const;

        /** broadcast value array from sender to all other processes. */
        void bcast(int i_sendFrom, const type_info & i_type, long long i_size, void * io_valueArr);

        /** broadcast vector of db rows from sender to all other processes. */
        void bcastPacked(int i_sendFrom, IRowBaseVec & io_rowVec, const IPackedAdapter & i_adapter);

        /** start non-blocking send of value array to i_sendTo process. */
        void startSend(int i_sendTo, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * i_valueArr)
        {
             return MsgExecBase::startSend(i_sendTo, i_msgTag, i_type, i_size, i_valueArr);
        }

        /** pack and start non-blocking send of vector of db rows to i_sendTo process. */
        void startSendPacked(int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter)
        {
             return MsgExecBase::startSendPacked(i_sendTo, i_rowVec, i_adapter);
        }

        /** initiate non-blocking recveive of value array into io_valueArr. */
        void startRecv(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr)
        {
             return MsgExecBase::startRecv(i_recvFrom, i_msgTag, i_type, i_size, io_valueArr);
        }

        /** initiate non-blocking recveive of vector of db rows into io_rowVec. */
        void startRecvPacked(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter)
        {
             return MsgExecBase::startRecvPacked(i_recvFrom, io_resultRowVec, i_adapter);
        }

        /** try to non-blocking receive value array, return true if completed. */
        bool tryReceive(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter) const
        {
             return MsgExecBase::tryReceive(i_recvFrom, io_resultRowVec, i_adapter);
        }

        /** try to non-blocking receive and unpack vector of db rows, return true if completed. */
        bool tryReceive(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr) const
        {
             return MsgExecBase::tryReceive(i_recvFrom, i_msgTag, i_type, i_size, io_valueArr);
        }

        /** wait for all non-blocking send to be completed. */
        void waitSendAll(void) { return MsgExecBase::waitSendAll(); }

        /** wait for all non-blocking receive to be completed. */
        void waitRecvAll(void) { return MsgExecBase::waitRecvAll(); }

    private:
        // cleanup MPI resources
        void cleanup(void) throw();

    private:
        MpiExec(const MpiExec & i_exec);                // = delete;
        MpiExec & operator=(const MpiExec & i_exec);    // = delete;
    };
}

#endif  // MSG_MPI_EXEC_H
