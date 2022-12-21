/**
 * @file
 * OpenM++: message passing library public interface
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_MSG_H
#define OM_MSG_H

#include <string>
using namespace std;

#include "libopenm/omLog.h"
#include "libopenm/omModelRunState.h"
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

        /** initial message at process start */
        initial,

        /** modeling job status */
        statusUpdate,

        /** microdata row count for each entity */
        microdataSize,

        /** microdata entity rows */
        microdata,

        /** lang_lst db rows */
        langLst = 32,

        /** lang_word db rows */
        langWord,

        /** model_dic db rows */
        modelDic,

        /** type_dic db rows */
        typeDic,

        /** type_enum_lst db rows */
        typeEnumLst,

        /** parameter_dic db rows */
        parameterDic,

        /** parameter_dims db rows */
        parameterDims,

        /** table_dic db rows */
        tableDic,

        /** table_dims db rows */
        tableDims,

        /** table_acc db rows */
        tableAcc,

        /** table_expr db rows */
        tableExpr,

        /** entity_dic db rows */
        entityDic,

        /** entity_attr db rows */
        entityAttr,

        /** run_option db rows */
        runOption,

        /** generic (code,value) db rows */
        codeValue,

        /** input parameter */
        //inParam,

        /** output table sub-value base */
        outSubValueBase = 128
    };

    /** public interface to pack and unpack rows of metadata db-table */
    struct IPackedAdapter
    {
    public:
        virtual ~IPackedAdapter(void) noexcept { }

        /**
         * pack adapter factory: create new adapter for metadata table db rows.
         *
         * @param[in] i_msgTag  tag to identify message content (parameter or output data)
         */
        static IPackedAdapter * create(MsgTag i_msgTag);

        /** return message tag */
        virtual MsgTag tag(void) const noexcept = 0;

        /**
         * pack vector of db rows into byte vector.
         *
         * @param[in] i_rowVec   vector of db rows to pack
         */
        virtual const vector<uint8_t> pack(const IRowBaseVec & i_rowVec) const = 0;

        /**
         * unpack from byte[] into vector of rows.
         *
         * @param[in]     i_packSize   size of array to unpack
         * @param[in]     i_packedData byte array to unpack
         * @param[in,out] io_rowVec    vector to push back received db rows
         */
        virtual void unpackTo(int i_packSize, void * i_packedData, IRowBaseVec & io_rowVec) const = 0;

        /** return byte size to pack source array. */
        static size_t packedSize(const type_info & i_type, size_t i_size);

        /** return new allocated and packed copy of source array. */
        static unique_ptr<uint8_t[]> packArray(const type_info & i_type, size_t i_size, void * i_valueArr);
    };

    /** public interface for message passing */
    class IMsgExec
    {
    public:
        /** cleanup message passing resources. */
        virtual ~IMsgExec(void) noexcept = 0;

        /**
         * create new message passing interface.
         *
         * @param[in] argc         main argc to pass to MPI_Init
         * @param[in] argv         main argv to pass to MPI_Init
         * @param[in] i_final  final model run state interface
         */
        static IMsgExec * create(int argc, char **argv, IFinalState * i_final);

        /** root process rank, always zero */
        static const int rootRank = 0;

        /** if root process then true (root process rank == 0) */
        bool isRoot(void) const { return rank() == rootRank; }

        /** return total number of processes in world communicator. */
        virtual int worldSize(void) const = 0;

        /** return current process rank. */
        virtual int rank(void) const = 0;

        /** return rank in modeling group. */
        virtual int groupRank(void) const = 0;

        /** set clean exit flag for normal shutdown of message passing interface. */
        virtual void setCleanExit(bool i_isClean = false) = 0;

        /** create groups for parallel run of modeling task.
         *
         * @param[in] i_groupSize   number of processes in each group
         * @param[in] i_groupCount  number of groups
        */
        virtual void createGroups(int i_groupSize, int i_groupCount) = 0;

        /**
         * broadcast value from root to all other processes.
         *
         * @param[in]     i_groupOne  if zero then worldwide else one-based group number
         * @param[in]     i_type      value type
         * @param[in,out] io_value    value to send or output value to receive
         */
        virtual void bcastValue(int i_groupOne, const type_info & i_type, void * io_value) = 0;

        /**
         * broadcast integer value from root to all other processes.
         *
         * @param[in]     i_groupOne  if zero then worldwide else one-based group number
         * @param[in,out] io_value    integer value to send or output value to receive
         */
        void bcastInt(int i_groupOne, void * io_value) { bcastValue(i_groupOne, typeid(int), io_value); }

        /** send broadcast value array from root to all other processes.
         *
         * @param[in]     i_groupOne  if zero then worldwide else one-based group number
         * @param[in]     i_type      value type
         * @param[in]     i_size      size of array
         * @param[in,out] io_valueArr value array to send or output buffer to receive
         */
        virtual void bcastSend(int i_groupOne, const type_info & i_type, size_t i_size, void * io_valueArr) = 0;

        /**
         * receive broadcasted value array from root process.
         *
         * @param[in]     i_groupOne  if zero then worldwide else one-based group number
         * @param[in]     i_type      value type
         * @param[in]     i_size      size of array
         * @param[in,out] io_valueArr value array to send or output buffer to receive
         */
        virtual void bcastReceive(int i_groupOne, const type_info & i_type, size_t i_size, void * io_valueArr) = 0;


        /** send broadcast vector of db rows from root to all other processes.
         *
         * @param[in]     i_groupOne  if zero then worldwide else one-based group number
         * @param[in,out] io_rowVec   vector of db rows to send or vector to push back received db rows
         * @param[in]     i_adapter   adapter to pack and unpack db rows
         */
        virtual void bcastSendPacked(int i_groupOne, IRowBaseVec & io_rowVec, const IPackedAdapter & i_adapter) = 0;

        /** receive broadcasted vector of db rows from root.
         *
         * @param[in]     i_groupOne  if zero then worldwide else one-based group number
         * @param[in,out] io_rowVec   vector of db rows to send or vector to push back received db rows
         * @param[in]     i_adapter   adapter to pack and unpack db rows
         */
        virtual void bcastReceivePacked(int i_groupOne, IRowBaseVec & io_rowVec, const IPackedAdapter & i_adapter) = 0;

        /**
         * start non-blocking send of value array to i_sendTo process.
         *
         * @param[in] i_sendTo    receiver proccess rank
         * @param[in] i_msgTag    tag to identify message content (parameter or output data)
         * @param[in] i_type      value type
         * @param[in] i_size      size of array
         * @param[in] i_valueArr  value array to send
         */
        virtual void startSend(int i_sendTo, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * i_valueArr) = 0;

        /**
         * pack and start non-blocking send of vector of db rows to i_sendTo process.
         *
         * @param[in] i_sendTo   receiver proccess rank
         * @param[in] i_rowVec   vector of db rows to send
         * @param[in] i_adapter  adapter to pack db rows
         */
        virtual void startSendPacked(int i_sendTo, const IRowBaseVec & i_rowVec, const IPackedAdapter & i_adapter) = 0;

        /**
         * try to non-blocking receive value array, return true if completed.
         *
         * @param[in]     i_recvFrom  sender proccess rank
         * @param[in]     i_msgTag    tag to identify message content (parameter or output data)
         * @param[in]     i_type      value type
         * @param[in]     i_size      size of array
         * @param[in,out] io_valueArr allocated buffer to recieve value array
         */
        virtual bool tryReceive(int i_recvFrom, MsgTag i_msgTag, const type_info & i_type, size_t i_size, void * io_valueArr) const = 0;

        /**
         * try to non-blocking receive and unpack vector of db rows, return true if completed.
         *
         * @param[in]     i_recvFrom      sender proccess rank
         * @param[in,out] io_resultRowVec vector to push back received db rows
         * @param[in]     i_adapter       adapter to unpack db rows
         */
        virtual bool tryReceive(int i_recvFrom, IRowBaseVec & io_resultRowVec, const IPackedAdapter & i_adapter) const = 0;

        /** wait for non-blocking send to be completed */
        virtual void waitSendAll(void) = 0;
    };
}

#endif  // OM_MSG_H
