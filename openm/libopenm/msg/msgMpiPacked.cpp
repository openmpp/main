/**
 * @file
 * OpenM++ message passing library: pack and unpack wrapper class for MPI-based implementation
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifdef OM_MSG_MPI

using namespace std;

#include "msgCommon.h"
using namespace openm;

/**
* MPI_Pack bool value into data buffer at i_packPos position.   
*
* @param[in]     i_value       bool value to be packed
* @param[in]     i_packedSize  total size in bytes of io_packedData buffer
* @param[in,out] io_packedData destination buffer to pack MPI message
* @param[in,out] io_packPos    current position in io_packedData buffer
*/
void MpiPacked::pack(bool i_value, int i_packedSize, void * io_packedData, int & io_packPos)
{
    int8_t val = i_value ? 1 : 0;
    int mpiRet = MPI_Pack(&val, 1, MPI_INT8_T, io_packedData, i_packedSize, &io_packPos, MPI_COMM_WORLD);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);
}

/**
 * MPI_Pack string into data buffer at i_packPos position.
 *
* @param[in]     i_value       string to be packed
* @param[in]     i_packedSize  total size in bytes of io_packedData buffer
* @param[in,out] io_packedData destination buffer to pack MPI message
* @param[in,out] io_packPos    current position in io_packedData buffer
*/
void MpiPacked::pack(const string & i_value, int i_packedSize, void * io_packedData, int & io_packPos)
{
    char zeroChar = '\0';
    int len = (int)i_value.length() + 1;

    int mpiRet = MPI_Pack(&len, 1, MPI_INT, io_packedData, i_packedSize, &io_packPos, MPI_COMM_WORLD);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

    if (len > 1) {
        mpiRet = MPI_Pack((void *)(i_value.c_str()), (int)i_value.length(), MPI_CHAR, io_packedData, i_packedSize, &io_packPos, MPI_COMM_WORLD);
        if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);
    }
    mpiRet = MPI_Pack(&zeroChar, 1, MPI_CHAR, io_packedData, i_packedSize, &io_packPos, MPI_COMM_WORLD);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);
}

/**
* MPI_Unpack bool value from data buffer at i_packPos position and return the value.
*
* @param[in]       i_packedSize  total size in bytes of i_packedData buffer
* @param[in]       i_packedData  source MPI message buffer to unpack
* @param[in,out]   io_packPos    current position in i_packedData buffer
*/
bool MpiPacked::unpackBool(int i_packedSize, void * i_packedData, int & io_packPos)
{
    if (io_packPos >= i_packedSize) 
        throw MsgException("Unpack error: position=%d out of range=%d", io_packPos, i_packedSize);

    int8_t val;
    int mpiRet = MPI_Unpack(i_packedData, i_packedSize, &io_packPos, &val, 1, MPI_INT8_T, MPI_COMM_WORLD);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

    return val != 0;
}

/**
* MPI_Unpack string from data buffer at i_packPos position and return the string.
*
* @param[in]       i_packedSize  total size in bytes of i_packedData buffer
* @param[in]       i_packedData  source MPI message buffer to unpack
* @param[in,out]   io_packPos    current position in i_packedData buffer
*/
string MpiPacked::unpackStr(int i_packedSize, void * i_packedData, int & io_packPos)
{
    if (io_packPos >= i_packedSize) 
        throw MsgException("Unpack error: position=%d out of range=%d", io_packPos, i_packedSize);

    int len;
    int mpiRet = MPI_Unpack(i_packedData, i_packedSize, &io_packPos, &len, 1, MPI_INT, MPI_COMM_WORLD);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

    if (len < 1 || io_packPos + len > i_packedSize) 
        throw MsgException("Unpack error: string length=%d or position=%d out of range=%d", len, io_packPos, i_packedSize);

    unique_ptr<char[]> cBuf(new char[len]);
    mpiRet = MPI_Unpack(i_packedData, i_packedSize, &io_packPos, cBuf.get(), len, MPI_CHAR, MPI_COMM_WORLD);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

    string sVal = cBuf.get();
    return sVal;
}

/**
* return MPI pack size for specified primitive type.
*
* @param[in] i_type type of value to be packed
*/
int MpiPacked::packedSize(const type_info & i_type)
{
    int nSize = 0;
    int mpiRet = MPI_Pack_size(1, toMpiType(i_type), MPI_COMM_WORLD, &nSize);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

    return nSize;
}

/** return MPI pack size for bool value. */
int MpiPacked::packedSize(bool /* i_value */)
{
    int nSize = 0;
    int mpiRet = MPI_Pack_size(1, MPI_INT8_T, MPI_COMM_WORLD, &nSize);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);

    return nSize;
}

/**
* return MPI pack size for string value.
*
* @param[in] i_value    string to be packed
*/
int MpiPacked::packedSize(const string & i_value)
{
    int packSize = 0;
    int nSize = 0;

    int mpiRet = MPI_Pack_size(1, MPI_INT, MPI_COMM_WORLD, &nSize);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);
    packSize += nSize;

    mpiRet = MPI_Pack_size((int)(i_value.length() + 1), MPI_CHAR, MPI_COMM_WORLD, &nSize);
    if (mpiRet != MPI_SUCCESS) throw MpiException(mpiRet);
    packSize += nSize;

    return packSize;
}

/**
* return MPI_Datatype corresponding to source primitive type.
*
* @param[in] i_type source primitive type
*/
MPI_Datatype MpiPacked::toMpiType(const type_info & i_type)
{
    if (i_type == typeid(char)) return MPI_CHAR;
    if (i_type == typeid(unsigned char)) return MPI_UNSIGNED_CHAR;
    if (i_type == typeid(short)) return MPI_SHORT;
    if (i_type == typeid(unsigned short)) return MPI_UNSIGNED_SHORT;
    if (i_type == typeid(int)) return MPI_INT;
    if (i_type == typeid(unsigned int)) return MPI_UNSIGNED;
    if (i_type == typeid(long)) return MPI_LONG;
    if (i_type == typeid(unsigned long)) return MPI_UNSIGNED_LONG;
    if (i_type == typeid(long long)) return MPI_LONG_LONG;
    if (i_type == typeid(unsigned long long)) return MPI_UNSIGNED_LONG_LONG;
    if (i_type == typeid(int8_t)) return MPI_INT8_T;
    if (i_type == typeid(uint8_t)) return MPI_UINT8_T;
    if (i_type == typeid(int16_t)) return MPI_INT16_T;
    if (i_type == typeid(uint16_t)) return MPI_UINT16_T;
    if (i_type == typeid(int32_t)) return MPI_INT32_T;
    if (i_type == typeid(uint32_t)) return MPI_UINT32_T;
    if (i_type == typeid(int64_t)) return MPI_INT64_T;
    if (i_type == typeid(uint64_t)) return MPI_UINT64_T;
    // if (i_type == typeid(bool)) commented to pack/unpack errors return MPI_INT8_T; 
    if (i_type == typeid(float)) return MPI_FLOAT;
    if (i_type == typeid(double)) return MPI_DOUBLE;
    if (i_type == typeid(long double)) return MPI_LONG_DOUBLE;
    // if (i_type == typeid(string)) string value required to determine the size

    throw MsgException("Pack or unpack error: invalid source type");    // conversion to target type is not supported
}

#endif  // OM_MSG_MPI
