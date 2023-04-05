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
    * auto [nMem, nMax] = getProcessMemorySize();
    *
    * if (nMem > 0) theLog->logFormatted(LT("Current memory size: %f MB"), ((double)nMem / (1024.0 * 1024.0)));
    * if (nMax > 0) theLog->logFormatted(LT("Maximum memory size: %f MB"), ((double)nMax / (1024.0 * 1024.0)));
    * @endcode
    */
    extern std::tuple<uint64_t, uint64_t> getProcessMemorySize(void);
}

#endif  // OM_H_OS_H
