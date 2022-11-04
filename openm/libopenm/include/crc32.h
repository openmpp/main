/**
 * @file
 * OpenM++ CRC-32 impelementation, it is same CRC-32 as V.42, gzip, PNG
 */
// Copyright (c) 2016 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef CRC32_H
#define CRC32_H

#include <string>
#include <iomanip>
#include <sstream>
 
using namespace std;

namespace openm
{
    /** calculate CRC-32 on next data block of bytes */
    extern uint32_t calculateNextCrc32(uint32_t i_initialCrc, const void * i_data, size_t i_dataSize);

    /** return CRC-32 of source string */
    extern string crc32String(const string & i_source);
}

#endif  // CRC32_H
