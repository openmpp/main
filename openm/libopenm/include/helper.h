/**
 * @file
 * OpenM++ common helper utilities.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef HELPER_H
#define HELPER_H

#include <mutex>
#include "libopenm/common/omHelper.h"

using namespace std;

namespace openm
{
    /** mutex to lock library operations */
    extern recursive_mutex rtMutex;
}

// define max size of file path
#ifdef PATH_MAX
    #define OM_PATH_MAX (FILENAME_MAX < PATH_MAX ? FILENAME_MAX : PATH_MAX)
#endif
#ifndef OM_PATH_MAX
    #ifdef _MAX_PATH
        #define OM_PATH_MAX (FILENAME_MAX < _MAX_PATH ? FILENAME_MAX : _MAX_PATH)
    #else
        #define OM_PATH_MAX (FILENAME_MAX < 4096 ? FILENAME_MAX : 4096)
    #endif 
#endif 

#endif  // HELPER_H
