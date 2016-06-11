/**
 * @file
 * OpenM++: public interface for log and trace support.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_OM_LOG_H
#define OM_OM_LOG_H

#include <string>
using namespace std;

namespace openm
{
    /** base for log public interface */
    struct ILogBase
    {
        virtual ~ILogBase(void) throw() = 0;

        /** log message */
        virtual void logMsg(const char * i_msg, const char * i_extra = NULL) throw() = 0;

        /** log message formatted with vsnprintf() */
        virtual void logFormatted(const char * i_format, ...) throw() = 0;

        /** return timestamp suffix of log file name: _20120817_160459_0148.
        *
        * it is never return empty "" string, even no log enabled or timestamp disabled for log file
        */
        virtual const string timeStampSuffix(void) throw() = 0;

        /** return "stamped" log file name suffix which may include timestamp and pid. */
        virtual const string suffix(void) throw() = 0;
    };

    /** log public interface: log to console and into log files */
    struct ILog : public virtual ILogBase
    {
        virtual ~ILog(void) throw() = 0;

        /** log exception */
        virtual void logErr(const exception & i_ex, const char * i_msg = NULL) throw() = 0;

        /** log sql query */
        virtual void logSql(const char * i_sql) throw() = 0;

        /** re-initialize log file name(s) and other log settings */
        virtual void init(
            bool i_logToConsole,
            const char * i_basePath,
            bool i_useTimeStamp = false, 
            bool i_usePidStamp = false, 
            bool i_noMsgTime = false,
            bool i_isLogSql = false
            ) throw() = 0;
    };

    /** trace log public interface: model event log to console and into log files */
    struct ITrace : public virtual ILogBase
    {
        virtual ~ITrace(void) throw() = 0;

        /** re-initialize log file name(s) and other log settings.
        *
        * @param[in]   i_logToConsole  if true then log to console
        * @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
        * @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
        * @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
        * @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
        */
        virtual void init(
            bool i_logToConsole,
            const char * i_basePath,
            bool i_useTimeStamp = false,
            bool i_usePidStamp = false,
            bool i_noMsgTime = false
            ) throw() = 0;
    };
}

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

#endif  // OM_OM_LOG_H
