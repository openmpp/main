/**
 * @file
 * OpenM++: message passing library common classes and interfaces
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MSG_MPI_H
#define MSG_MPI_H

#include "msg.h"

using namespace std;

namespace openm
{
    /** messaging library MPI exception */
    class MpiException : public MsgException
    {
    public:
        /** create messaging library exception based on MPI return code */
        MpiException(int i_mpiReturn) : MsgException(makeErrorMsg(i_mpiReturn).c_str()), mpiRet(i_mpiReturn), mpiRank(0)
        { }

        /** create messaging library exception based on MPI return code and process rank */
        MpiException(int i_mpiReturn, int i_rank) : MsgException(makeErrorMsg(i_mpiReturn, i_rank).c_str()), mpiRet(i_mpiReturn), mpiRank(i_rank)
        { }

        /** get MPI return code */
        int mpiReturn(void) const { return mpiRet; }

        /** get MPI process rank */
        int rank(void) const { return mpiRank; }

    private:
        int mpiRet;     // return code from MPI call
        int mpiRank;    // MPI process rank

        /** build error message by MPI error code */
        static const string makeErrorMsg(int i_mpiReturn);

        /** build error message by MPI error code and rank */
        static const string makeErrorMsg(int i_mpiReturn, int i_rank);
    };
}

#include "msgMpiPacked.h"
#include "msgMpiExec.h"
#include "msgMpiSend.h"
#include "msgMpiRecv.h"

#endif  // MSG_MPI_H
