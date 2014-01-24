/**
 * @file
 * OpenM++: message passing library main class for MPI-based implementation
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifdef OM_MSG_MPI

using namespace std;

#include "msgCommon.h"
using namespace openm;

/**
* create message passing interface by MPI_Init.
*
* @param[in,out] argc  main argc to pass to MPI_Init
* @param[in,out] argv  main argv to pass to MPI_Init
*/
MpiExec::MpiExec(int & argc, char ** & argv) :
    MsgExecBase()
{
    // cleanup resources on failure
    exit_guard<MpiExec> onExit(this, &MpiExec::cleanup);

    // initialize MPI
    try {
        int mpiRet = MPI_Init(&argc, &argv);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

        // return error code instead of aborting process
        mpiRet = MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

        // get our rank and number of models executes
        mpiRet = MPI_Comm_rank(MPI_COMM_WORLD, &selfRank);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

        mpiRet = MPI_Comm_size(MPI_COMM_WORLD, &worldCommSize);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw MsgException(ex.what());
    }

    onExit.hold();  // initialization completed OK
}

/** cleanup message passing resources by MPI_Finalize. */
MpiExec::~MpiExec(void) throw()
{
    try {
        cleanup();
    }
    catch (...) { }
}

// Cleanup MPI resources
void MpiExec::cleanup(void) throw()
{
    try {
        MPI_Finalize();
    }
    catch (...) { }
}

/** wait until all processes come to the barrier. */
void MpiExec::waitOnBarrier(void) const 
{ 
    try {
        int mpiRet = MPI_Barrier(MPI_COMM_WORLD);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw MsgException(ex.what());
    }
}

/**
* broadcast value array from sender to all other processes.
*
* @param[in]     i_sendFrom  sender proccess rank
* @param[in]     i_type      type of value array
* @param[in]     i_size      size of value array
* @param[in,out] io_valueArr value array to send or output buffer to receive
*/
void MpiExec::bcast(int i_sendFrom, const type_info & i_type, long long i_size, void * io_valueArr)
{
    try {
        // if this is a sender then send it by broadcast
        if (selfRank == i_sendFrom) {

            // send size of array
            if (i_size <= 0 || i_size >= INT_MAX) 
                throw MsgException("Invalid size of array to broadcast: %d", i_size);

            int sendSize = (int)i_size;
            int mpiRet = MPI_Bcast(&sendSize, 1, MPI_INT, i_sendFrom, MPI_COMM_WORLD);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);

            // send value array
            mpiRet = MPI_Bcast(io_valueArr, sendSize, MpiPacked::toMpiType(i_type), i_sendFrom, MPI_COMM_WORLD);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);
        }
        else {  // receive the data

            // receive size of array
            int recvSize = 0;
            int mpiRet = MPI_Bcast(&recvSize, 1, MPI_INT, i_sendFrom, MPI_COMM_WORLD);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);
            if (recvSize <= 0 || recvSize != i_size) 
                throw MsgException("Invalid size of array broadcasted: %d, expected: %d", recvSize, i_size);

            // receive value array
            mpiRet = MPI_Bcast(io_valueArr, recvSize, MpiPacked::toMpiType(i_type), i_sendFrom, MPI_COMM_WORLD);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);
        }
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw MsgException(ex.what());
    }
}

/**
* broadcast vector of db rows from sender to all other processes.
*
* @param[in]     i_sendFrom  sender proccess rank
* @param[in,out] io_rowVec   vector of db rows to send or vector to push back received db rows
* @param[in]     i_adapter   adapter to pack and unpack db rows
*/
void MpiExec::bcastPacked(int i_sendFrom, IRowBaseVec & io_rowVec, const IPackedAdapter & i_adapter)
{
    try {
        // if this is a sender then pack db rows and send it by broadcast
        if (selfRank == i_sendFrom) {

            // pack db rows
            vector<char> packedData = i_adapter.pack(io_rowVec);

            // send size of packed data
            if (packedData.size() <= 0 || packedData.size() >= INT_MAX) 
                throw MsgException("Invalid size of data to broadcast: %d", packedData.size());

            int packedSize = (int)packedData.size();
            int mpiRet = MPI_Bcast(&packedSize, 1, MPI_INT, i_sendFrom, MPI_COMM_WORLD);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);

            // send packed db rows
            mpiRet = MPI_Bcast(packedData.data(), packedSize, MPI_PACKED, i_sendFrom, MPI_COMM_WORLD);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);
        }
        else {  // receive and unpack db rows

            // receive size of packed data
            int packedSize = 0;
            int mpiRet = MPI_Bcast(&packedSize, 1, MPI_INT, i_sendFrom, MPI_COMM_WORLD);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);
            if (packedSize <= 0 || packedSize >= INT_MAX) throw MsgException("Invalid size of data broadcasted: %d, ", packedSize);

            // receive packed db rows
            unique_ptr<char[]> recvPack(new char[packedSize]);

            mpiRet = MPI_Bcast(recvPack.get(), packedSize, MPI_PACKED, i_sendFrom, MPI_COMM_WORLD);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet, selfRank);

            // unpack received db rows
            i_adapter.unpackTo(packedSize, recvPack.get(), io_rowVec);
        }
    }
    catch (MsgException & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw;
    }
    catch (exception & ex) {
        theLog->logErr(ex, OM_FILE_LINE);
        throw MsgException(ex.what());
    }
}

#endif  // OM_MSG_MPI

