/**
 * @file
 * OpenM++ platform-specific utility functions.
 */
// Copyright (c) 2013-2023 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_H_OS_H
#define OM_H_OS_H

#include <tuple>
#include "omHelper.h"

namespace openm
{
    /** return current process memory size and max peak memory size, in bytes.
    *
    * if return value is zero then memory information not avaliable.
    * 
    * usage example: \n
    * @code
    *      auto [nCurrentMem, nMaxMem] = getProcessMemorySize();
    * 
    *      if (nCurrentMem > 0) theLog->logFormatted("Current process memory: %llu bytes", nCurrentMem);
    *      if (nMaxMem > 0) theLog->logFormatted("Maximum process memory: %llu bytes", nMaxMem);
    * @endcode
    */
    extern std::tuple<uint64_t, uint64_t> getProcessMemorySize(void);
}

#endif  // OM_H_OS_H
