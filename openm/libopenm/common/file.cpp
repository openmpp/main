/**
 * @file
 * OpenM++ common file utility functions.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/common/omFile.h"
#include "helper.h"
#include "log.h"

using namespace openm;

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
    return i_filePath != NULL && i_filePath[0] != '\0' && !access(i_filePath, 0);
}

/**
 * return base directory of the path or empty string if no / or \ in the path.
 *          
 * For example: C:\bin\modelOne.exe => C:\bin \n
 * It is very primitive function and results incorrect if: "special\ path" => "special
 */
string openm::baseDirOf(const string & i_path)
{
    string::size_type nLast = i_path.find_last_of("/\\");
    if (nLast != string::npos) return i_path.substr(0, nLast);
    return "";
}

/** return base directory of the path or empty string if no / or \ in the path. */
string openm::baseDirOf(const char * i_path)
{
    if (i_path != nullptr) {
        string sPath = i_path;
        return baseDirOf(sPath);
    }
    return "";
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
string openm::makeDefaultPath(const char * i_exePath, const char * i_extension)
{
char cwd[OM_PATH_MAX + 1];

    // get current working directory
    string curDir;
    if (getcwd(cwd, OM_PATH_MAX) != nullptr) curDir = cwd;

    // get executable basename
    string baseName = i_exePath != nullptr ? i_exePath : "";

    string::size_type namePos = baseName.find_last_of("/\\");
    if (namePos != string::npos && namePos + 1 < baseName.length()) baseName = baseName.substr(namePos + 1);

    if (endWithNoCase(baseName, ".exe")) baseName = baseName.substr(0, baseName.length() - 4);

    // combine: current/working/directory/basename.log
    string sPath = 
        (!curDir.empty() ? curDir + "/" : "") + 
        (!baseName.empty() ? baseName : "openm") + 
        (i_extension != nullptr ? i_extension : "");

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
 * It does replace all \ with / even in "special\ path/", except of leading \\ slashes
 * For example: 
 *   input\ ageSex csv => input/ageSex.csv
*    \\host\share ageSex.csv => \\host/share.ageSex.csv
 */
string openm::makeFilePath(const char * i_dirPath, const char * i_name, const char * i_extension)
{
    // make directory: replace all \ by / except of leading \\ and append trailng/ separator
    string path = (i_dirPath != nullptr) ? i_dirPath : "";

    if (path.length() > 0) {
        std::replace(
            ((path.length() >= 2 && path[0] == '\\' && path[1] == '\\') ? path.begin() + 2 : path.begin()),
            path.end(),
            '\\', '/'
        );
        if (path.length() >= 2 && path != "\\\\" && path.back() != '/') path += '/';
        if (path.length() == 1 && path.back() != '/') path += '/';
    }

    // make name: remove final dot. 
    // if path not empty then remove leading / and replace all \ by / separator
    // if path empty then replace all \ by / except of leading \\ slashes
    string name = (i_name != nullptr) ? i_name : "";

    if (!name.empty() && name != "." && name != ".." && name.back() == '.') name = name.substr(0, name.length() - 1);

    if (!path.empty()) {
        std::replace(name.begin(), name.end(), '\\', '/');
        if (!name.empty() && name[0] == '/') name = name.substr(1);
    }
    else {
        std::replace(
            ((name.length() >= 2 && name[0] == '\\' && name[1] == '\\') ? name.begin() + 2 : name.begin()),
            name.end(),
            '\\', '/'
        );
    }

    // make extension: add leading .dot
    string ext = (i_extension != nullptr) ? i_extension : "";
    if (!ext.empty() && ext[0] != '.') ext = '.' + ext;

    return path + name + ext;
}
