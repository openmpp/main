/**
 * @file
 * OpenM++: log to console and into log files
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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

#include "libopenm/omLog.h"
#include "libopenm/omError.h"
#include "libopenm/common/omHelper.h"
#include "helper.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4250)
#endif  // _WIN32

namespace openm
{
    /** Log base class: log to console and into log files. */
    class LogBase : public virtual ILogBase
    {
    public:
        /**
        * create log instance.
        *
        * @param[in]   i_logToConsole  if true then log to console
        * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
        * @param[in]   i_logToFile     if true then enable log to "last" file
        * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
        * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
        * @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
        */
        LogBase(
            bool i_logToConsole,
            const char * i_basePath,
            bool i_logToFile,
            bool i_useTimeStamp,
            bool i_usePidStamp,
            bool i_noMsgTime
            );
        ~LogBase(void) noexcept;

        /** return timestamp part of log file name: 2012_08_17_16_04_59_148.
        *
        * it is never return empty "" string, even no log enabled or timestamp disabled for log file
        */
        const string timeStamp(void) noexcept override;

        /** re-initialize log file name(s) and other log settings.
        *
        * @param[in]   i_logToConsole  if true then log to console
        * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
        * @param[in]   i_logToFile     if true then enable log to "last" file
        * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
        * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
        * @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
        */
        void init(
            bool i_logToConsole,
            const char * i_basePath,
            bool i_logToFile,
            bool i_useTimeStamp = false,
            bool i_usePidStamp = false,
            bool i_noMsgTime = false
            ) noexcept;

    protected:
        recursive_mutex theMutex;   // mutex to lock for log operations
        bool isConsoleEnabled;      // if true then log to console
        bool isLastEnabled;         // if true then last log enabled
        bool isLastCreated;         // if true then last log file created
        bool isStampedEnabled;      // if true then last log enabled
        bool isStampedCreated;      // if true then last log file created
        string tsPart;              // log file name timestamp part: 2012_08_17_16_04_59_148
        string pidPart;             // log file name pid part: 1234
        string lastPath;            // last log file path: /var/log/openm.log
        string stampedPath;         // stamped log file path: /var/log/openm.2012_08_17_16_04_59_148.1234.log
        bool isNoMsgTime;           // if true then not prefix messages with date-time

        static const size_t msgBufferSize = 32768;
        char msgBuffer[msgBufferSize + 1];          // buffer to format message

    protected:

        /** implement log message: log to console and log files */
        void doLogMsg(const char * i_msg, const char * i_extra) noexcept;

        // create log file or truncate existing, return false on error
        bool logFileCreate(const string & i_path) noexcept;

        // log to console
        bool logToConsole(
            const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra = nullptr
            ) noexcept;

        // log to file, return false on error
        virtual bool logToFile(
            bool i_isToStamped, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra = nullptr
            ) noexcept override = 0;

        // write date-time and message to output stream, return false on error
        void writeToLog(
            ostream & i_ost, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra = nullptr
            );

    private:
        LogBase(const LogBase & i_log) = delete;
        LogBase & operator=(const LogBase & i_log) = delete;
    };

    /**
    * log implementation class: log to console and into log files.
    *
    * log can be enabled/disabled for 3 independent streams: \n
    *  console         - cout stream \n
    *  "last run" file - log file with specified name, truncated on every model run \n
    *  "stamped" file  - log file with unique name, created for every model run
    * 
    * "stamped" name produced from "last run" name by adding time-stamp and pid-stamp, i.e.: \n
    *  modelOne.log => modelOne.2012_08_17_16_04_59_148.1234.log
    */
    class Log : public LogBase, public ILog
    {
    public:
        /**
         * create log instance.
         *
         * @param[in]   i_logToConsole  if true then log to console
         * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
         * @param[in]   i_logToFile     if true then enable log to "last" file
         * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
         * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
         */
        Log(
            bool i_logToConsole = false,
            const char * i_basePath = "openm.log",
            bool i_logToFile = false,
            bool i_useTimeStamp = false,
            bool i_usePidStamp = false
            );
        ~Log(void) noexcept;

        /**
         * re-initialize log file name(s) and other log settings.
         *
         * @param[in]   i_logToConsole  if true then log to console
         * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
         * @param[in]   i_logToFile     if true then enable log to "last" file
         * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
         * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
         * @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
         * @param[in]   i_isNoMsgTime   if true then not prefix log messgaes with date-time
         * @param[in]   i_isLogSql      if true then log SQL into log file
         */
        void init(
            bool i_logToConsole,
            const char * i_basePath,
            bool i_logToFile,
            bool i_useTimeStamp = false,
            bool i_usePidStamp = false, 
            bool i_noMsgTime = false,
            bool i_isLogSql = false
            ) noexcept;

        /** if log to file enabled return "last" log file path. */
        const string lastLogPath(void) noexcept override;

        /** if log to "stamped" file enabled return "stamped" log file path. */
        const string stampedLogPath(void) noexcept override;

        /** get language-specific message by source non-translated message */
        const string getMessage(const char * i_sourceMsg) noexcept override;

        /** get list of language name for the messages, eg: (en-ca, en) */
        const list<string> getLanguages(void) noexcept override;

        /** get copy of language-specific messages */
        const unordered_map<string, string> getLanguageMessages(void) noexcept override;

        /** set language-specific messages and update list of languages */
        void swapLanguageMessages(const list<string> & i_langLst, unordered_map<string, string> & io_msgMap) noexcept override;

        /** log message */
        void logMsg(const char * i_msg, const char * i_extra = NULL) noexcept override;

        /** log message formatted with vsnprintf() */
        void logFormatted(const char * i_format, ...) noexcept override;

        /** log exception */
        void logErr(const exception & i_ex, const char * i_msg = nullptr) noexcept;

        /** log sql query */
        void logSql(const char * i_sql) noexcept;

    private:
        bool isSqlLog;                          // if true then log sql queries into last log
        list<string> msgLangLst;                // list of languages, in order of user prefrences
        unordered_map<string, string> msgMap;   // language-specific messages

        // log to file, return false on error
        bool logToFile(
            bool i_isToStamped, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra = nullptr
            ) noexcept;

    private:
        Log(const Log & i_log) = delete;
        Log & operator=(const Log & i_log) = delete;
    };

    /**
    * trace log implementation class: model event log to console and into log files.
    *
    * trace log is faster than "regular" log but may not be flashed on runtime errors. \n
    * 
    * log can be enabled/disabled for 3 independent streams: \n
    *  console         - cout stream \n
    *  "last run" file - log file with specified name, truncated on every model run \n
    *  "stamped" file  - log file with unique name, created for every model run
    *
    * "stamped" name produced from "last run" name by adding time-stamp and pid-stamp, i.e.: \n
    *   trace.log => trace.2012_08_17_16_04_59_148.1234.log
    */
    class TraceLog : public LogBase, public ITrace
    {
    public:
        /**
        * create log instance.
        *
        * @param[in]   i_logToConsole  if true then log to console
        * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
        * @param[in]   i_logToFile     if true then enable log to "last" file
        * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
        * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
        * @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
        */
        TraceLog(
            bool i_logToConsole = false,
            const char * i_basePath = "",
            bool i_logToFile = false,
            bool i_useTimeStamp = false,
            bool i_usePidStamp = false,
            bool i_noMsgTime = false
            ) : LogBase(i_logToConsole, i_basePath, i_logToFile, i_useTimeStamp, i_usePidStamp, i_noMsgTime)
        { }

        ~TraceLog(void) noexcept;

        /** re-initialize log file name(s) and other log settings.
        *
        * @param[in]   i_logToConsole  if true then log to console
        * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
        * @param[in]   i_logToFile     if true then enable log to "last" file
        * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
        * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
        * @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
        */
        void init(
            bool i_logToConsole,
            const char * i_basePath,
            bool i_logToFile,
            bool i_useTimeStamp = false,
            bool i_usePidStamp = false,
            bool i_noMsgTime = false
            ) noexcept;

        /** log message */
        void logMsg(const char * i_msg, const char * i_extra = NULL) noexcept override;

        /** log message formatted with vsnprintf() */
        void logFormatted(const char * i_format, ...) noexcept override;

    private:
        ofstream lastSt;        // last log output stream
        ofstream stampedSt;     // stamped log output stream

        // log to file, return false on error
        bool logToFile(
            bool i_isToStamped, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra = nullptr
            ) noexcept;

    private:
        TraceLog(const TraceLog & i_log) = delete;
        TraceLog & operator=(const TraceLog & i_log) = delete;
    };
}

#ifdef _WIN32
#pragma warning(pop)
#endif  // _WIN32

#endif  // OM_LOG_H
