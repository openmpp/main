/**
 * @file
 * OpenM++: log to console and into log files
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef OM_LOG_H
#define OM_LOG_H

#include <chrono>
#include <mutex>
#include <thread>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdarg>
using namespace std;

#include "libopenm/omCommon.h"
#include "libopenm/common/omHelper.h"

namespace openm
{
    /**   
    * log implementation class: log to console and into log files.
    *
    * log can be enabled/disabled for 3 independent streams: \n
    *  console         - cout stream \n
    *  "last run" file - log file with specified name, truncated on every model run \n
    *  "stamped" file  - log file with unique name, created for every model run
    * 
    * "stamped" name produced from "last run" name by adding time-stamp and pid-stamp, i.e.: \n
    *  modelOne.log => modelOne_20120817_160459_0148.1234.log
    */
    class Log : public ILog
    {
    public:
        /**
         * create log instance.
         *
         * @param[in]   i_logToConsole  if true then log to console
         * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
         * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
         * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
         */
        Log(
            bool i_logToConsole = false,
            const char * i_basePath = "openm.log", 
            bool i_useTimeStamp = false,
            bool i_usePidStamp = false
            );
        ~Log(void) throw();

        /**
         * re-initialize log file name(s) and other log settings.
         *
         * @param[in]   i_logToConsole  if true then log to console
         * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
         * @param[in]   i_isNoMsgTime   if true then not prefix log messgaes with date-time
         * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
         * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
         * @param[in]   i_noLogTime     if true then not prefix log messgaes with date-time
         * @param[in]   i_isLogSql      if true then log SQL
         */
        void init(
            bool i_logToConsole,
            const char * i_basePath,
            bool i_useTimeStamp = false,
            bool i_usePidStamp = false, 
            bool i_noLogTime = false,
            bool i_isLogSql = false
            ) throw();

        /** log message */
        void logMsg(const char * i_msg, const char * i_extra = nullptr) throw();

        /** log message formatted with vsnprintf() */
        void logFormatted(const char * i_format, ...) throw();

        /** log exception */
        void logErr(const exception & i_ex, const char * i_msg = nullptr) throw();

        /** log sql query */
        void logSql(const char * i_sql) throw();

        /** return timestamp and pid suffix of log file name: _20120817_160459_0148\.1234 */
        const string suffix(void) throw();

        /** return timestamp suffix of log file name: _20120817_160459_0148 */
        const string timeStampSuffix(void) throw();

    private:
        recursive_mutex theMutex;   // mutex to lock for log operations
        bool isConsoleEnabled;      // if true then log to console
        bool isNoTimeLog;           // if true then not prefix log messgaes with date-time
        bool isSqlLog;              // if true then log sql queries into last log
        bool isLastEnabled;         // if true then last log enabled
        bool isLastCreated;         // if true then last log file created
        bool isStampedEnabled;      // if true then last log enabled
        bool isStampedCreated;      // if true then last log file created
        string tsSuffix_;           // log file name timestamp suffix: _20120817_160459_0148
        string pidSuffix_;          // log file name pid suffix: .1234
        string suffix_;             // log file name timestamp and pid suffix: _20120817_160459_0148.1234
        string lastPath_;           // last log file path: /var/log/openm.log
        string stampedPath_;        // stamped log file path: /var/log/openm_20120817_160459_0148.1234.log

        // set log file names
        // "last" log file name, ie: /var/log/openm.log 
        // "stamped" log file name (with optional timestamp and pid suffixes): /var/log/openm_20120817_160459_0148.1234.log
        void setLogPath(
            const char * i_basePath, 
            bool i_useTimeStamp = false, 
            bool i_usePidStamp = false
            );

        // log to console
        static bool logToConsole(
            const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra = nullptr
            ) throw();

        // create log file or truncate existing, return false on error
        static bool logFileCreate(const string & i_path) throw();

        // log to file, return false on error
        static bool logToFile(
            const string & i_path, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra = nullptr
            ) throw();

        // write date-time and message to output stream, return false on error
        static void writeToLog(
            ostream & i_ost, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra = nullptr
            );

    private:
        Log(const Log & i_log) = delete;
        Log & operator=(const Log & i_log) = delete;
    };
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

#endif  // OM_LOG_H
