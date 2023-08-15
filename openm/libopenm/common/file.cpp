/**
 * @file
 * OpenM++ common file utility functions.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include <filesystem>
#include "libopenm/common/omFile.h"
#include "helper.h"
#include "log.h"

using namespace openm;
namespace fs = std::filesystem;

#ifdef _WIN32
    #include <direct.h>
    #include <io.h>
    #include <process.h>
    #ifndef pid_t
        typedef int pid_t;
    #endif 
#else
    #include <unistd.h>
#endif  // _WIN32

/** return true if file or directory exists  */
bool openm::isFileExists(const char * i_filePath)
{
#ifndef _WIN32
    return i_filePath != NULL && i_filePath[0] != '\0' && !access(i_filePath, 0);
#else
    if (i_filePath == NULL || i_filePath[0] == '\0') return false;

    if (!isUtf8(strnlen(i_filePath, OM_STRLEN_MAX), i_filePath)) {
        return !access(i_filePath, 0);
    }
    // else convert UTF-8 file path to UTF-16LE
    return !_waccess(fromUtf8(i_filePath).c_str(), 0);

#endif  // _WIN32
}

/** open input file stream
*
* Windows-specific: if file path is UTF-8 then convert it to UTF-16LE
*/
void openm::openInpStream(ifstream & io_inpSt, const char * i_filePath, ios_base::openmode i_mode)
{
#ifndef _WIN32
    io_inpSt.open(i_filePath, i_mode);
#else  // _WIN32

    if (!isUtf8(strnlen(i_filePath, OM_STRLEN_MAX), i_filePath)) {
        io_inpSt.open(i_filePath, i_mode);
    } else {
        io_inpSt.open(fromUtf8(i_filePath), i_mode);
    }
#endif // _WIN32
}

/** open output file stream
*
* Windows-specific: if file path is UTF-8 then convert it to UTF-16LE
*/
void openm::openOutStream(ofstream & io_outSt, const char * i_filePath, ios_base::openmode i_mode)
{
#ifndef _WIN32
    io_outSt.open(i_filePath, i_mode);
#else  // _WIN32

    if (!isUtf8(strnlen(i_filePath, OM_STRLEN_MAX), i_filePath)) {
        io_outSt.open(i_filePath, i_mode);
    }
    else {
        io_outSt.open(fromUtf8(i_filePath), i_mode);
    }
#endif // _WIN32
}

/**
 * return base directory of the path or empty string if path is "." or ".."
 *          
 * For example: C:\bin\modelOne.exe => C:\bin
 */
const string openm::baseDirOf(const string & i_path)
{
    return baseDirOf(i_path.c_str());
}

/** return base directory of the path or empty string if path is "." or ".." */
const string openm::baseDirOf(const char * i_path)
{
    return (i_path != nullptr) ? fs::path(i_path).parent_path().generic_string() : "";
}

/**
 * make path from current working directory, executable name and specified extension.
 *
 * @param   i_exePath   full pathname of the executable file.
 * @param   i_extension new file extension.
 *
 * @return  path combined as current working directory, executable name and specified extension.
 *          
 * For example: C:\bin\modelOne.exe => D:\work\dir\modelOne.log
 */
const string openm::makeDefaultPath(const char * i_exePath, const char * i_extension)
{
    // get current working directory
    char cwd[OM_PATH_MAX + 1];

    string curDir;
    if (getcwd(cwd, OM_PATH_MAX) != nullptr) curDir = cwd;

    // get executable basename and remove extension if extension is ".exe" case insensitive
    string baseName = (i_exePath != nullptr) ? fs::path(i_exePath).filename().generic_string() : "";

    if (endWithNoCase(baseName, ".exe")) baseName = baseName.substr(0, baseName.length() - 4);

    // do not allow . or .. as file name
    if (baseName == "." || baseName == "..") baseName = "";

    // combine: current/working/directory/basename.log and return with generic / path separator
    fs::path p(curDir);
    string sPath = p.append((!baseName.empty() ? baseName : "openm")).concat((i_extension != nullptr ? i_extension : "")).generic_string();

    return sPath;
}

/**
 * make path by join directory, file name and specified extension.
 *
 * @param   i_dirPath   path or directory.
 * @param   i_name      file name.
 * @param   i_extension file extension, if not a null.
 *
 * @return  path combined as directory/name.extension
 *
 * It does remove all trailing dots ... from file name.
 * If i_dirPath not empty file name expected to be a relative path, not an absolute
 * and any leading / or \ are removed.
 */
const string openm::makeFilePath(const char * i_dirPath, const char * i_name, const char * i_extension)
{
    string dir = (i_dirPath != nullptr) ? i_dirPath : "";

    // make name: remove all final dots 
    string name = (i_name != nullptr) ? i_name : "";

    if (!name.empty()) {
        size_t n = name.find_last_not_of(".");
        if (n != string::npos) {
            name = name.erase(n + 1);
        }
        else {
            name.clear();
        }
    }

    // make name: if directory not empty then remove all leading / or \ separators
    if (!dir.empty() && !name.empty()) {
        size_t n = name.find_first_not_of("/\\");
        if (n != string::npos) {
            name = name.substr(n);
        }
        else {
            name.clear();
        }
    }

    // make extension: add leading .dot
    string ext = (i_extension != nullptr) ? i_extension : "";
    if (!ext.empty() && ext[0] != '.') ext = '.' + ext;

    // combine: dir/name.ext and return with generic / path separator
    fs::path p(dir);
    string sPath = p.append(name).concat(ext).generic_string();

    return sPath;
}
