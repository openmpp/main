/**
 * @file
 * OpenM++: message passing library public interface
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef OM_MSG_H
#define OM_MSG_H

#include <string>
using namespace std;

#include "libopenm/omCommon.h"
#include "libopenm/common/omHelper.h"
#include "libopenm/db/dbCommon.h"

namespace openm
{
    /** messaging library default error message: "unknown messaging error" */
    extern const char msgUnknownErrorMessage[];   

    /** messaging library exception */
    typedef OpenmException<4000, msgUnknownErrorMessage> MsgException;

    /** tag to identify message content */
    enum class MsgTag
    {
        /** reserved and should not be used */
        unused = 0,

        /** lang_lst db rows */
        langLst,

        /** lang_word db rows */
        langWord,

        /** model_dic db rows */
        modelDic,

        /** model_dic_txt db rows */
        modelDicTxt,

        /** run_lst db rows */
        runLst,

        /** run_txt db rows */
        runTxt,

        /** profile_lst db rows */
        profileLst,

        /** model_dic db rows */
        profileOption,

        /** run_option db rows */
        runOption,

        /** type_dic db rows */
        typeDic,

        /** type_dic_txt db rows */
        typeDicTxt,

        /** type_enum_lst db rows */
        typeEnumLst,

        /** type_enum_txt db rows */
        typeEnumTxt,

        /** parameter_dic db rows */
        parameterDic,

        /** parameter_dic_txt db rows */
        parameterDicTxt,

        /** parameter_run_txt db rows */
        parameterRunTxt,

        /** parameter_dims db rows */
        parameterDims,

        /** workset_lst db rows */
        worksetLst,

        /** workset_txt db rows */
        worksetTxt,

        /** workset_parameter db rows */
        worksetParameter,

        /** workset_parameter_txt db rows */
        worksetParameterTxt,

        /** table_dic db rows */
        tableDic,

        /** table_dic_txt db rows */
        tableDicTxt,

        /** table_dims db rows */
        tableDims,

        /** table_dims_txt db rows */
        tableDimsTxt,

        /** table_acc db rows */
        tableAcc,

        /** table_acc_txt db rows */
        tableAccTxt,

        /** table_unit db rows */
        tableUnit,

        /** table_unit_txt db rows */
        tableUnitTxt,

        /** group_lst db rows */
        groupLst,

        /** group_txt db rows */
        groupTxt,

        /** group_pc db rows */
        groupPc,

        /** input parameter */
        inParam,

        /** output table subsample base */
        outSubsampleBase = 128
    };

    /** public interface to pack and unpack rows of metadata db-table */
    struct IPackedAdapter
    {
    public:
        virtual ~IPackedAdapter(void) throw() { }

        /**
         * pack adapter factory: create new adapter for metadata table db rows.
         *
         * @param[in] i_msgTag  tag to identify message content (parameter or output data)
         */
        static IPackedAdapter * create(MsgTag i_msgTag);

        /** return message tag */
        virtual MsgTag tag(void) const throw() = 0;

        /**
         * pack vector of db rows into char vector.
         *
         * @param[in] i_rowVec   vector of db rows to pack
         */
        virtual const vector<char> pack(const IRowBaseVec & i_rowVec) const = 0;

        /**
         * unpack from char[] into vector of rows.
         *
         * @param[in]     i_packSize   size of array to unpack
         * @param[in]     i_packedData char array to unpack
         * @param[in,out] io_rowVec    vector to push back received db rows
         */
        virtual void unpackTo(int i_packSize, void * i_packedData, IRowBaseVec & io_rowVec) const = 0;
    };

    /** public interface for message passing */
    class IMsgExec
    {
    public:
        /** cleanup message passing resources. */
        virtual ~IMsgExec(void) throw() = 0;

        /**
         * create new message passing interface.
         *
         * @param[in] argc  main argc to pass to MPI_Init
         * @param[in] argv  main argv to pass to MPI_Init
         */
        static IMsgExec * create(int argc, char **argv);

        /** root process rank, always zero */
        static const int rootRank = 0;

        /** if root process then true (root process rank == 0) */
        bool isRoot(void) const throw() { return rank() == rootRank; }

        /** return current process rank. */
        virtual int rank(void) const throw() = 0;

        /** return total number of processes in world communicator. */
        virtual int worldSize(void) const throw() = 0;

        /** wait until all processes come to the barrier. */
        virtual void waitOnBarrier(void) const = 0;

        /**
         * broadcast value array from sender to all other processes.
         *
         * @param[in]     i_sendFrom  sender proccess rank
         * @param[in]     i_type      type of value array
         * @param[in]     i_size      size of value array
         * @param[in,out] io_valueArr value array to send or output buffer to receive
         */
        virtual void bcast(int i_sendFrom, const type_info & i_type, long long i_size, void * io_valueArr) = 0;

        /**
         * broadcast vector of db rows from sender to all other processes.
         *
         * @param[in]     i_sendFrom  sender proccess rank
         * @param[in,out] io_rowVec   vector of db rows to send or vector to push back received db rows
         * @param[in]     i_adapter   adapter to pack and unpack db rows
         */
        virtual void bcastPacked(int i_sendFrom, IRowBaseVec & io_rowVec, const IPackedAdapter & i_adapter) = 0;

        /**
         * start non-blocking send of value array to i_sendTo process.
         *
         * @param[in] i_sendTo    receiver proccess rank
         * @param[in] i_msgTag    tag to identify message content (parameter or output data)
         * @param[in] i_type      type of value array
         * @param[in] i_size      size of value array
         * @param[in] i_valueArr  value array to send (should not be deallocated until send compled)
         */
        virtual void startSend(int i_sendTo, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * i_valueArr) = 0;

        /**
         * pack and start non-blocking send of vector of db rows to i_sendTo process.
         *
         * @param[in] i_sendTo   receiver proccess rank
         * @param[in] i_rowVec   vector of db rows to send
         * @param[in] i_adapter  adapter to pack db rows
         */
        virtual void startSendPacked(int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter) = 0;

        /**
         * initiate non-blocking recveive of value array into io_valueArr.
         *
         * @param[in]     i_recvFrom  sender proccess rank
         * @param[in]     i_msgTag    tag to identify message content (parameter or output data)
         * @param[in]     i_type      type of value array
         * @param[in]     i_size      size of value array
         * @param[in,out] io_valueArr allocated buffer to recieve value array
         */
        virtual void startRecv(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr) = 0;

        /**
         * initiate non-blocking recveive of vector of db rows into io_rowVec.
         *
         * @param[in]     i_recvFrom      sender proccess rank
         * @param[in,out] io_resultRowVec vector to push back received db rows
         * @param[in]     i_adapter       adapter to unpack db rows
         */
        virtual void startRecvPacked(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter) = 0;

        /**
         * try to non-blocking receive value array, return true if completed.
         *
         * @param[in]     i_recvFrom  sender proccess rank
         * @param[in]     i_msgTag    tag to identify message content (parameter or output data)
         * @param[in]     i_type      type of value array
         * @param[in]     i_size      size of value array
         * @param[in,out] io_valueArr allocated buffer to recieve value array
         */
        virtual bool tryReceive(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, long long i_size, void * io_valueArr) const = 0;

        /**
         * try to non-blocking receive and unpack vector of db rows, return true if completed.
         *
         * @param[in]     i_recvFrom      sender proccess rank
         * @param[in,out] io_resultRowVec vector to push back received db rows
         * @param[in]     i_adapter       adapter to unpack db rows
         */
        virtual bool tryReceive(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter) const = 0;

        /** wait for all non-blocking send to be completed. */
        virtual void waitSendAll(void) = 0;

        /** wait for all non-blocking receive to be completed. */
        virtual void waitRecvAll(void) = 0;
    };

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
         * @param[in] i_valueArr value array to send (should not be deallocated until send compled)
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
        virtual ~IMsgRecvArray(void) throw()= 0;

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

#define OM_SEND_SLEEP_TIME  29L     /* msec, send completion test sleep interval */
#define OM_RECV_SLEEP_TIME  31L     /* msec, receive probe sleep interval */

#endif  // OM_MSG_H
