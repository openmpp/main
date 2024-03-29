/**
 * @file
 * OpenM++ common file utilities.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_H_FILE_H
#define OM_H_FILE_H

#include "omHelper.h"

namespace openm
{
    /** return true if file or directory exists */
    extern bool isFileExists(const char * i_filePath);

    /** open input file stream
    *
    * Windows-specific: if file path is UTF-8 then convert it to UTF-16LE
    */
    extern void openInpStream(std::ifstream & io_inpSt, const char * i_filePath, std::ios_base::openmode i_mode);

    /** open output file stream
    *
    * Windows-specific: if file path is UTF-8 then convert it to UTF-16LE
    */
    extern void openOutStream(std::ofstream & io_outSt, const char * i_filePath, std::ios_base::openmode i_mode);

    /** return base directory of the path or empty string if path is "." or ".." */
    extern const std::string baseDirOf(const std::string & i_path);

    /** return base directory of the path or empty string if path is "." or ".." */
    extern const std::string baseDirOf(const char * i_path);

    /** make path from current working directory, executable name and specified extension. */
    extern const std::string makeDefaultPath(const char * i_exePath, const char * i_extension);

    /** make path by join directory, file name and specified extension.
     *
     * @param   i_dirPath   path or directory.
     * @param   i_name      file name.
     * @param   i_extension file extension, if not a null.
     *
     * @return  path combined as directory/name.extension
     *
     * It does replace all \ with / even in "special\ path/", except of leading \\ slashes
     * For example:
     *   input\ ageSex csv => input/ageSex.csv
     *    \\host\share ageSex.csv => \\host/share.ageSex.csv
     */
    std::string const makeFilePath(const char * i_dirPath, const char * i_name, const char * i_extension = nullptr);

    /** convert null-terminated bytes from current user (or specified) code page to UTF-8 string. */
    extern const std::string toUtf8(const char * i_byteArr, const char * i_codePageName = nullptr);

    /** convert bytes from current user (or specified) code page to UTF-8 string. */
    extern const std::string toUtf8(size_t i_size, const char * i_byteArr, const char * i_codePageName = nullptr);

    /** return true if bytes are valid UTF-8 */
    extern bool isUtf8(size_t i_size, const char * i_byteArr);

    /** read file and return content as UTF-8 as string */
    extern const std::string fileToUtf8(const char * i_filePath, const char * i_codePageName = nullptr);

    /** read file, split by linefeed \n and return content as list of UTF-8 as strings */
    extern const std::list<std::string> fileToUtf8Lines(const char * i_filePath, const char * i_codePageName = nullptr);

#ifdef _WIN32
    /**
    * Windows only: convert null-terminated bytes from UTF-8 string to UTF-16LE.
    *
    * @return UTF-16LE string or empty "" string on error.
    */
    extern const std::wstring fromUtf8(const char * i_byteArr);
#endif // _WIN32
}

#endif  // OM_H_FILE_H
