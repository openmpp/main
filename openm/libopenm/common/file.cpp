/**
 * @file
 * OpenM++ common file utilities.
 */
// Copyright (c) 2013 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/common/omFile.h"
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


/** return true if file exists  */
bool openm::isFileExists(const char * i_filePath)
{
    return i_filePath != NULL && i_filePath[0] != '\0' && !access(i_filePath, 0);
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
char cwd[OM_PATH_MAX +1];

    // get current working directory
    string curDir;
    if (getcwd(cwd, OM_PATH_MAX) != NULL) curDir = cwd;

    // get executable basename
    string baseName = i_exePath != NULL ? i_exePath : "";

    string::size_type namePos = baseName.find_last_of("/\\");
    if (namePos != string::npos && namePos + 1 < baseName.length()) baseName = baseName.substr(namePos + 1);

    if (endWithNoCase(baseName, ".exe")) baseName = baseName.substr(0, baseName.length() - 4);

    // combine: current/working/directory/basename.log
    string sPath = (!curDir.empty() ? curDir + "/" : "") + (!baseName.empty() ? baseName : "openm") + (i_extension != NULL ? i_extension : "");
    return sPath;
}
