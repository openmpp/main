/**
 * @file
 * OpenM++: public interface for log and trace support.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_H_LOG_H
#define OM_H_LOG_H

#include <string>
#include <list>
#include <unordered_map>
#include <source_location>

namespace openm
{
    /** base for log public interface */
    struct ILogBase
    {
        virtual ~ILogBase(void) noexcept = 0;

        /** return true if log to console or to file enabled */
        virtual const bool isEnabled(void) noexcept = 0;

        /** log message */
        virtual void logMsg(const char * i_msg, const char * i_extra = NULL) noexcept = 0;

        /** log message formatted with vsnprintf() */
        virtual bool logFormatted(const char * i_format, ...) noexcept = 0;

        /** return timestamp part of log file name: 2012_08_17_16_04_59_148.
        *
        * it is never return empty "" string, even no log enabled or timestamp disabled for log file
        */
        virtual const std::string timeStamp(void) noexcept = 0;

        /** use process rank as log message prefix */
        virtual void setRank(int i_rank, int i_worldSize) noexcept = 0;

        /** error at file create of last log or stamped log file */
        virtual const bool isCreateError(void) noexcept = 0;
    };

    /** log public interface: log to console and into log files */
    struct ILog : public virtual ILogBase
    {
        virtual ~ILog(void) noexcept = 0;

        /** log exception */
        virtual void logErr(const std::exception & i_ex, const char * i_msg = NULL) noexcept = 0;

        /** log sql query */
        virtual void logSql(const char * i_sql) noexcept = 0;

        /** re-initialize log file name(s) and other log settings.
        * @param[in]   i_logToConsole  if true then log to console
        * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
        * @param[in]   i_logToFile     if true then enable log to "last" file
        * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
        * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
        * @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
        * @param[in]   i_isLogSql      if true then log SQL into log file
        */
        virtual void init(
            bool i_logToConsole,
            const char * i_basePath,
            bool i_logToFile,
            bool i_useTimeStamp = false, 
            bool i_usePidStamp = false, 
            bool i_noMsgTime = false,
            bool i_isLogSql = false
            ) noexcept = 0;

        /** if log to file enabled return "last" log file path. */
        virtual const std::string lastLogPath(void) noexcept = 0;

        /** if log to "stamped" file enabled return "stamped" log file path. */
        virtual const std::string stampedLogPath(void) noexcept = 0;

        /** get language-specific message by source non-translated message */
        virtual const std::string getMessage(const char * i_sourceMsg) noexcept = 0;

        /** get list of language name for the messages, eg: (en-ca, en) */
        virtual const std::list<std::string> getLanguages(void) noexcept = 0;

        /** get copy of language-specific messages */
        virtual const std::unordered_map<std::string, std::string> getLanguageMessages(void) noexcept = 0;

        /** set language-specific messages and update list of languages */
        virtual void swapLanguageMessages(const std::list<std::string> & i_langLst, std::unordered_map<std::string, std::string> & io_msgMap) noexcept = 0;
    };

    /** trace log public interface: model event log to console and into log files */
    struct ITrace : public virtual ILogBase
    {
        virtual ~ITrace(void) noexcept = 0;

        /** re-initialize log file name(s) and other log settings.
        *
        * @param[in]   i_logToConsole  if true then log to console
        * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
        * @param[in]   i_logToFile     if true then enable log to "last" file
        * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
        * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
        * @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
        */
        virtual void init(
            bool i_logToConsole,
            const char * i_basePath,
            bool i_logToFile,
            bool i_useTimeStamp = false,
            bool i_usePidStamp = false,
            bool i_noMsgTime = false
            ) noexcept = 0;

        /** log message formatted with vsnprintf(), throw exception on error */
        virtual void logFormattedOrFail(const char * i_format, ...) = 0;
    };

    /** checkpoint log message */
    inline thread_local const char * the_checkpoint_message = "";

    /** checkpoint source code location: file name, line and column */
    inline thread_local std::source_location the_checkpoint_location;
}

/** save checkpoint message and source code location */
#define CHECKPOINT(x) { openm::the_checkpoint_location = std::source_location::current(); openm::the_checkpoint_message = x; }

/** output to the log latest checkpoint message and source code location */
#define LOG_CHECKPOINT \
    theLog->logFormatted("%s: [%d,%d] %s", \
        the_checkpoint_message, \
        the_checkpoint_location.line(), the_checkpoint_location.column(), the_checkpoint_location.file_name());

//
// OpenM++ common library export
//
#ifdef __cplusplus
    extern "C" {
#endif

/** log global instance: log to console and into log files */
extern openm::ILog * theLog;

/** trace log global instance: model debug output to console and log files */
extern openm::ITrace * theTrace;

#ifdef __cplusplus
    }
#endif

/** LT localisation function: return is temporary const char* and must be copied to avoid memory violation crash. */
#define LT(sourceMessage) ((theLog->getMessage(sourceMessage)).c_str())

/** NO_LT localisation function: return source content as is. Use it to suppress translation, for example: WarningMsg( NO_LT("};") ); */
#define NO_LT(sourceMessage) sourceMessage

#endif  // OM_H_LOG_H
