/**
 * @file
 * OpenM++: log to console and into log files
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "log.h"
using namespace openm;

#ifdef _WIN32
    #include <direct.h>
    #include <process.h>
    #ifndef pid_t
        typedef int pid_t;
    #endif 
#else
    #include <unistd.h>
#endif  // _WIN32

/** actual log instance: initialize default log before main */
static Log defaultLog(true, "openm.log");

/** public log interface */
ILog * theLog = &defaultLog;

// buffer to format message
static const size_t msgBufferSize = 32768;
static char msgBuffer[msgBufferSize];

// log public interface
ILog::~ILog(void) throw() { }

/**
* create log instance.
*
* @param[in]   i_logToConsole  if true then log to console
* @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
* @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
* @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
*/
Log::Log(
    bool i_logToConsole, const char * i_basePath, bool i_useTimeStamp, bool i_usePidStamp
    ) :
    isConsoleEnabled(i_logToConsole),
    isNoTimeLog(false),
    isSqlLog(false),
    isLastEnabled(false),
    isLastCreated(false),
    isStampedEnabled(false),
    isStampedCreated(false),
    lastPath_((i_basePath != nullptr) ? i_basePath : ""),
    stampedPath_("")
{
    unique_lock<recursive_mutex> lck(theMutex);
    init(i_logToConsole, i_basePath, i_useTimeStamp, i_usePidStamp, isNoTimeLog, isSqlLog);
}

/** cleanup log resources */
Log::~Log(void) throw()
{ 
    try {
        unique_lock<recursive_mutex> lck(theMutex);
        isConsoleEnabled = isSqlLog = isLastEnabled = isStampedEnabled = false;
    }
    catch (...) { }
}

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
void Log::init(
    bool i_logToConsole, const char * i_basePath, bool i_useTimeStamp, bool i_usePidStamp, bool i_noLogTime, bool i_isLogSql
    ) throw()
{
    try {
        unique_lock<recursive_mutex> lck(theMutex);

        isConsoleEnabled = i_logToConsole;
        if (tsSuffix_ == "") tsSuffix_ = makeTimeStamp(chrono::system_clock::now());
        if (pidSuffix_ == "") pidSuffix_ = '.' + to_string(getpid());
        setLogPath(i_basePath, i_useTimeStamp, i_usePidStamp);
        isNoTimeLog = i_noLogTime;
        isSqlLog = i_isLogSql && isLastEnabled;
    }
    catch (...) { }
}

/** return timestamp and pid suffix of log file name: _20120817_160459_0148\.1234   
*
* it can return empty "" string, depending on log settings
*/
const string Log::suffix(void) throw()
{
    try {
        unique_lock<recursive_mutex> lck(theMutex);
        return suffix_.substr();
    }
    catch (...) { 
    }
    return "";  // return on exception
}

/** return timestamp suffix of log file name: _20120817_160459_0148.   
*
* it is never return empty "" string, even no log enabled or timestamp disable for log file
*/
const string Log::timeStampSuffix(void) throw()
{
    try {
        unique_lock<recursive_mutex> lck(theMutex);
        return tsSuffix_.substr();
    }
    catch (...) { 
    }
    return "";  // return on exception
}

// set log file names
// "last" log file name, ie: /var/log/openm.log 
// "stamped" log file name (with optional timestamp and pid suffixes): /var/log/openm_20120817_160459_0148.1234.log
void Log::setLogPath(const char * i_basePath, bool i_useTimeStamp, bool i_usePidStamp)
{
    isLastEnabled = isLastCreated = isStampedEnabled = isStampedCreated = false;

    // if no base log file name specified then log disabled
    size_t len = (i_basePath != nullptr) ? strnlen(i_basePath, OM_PATH_MAX) : 0;
    if (len <= 0 || len >= OM_PATH_MAX) return;

    lastPath_ = i_basePath;     // last log file name

    // stamped log file suffix: use supplied "forced" suffix or build timestamp and pid stamp suffix
    suffix_ = ((i_useTimeStamp ? tsSuffix_ : "") + (i_usePidStamp ? pidSuffix_ : ""));

    // if suffix not empty then make stamped log file name by inserting suffix before file extension
    if (!suffix_.empty()) {

        stampedPath_ = lastPath_;
        string::size_type namePos = stampedPath_.find_last_of("/\\");
        string::size_type extPos = stampedPath_.rfind('.');

        if (extPos != string::npos && (namePos == string::npos || (namePos != string::npos && extPos > namePos))) {
            stampedPath_.insert(extPos, suffix_);
        }
        else {
            stampedPath_.append(suffix_);
        }
        isStampedEnabled = true;    // enable use of stamped log file
    }

    isLastEnabled = true;   // enable use of last log file
}

/** log message */
void Log::logMsg(const char * i_msg, const char * i_extra) throw()
{
    try {
        if (i_msg == nullptr && i_extra == nullptr) return;   // nothing to log

        chrono::system_clock::time_point msgTime = 
            (!isNoTimeLog) ? chrono::system_clock::now() : chrono::system_clock::time_point::min();

        unique_lock<recursive_mutex> lck(theMutex);     // lock the log 

        if (isConsoleEnabled) isConsoleEnabled = logToConsole(msgTime, i_msg, i_extra);
        if (isLastEnabled) {
            if (!isLastCreated) isLastEnabled = isLastCreated = logFileCreate(lastPath_);
            if (isLastEnabled) isLastEnabled = logToFile(lastPath_, msgTime, i_msg, i_extra);
        }
        if (isStampedEnabled) {
            if (!isStampedCreated) isStampedEnabled = isStampedCreated = logFileCreate(stampedPath_);
            if (isStampedEnabled) isStampedEnabled = logToFile(stampedPath_, msgTime, i_msg, i_extra);
        }
    }
    catch (...) { }
}

/** log message formatted with vsnprintf() */
void Log::logFormatted(const char * i_format, ...) throw()
{
    try {
        if (i_format == nullptr) return;       // nothing to log

        unique_lock<recursive_mutex> lck(theMutex); // lock the log 

        // format message for the log
        va_list args;
        va_start(args, i_format);
        formatTo(msgBufferSize, msgBuffer, i_format, args);
        va_end(args);

        // log formatted message
        logMsg(msgBuffer);
    }
    catch (...) { }
}

/** log exception */
void Log::logErr(const exception & i_ex, const char * i_msg) throw()
{
    try {
        logMsg(i_msg, i_ex.what());
    }
    catch (...) { }
}

/** log sql query */
void Log::logSql(const char * i_sql) throw()
{
    try {
        if (i_sql == nullptr) return;  // nothing to log

        chrono::system_clock::time_point msgTime =
            (!isNoTimeLog) ? chrono::system_clock::now() : chrono::system_clock::time_point::min();

        unique_lock<recursive_mutex> lck(theMutex);     // lock the log 

        if (isSqlLog && isLastEnabled) {
            if (!isLastCreated) isLastEnabled = isLastCreated = logFileCreate(lastPath_);
            if (isLastEnabled) isLastEnabled = logToFile(lastPath_, msgTime, i_sql);
        }
    }
    catch (...) { }
}

// create log file or truncate existing, return false on error
bool Log::logFileCreate(const string & i_path) throw()
{
    try {
        ofstream logSt;
        exit_guard<ofstream> onExit(&logSt, &ofstream::close);  // close on exit

        logSt.open(i_path, ios_base::out | ios_base::trunc);
        bool isCreated = !logSt.fail();
        return isCreated;
    }
    catch (...) { 
        return false;   // log failed
    }
}

// log to file, return false on error
bool Log::logToFile(
    const string & i_path, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
    ) throw()
{
    try {
        ofstream logSt;
        exit_guard<ofstream> onExit(&logSt, &ofstream::close);  // close on exit

        logSt.open(i_path, ios_base::out | ios_base::app);
        bool isOk = !logSt.fail();
        if (isOk) {
            writeToLog(logSt, i_msgTime, i_msg, i_extra);
            isOk = !logSt.fail();
        }
        return isOk;
    }
    catch (...) { 
        return false;   // log failed
    }
}

// log to console
bool Log::logToConsole(
    const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
    ) throw()
{
    try {
        writeToLog(cout, i_msgTime, i_msg, i_extra);
    }
    catch (...) { 
        return false;   // log failed
    }
    return true;
}

// write date-time and message to output stream
void Log::writeToLog(
    ostream & i_ost, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
    )
{
    if (i_msg == nullptr && i_extra == nullptr) return;   // nothing to log

    // check is date-time prefix required
    if (i_msgTime != chrono::system_clock::time_point::min()) {

        time_t now_time = chrono::system_clock::to_time_t(i_msgTime);
        tm * now_tm = localtime(&now_time);

        i_ost << setfill('0') <<
            setw(4) << now_tm->tm_year + 1900 << '-' << setw(2) << now_tm->tm_mon + 1 << '-' << setw(2) << now_tm->tm_mday <<
            ' ' <<
            setw(2) << now_tm->tm_hour << ':' << setw(2) << now_tm->tm_min << ':' << setw(2) << now_tm->tm_sec <<
            '.' <<
            setw(4) << chrono::duration_cast<chrono::milliseconds>(i_msgTime.time_since_epoch()).count() % 1000LL <<
            ' ';
    }

    // write message and extra info
    if (i_msg != nullptr) i_ost << i_msg;
    if (i_msg != nullptr && i_extra != nullptr) i_ost << ':';
    if (i_extra != nullptr) i_ost << ' ' << i_extra;
    
    i_ost << endl;
}
