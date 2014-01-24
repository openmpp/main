/**
 * @file
 * OpenM++: public interface for common structures.
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef OM_COMMON_H
#define OM_COMMON_H

#include <cfloat>
#include <string>
using namespace std;

namespace openm
{
    /** model types */
    enum class ModelType
    {
        /** case based model */
        caseBased,

        /** time based model */
        timeBased
    };

    /** log public interface: log to console and into log files */
    struct ILog
    {
        virtual ~ILog(void) throw() = 0;

        /** log message */
        virtual void logMsg(const char * i_msg, const char * i_extra = NULL) throw() = 0;

        /** log message formatted with vsnprintf() */
        virtual void logFormatted(const char * i_format, ...) throw() = 0;

        /** log exception */
        virtual void logErr(const exception & i_ex, const char * i_msg = NULL) throw() = 0;

        /** log sql query */
        virtual void logSql(const char * i_sql) throw() = 0;

        /** return timestamp suffix of log file name: _20120817_160459_0148.   
        *
        * it is never return empty "" string, even no log enabled or timestamp disable for log file
        */
        virtual const string timeStampSuffix(void) throw() = 0;

        /** re-initialize log file name(s) and other log settings */
        virtual void init(
            bool i_logToConsole,
            const char * i_basePath,
            bool i_useTimeStamp = false, 
            bool i_usePidStamp = false, 
            const char * i_forceSuffix = NULL,
            bool i_isLogSql = false
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

#ifdef __cplusplus
    }
#endif

// add file and line number to debug log
#ifdef _DEBUG
    #define OM_DBG_STR(line)        #line
    #define OM_DBG_TO_STR(s_line)   OM_DBG_STR(s_line)
    #define OM_FILE_LINE            __FILE__ ":" OM_DBG_TO_STR( __LINE__ )
#else
//    #define OM_FILE_LINE 
    #define OM_FILE_LINE            ""
#endif  // _DEBUG

//#define OM_LOG_MSG(msg)         msg OM_FILE_LINE

#endif  // OM_COMMON_H
