/**
 * @file
 * OpenM++ message passing library: pack and unpack wrapper class for MPI-based implementation
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef MSG_MPI_PACKED_H
#define MSG_MPI_PACKED_H

using namespace std;

#include "msgCommon.h"

namespace openm
{
    /** wrapper class around of MPI_Pack and MPI_Unpack */
    class MpiPacked
    {
    public:
        /**
         * MPI_Pack value of primitive type into data buffer at i_packPos position.
         *
         * @tparam          TVal          type of i_value to be packed.
         * @param[in]       i_value       value of primitive type to be packed
         * @param[in]       i_packedSize  total size in bytes of io_packedData buffer
         * @param[in,out]   io_packedData buffer to pack MPI message
         * @param[in,out]   io_packPos    current position in io_packedData buffer
         */
        template<typename TVal>
        static void pack(TVal i_value, int i_packedSize, void * io_packedData, int & io_packPos)
        {
            int mpiRet = MPI_Pack(&i_value, 1, toMpiType(typeid(TVal)), io_packedData, i_packedSize, &io_packPos, MPI_COMM_WORLD);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);
        }

        /**
         * MPI_Unpack value of primitive type from data buffer at i_packPos position and return the value.
         *
         * @tparam          TVal          type of value to be unpacked.
         * @param[in]       i_packedSize  total size in bytes of i_packedData buffer
         * @param[in]       i_packedData  source MPI message buffer to unpack
         * @param[in,out]   io_packPos    current position in i_packedData buffer
         */
        template<typename TVal>
        static TVal unpack(int i_packedSize, void * i_packedData, int & io_packPos)
        {
            TVal val;
            int mpiRet = MPI_Unpack(i_packedData, i_packedSize, &io_packPos, &val, 1, toMpiType(typeid(TVal)), MPI_COMM_WORLD);
            if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

            return val;
        }

        /** MPI_Pack string into data buffer at i_packPos position. */
        static void pack(const string & i_value, int i_packedSize, void * io_packedData, int & io_packPos);

        /** MPI_Unpack string from data buffer at i_packPos position and return the string. */
        static string unpackStr(int i_packedSize, void * i_packedData, int & io_packPos);

        /** return an MPI_Pack'ed copy of source array. */
        static unique_ptr<uint8_t[]> packArray(const type_info & i_type, size_t i_size, void * i_valueArr);

        /** return an MPI_Pack'ed copy of source string array. */
        static unique_ptr<uint8_t[]> packArray(size_t i_size, const string * i_valueArr);

        /** unpack MPI_Pack'ed string array from i_packedData into supplied io_valueArr. */
        static void unpackArray(int i_packedSize, void * i_packedData, size_t i_size, string * io_valueArr);

        /** return MPI pack size for array of specified primitive type values. */
        static int packedSize(const type_info & i_type, size_t i_size);

        /** return MPI pack size of string array. */
        static int packedSize(size_t i_size, const string * i_valueArr);

        /** return MPI pack size for specified primitive type. */
        static int packedSize(const type_info & i_type);

        /** return MPI pack size for string value. */
        static int packedSize(const string & i_value);

        /** return MPI type corresponding to source primitive type. */
        static MPI_Datatype toMpiType(const type_info & i_type);
    };

    /** MPI-based adapter to pack and unpack metadata db row.
    *
    * @tparam  TRow    type of metadata db row.
    */
    template<typename TRow> struct RowMpiPackedAdapter
    {
    public:
        /**
         * pack db row into MPI message.
         *
         * @param[in]     i_row         unique_ptr to source metadata db row
         * @param[in]     i_packedSize  total size in bytes of io_packedData buffer
         * @param[in,out] io_packedData destination buffer to pack MPI message
         * @param[in,out] io_packPos    current position in io_packedData buffer
         */
        static void pack(const IRowBaseUptr & i_row, int i_packedSize, void * io_packedData, int & io_packPos);

        /**
         * unpack MPI message into db row.
         *
         * @param[in,out] io_row        unique_ptr to destination metadata db row
         * @param[in]     i_packedSize  total size in bytes of i_packedData buffer
         * @param[in]     i_packedData  source MPI message buffer to unpack
         * @param[in,out] io_packPos    current position in i_packedData buffer
         */
        static void unpackTo(const IRowBaseUptr & io_row, int i_packedSize, void * i_packedData, int & io_packPos);

        /**
         * return byte size to pack db row into MPI message.
         *
         * @param[in]   i_row   unique_ptr to source metadata db row
         */
        static int packedSize(const IRowBaseUptr & i_row);
    };

    /**
     * MPI-based adapter to pack and unpack vector of metadata db rows.
     *
     * @tparam rowMsgTag    message tag for this type of metadata db row.
     * @tparam TRow         type of metadata db row.
     */
    template <MsgTag rowMsgTag, typename TRow> struct MetaMpiPackedAdapter : public IPackedAdapter
    {
    public:
        /** return message tag */
        MsgTag tag(void) const noexcept override { return rowMsgTag; }

        /**
         * pack vector of metadata db rows into byte vector.
         *
         * @param[in] i_rowVec source vector of metadata db rows
         */
        const vector<uint8_t> pack(const IRowBaseVec & i_rowVec) const override
        {
            lock_guard<recursive_mutex> lck(msgMutex);

            int packSize = packedSize(i_rowVec);
            vector<uint8_t> packedData(packSize);

            int packPos = 0;
            int rowCount = (int)i_rowVec.size();
            MpiPacked::pack<int>(rowCount, packSize, packedData.data(), packPos);

            for (IRowBaseVec::const_iterator rowIt = i_rowVec.begin(); rowIt != i_rowVec.end(); rowIt++) {
                RowMpiPackedAdapter<TRow>::pack(*rowIt, packSize, packedData.data(), packPos);
            }
            return packedData;
        }

        /**
         * unpack from byte[] message bufer into vector of metadata db rows.
         *
         * @param[in]     i_packSize    total size in bytes of i_packedData buffer
         * @param[in]     i_packedData  source MPI message buffer to unpack
         * @param[in,out] io_rowVec     destination vector to append metadata db rows
         */
        void unpackTo(int i_packSize, void * i_packedData, IRowBaseVec & io_rowVec) const override
        {
            lock_guard<recursive_mutex> lck(msgMutex);

            int packPos = 0;
            int rowCount = MpiPacked::unpack<int>(i_packSize, i_packedData, packPos);

            for (int nRow = 0; nRow < rowCount; nRow++) {
                IRowBaseUptr row(new TRow());
                RowMpiPackedAdapter<TRow>::unpackTo(row, i_packSize, i_packedData, packPos);
                io_rowVec.push_back(std::move(row));
            }
        }

    private:
        /**
         * return byte size to pack vector of metadata db rows.
         *
         * @param[in] i_rowVec source vector of metadata db rows
         */
        int packedSize(const IRowBaseVec & i_rowVec) const
        {
            int packSize = MpiPacked::packedSize(typeid(int));

            for (IRowBaseVec::const_iterator rowIt = i_rowVec.begin(); rowIt != i_rowVec.end(); rowIt++) {
                packSize += RowMpiPackedAdapter<TRow>::packedSize(*rowIt);
            }
            return packSize;
        }
    };
}

#endif  // MSG_MPI_PACKED_H
