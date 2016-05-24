/**
 * @file
 * OpenM++ common file utilities.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_FILE_H
#define OM_FILE_H

#include "libopenm/common/omHelper.h"

using namespace std;

namespace openm
{
    /** return true if file exists  */
    extern bool isFileExists(const char * i_filePath);

    /** make path from current working directory, executable name and specified extension. */
    extern string makeDefaultPath(const char * i_exePath, const char * i_extension);

    /** convert null-terminated bytes from current user (or specified) code page to UTF-8 string. */
    extern string toUtf8(const char * i_byteArr, const char * i_codePageName = NULL);

    /** convert bytes from current user (or specified) code page to UTF-8 string. */
    extern string toUtf8(size_t i_size, const char * i_byteArr, const char * i_codePageName = NULL);

    /** return true if bytes are valid UTF-8 */
    extern bool isUtf8(size_t i_size, const char * i_byteArr);

    /** read file and return content as UTF-8 as string */
    extern string fileToUtf8(const char * i_filePath, const char * i_codePageName = NULL);
}

#endif  // OM_FILE_H
