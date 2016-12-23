/**
 * @file
 * OpenM++: log to console and into log files
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

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

/** helper exception default error message */
const char openm::helperUnknownErrorMessage[] = "unknown error in helper method";

/** actual log instance: initialize default log before main */
static Log defaultLog(true, "openm.log");

/** public log interface */
ILog * theLog = &defaultLog;

/** actual trace log instance: disabled by default */
static TraceLog defaultTrace;

/** public trace log interface */
ITrace * theTrace = &defaultTrace;

// log public interfaces
ILogBase::~ILogBase(void) throw() { }
ILog::~ILog(void) throw() { }
ITrace::~ITrace(void) throw() { }

/**
* create log instance.
*
* @param[in]   i_logToConsole  if true then log to console
* @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
* @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
* @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
* @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
*/
LogBase::LogBase(
    bool i_logToConsole, const char * i_basePath, bool i_useTimeStamp, bool i_usePidStamp, bool i_noMsgTime
    ) :
    isConsoleEnabled(i_logToConsole),
    isLastEnabled(false),
    isLastCreated(false),
    isStampedEnabled(false),
    isStampedCreated(false),
    lastPath((i_basePath != nullptr) ? i_basePath : ""),
    stampedPath(""),
    isNoMsgTime(false)
{
    lock_guard<recursive_mutex> lck(theMutex);

    tsSuffix = makeTimeStamp(chrono::system_clock::now());
    pidSuffix = '.' + to_string(getpid());
    msgBuffer[0] = msgBuffer[msgBufferSize] = '\0';

    init(i_logToConsole, i_basePath, i_useTimeStamp, i_usePidStamp, i_noMsgTime);
}

/** cleanup log resources */
LogBase::~LogBase(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        isConsoleEnabled = isLastEnabled = isStampedEnabled = false;
    }
    catch (...) { }
}

/** re-initialize log file name(s) and other log settings.
*
* @param[in]   i_logToConsole  if true then log to console
* @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
* @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
* @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
* @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
*/
void LogBase::init(
    bool i_logToConsole, const char * i_basePath, bool i_useTimeStamp, bool i_usePidStamp, bool i_noMsgTime
    ) throw()
{
    try {
        // reset log status flags
        lock_guard<recursive_mutex> lck(theMutex);

        isConsoleEnabled = i_logToConsole;
        isNoMsgTime = i_noMsgTime;
        isLastEnabled = isLastCreated = isStampedEnabled = isStampedCreated = false;

        // if no base log file name specified then log to file(s) disabled
        size_t len = (i_basePath != nullptr) ? strnlen(i_basePath, OM_PATH_MAX) : 0;
        if (len <= 0 || len >= OM_PATH_MAX) return;

        lastPath = i_basePath;      // last log file name
        isLastEnabled = true;       // enable use of last log file

        // stamped log file suffix: build timestamp and pid stamp suffix
        fileSuffix = ((i_useTimeStamp ? tsSuffix : "") + (i_usePidStamp ? pidSuffix : ""));

        // if suffix not empty then make stamped log file name by inserting suffix before file extension
        if (!fileSuffix.empty()) {

            stampedPath = lastPath;
            string::size_type namePos = stampedPath.find_last_of("/\\");
            string::size_type extPos = stampedPath.rfind('.');

            if (extPos != string::npos && (namePos == string::npos || (namePos != string::npos && extPos > namePos))) {
                stampedPath.insert(extPos, fileSuffix);
            }
            else {
                stampedPath.append(fileSuffix);
            }
            isStampedEnabled = true;    // enable use of stamped log file
        }

    }
    catch (...) { }
}

/** return timestamp suffix of log file name: _20120817_160459_0148.
*
* it is never return empty "" string, even no log enabled or timestamp disabled for log file
*/
const string LogBase::timeStampSuffix(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        return tsSuffix.substr();
    }
    catch (...) {
    }
    return "";  // return on exception
}

/** return timestamp and pid suffix of log file name.
*
* example: _20120817_160459_0148.1234
* it can return empty "" string, depending on log settings
*/
const string LogBase::suffix(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        return fileSuffix.substr();
    }
    catch (...) {
    }
    return "";  // return on exception
}

/** implement log message: log to console and log files */
void LogBase::doLogMsg(const char * i_msg, const char * i_extra) throw()
{
    if (i_msg == nullptr && i_extra == nullptr) return;   // nothing to log

    chrono::system_clock::time_point now = chrono::system_clock::now();

    lock_guard<recursive_mutex> lck(theMutex);     // lock the log 

    if (isConsoleEnabled) {
        isConsoleEnabled = logToConsole(now, i_msg, i_extra);
    }
    if (isLastEnabled) {
        if (!isLastCreated) isLastEnabled = isLastCreated = logFileCreate(lastPath);
        if (isLastEnabled) isLastEnabled = logToFile(false, now, i_msg, i_extra);
    }
    if (isStampedEnabled) {
        if (!isStampedCreated) isStampedEnabled = isStampedCreated = logFileCreate(stampedPath);
        if (isStampedEnabled) isStampedEnabled = logToFile(true, now, i_msg, i_extra);
    }
}

// create log file or truncate existing, return false on error
bool LogBase::logFileCreate(const string & i_path) throw()
{
    try {
        ofstream logSt;
        exit_guard<ofstream> onExit(&logSt, &ofstream::close);  // close on exit

        logSt.open(i_path, ios::out | ios::trunc);
        bool isCreated = !logSt.fail();
        return isCreated;
    }
    catch (...) {
        return false;   // log failed
    }
}

// log to console
bool LogBase::logToConsole(
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
void LogBase::writeToLog(
    ostream & i_ost, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
    )
{
    if (i_msg == nullptr && i_extra == nullptr) return;   // nothing to log

    // check is date-time prefix required
    if (!isNoMsgTime) {

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
    if (i_msg != nullptr && i_extra == nullptr) i_ost << i_msg;
    if (i_msg != nullptr && i_extra != nullptr) i_ost << i_msg << ": " << i_extra;
    if (i_msg == nullptr && i_extra != nullptr) i_ost << i_extra;
    i_ost << endl;
}

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
    LogBase(i_logToConsole, i_basePath, i_useTimeStamp, i_usePidStamp, false),
    isSqlLog(false)
{
    lock_guard<recursive_mutex> lck(theMutex);
    init(i_logToConsole, i_basePath, i_useTimeStamp, i_usePidStamp, isNoMsgTime, isSqlLog);
}

/** cleanup log resources */
Log::~Log(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        isSqlLog = false;
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
* @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
* @param[in]   i_isLogSql      if true then log SQL into log file
*/
void Log::init(
    bool i_logToConsole, const char * i_basePath, bool i_useTimeStamp, bool i_usePidStamp, bool i_noMsgTime, bool i_isLogSql
    ) throw()
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        LogBase::init(i_logToConsole, i_basePath, i_useTimeStamp, i_usePidStamp, i_noMsgTime);
        isSqlLog = i_isLogSql && isLastEnabled;
    }
    catch (...) { }
}

/** get list of language name for the messages, eg: (en-ca, en) */
const list<string> Log::getLanguages(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        list<string> l(msgLangLst.cbegin(), msgLangLst.cend());
        return l;
    }
    catch (...) {
        return list<string>();
    }
}

/** get copy of language-specific messages */
const unordered_map<string, string> Log::getLanguageMessages(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        unordered_map<string, string> m(msgMap.cbegin(), msgMap.cend());
        return m;
    }
    catch (...) {
        return unordered_map<string, string>();
    }
}

/** set language-specific messages and update list of languages */
void Log::swapLanguageMessages(const list<string> & i_langLst, unordered_map<string, string> & io_msgMap) throw()
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        msgLangLst.assign(i_langLst.cbegin(), i_langLst.cend());
        msgMap.swap(io_msgMap);
    }
    catch (...) { }
}

/** log message */
void Log::logMsg(const char * i_msg, const char * i_extra) throw()
{
    try {
        if (i_msg == nullptr && i_extra == nullptr) return;   // nothing to log

        lock_guard<recursive_mutex> lck(theMutex);     // lock the log 

        // translate log message
        if (i_msg != nullptr) {
            const unordered_map<string, string>::const_iterator msgIt = msgMap.find(i_msg);
            if (msgIt != msgMap.cend()) {
                doLogMsg(msgIt->second.c_str(), i_extra);   // log translated message
                return;
            }
        }
        // else log original message: no translation
        doLogMsg(i_msg, i_extra);
    }
    catch (...) { }
}

/** log message formatted with vsnprintf() */
void Log::logFormatted(const char * i_format, ...) throw()
{
    try {
        if (i_format == nullptr) return;       // nothing to log

        lock_guard<recursive_mutex> lck(theMutex); // lock the log 

        // translate log format string
        const char * fmt = i_format;
 
        const unordered_map<string, string>::const_iterator msgIt = msgMap.find(fmt);
        if (msgIt != msgMap.cend()) fmt = msgIt->second.c_str();

        // format message for the log
        va_list args;
        va_start(args, i_format);
        formatTo(msgBufferSize, msgBuffer, fmt, args);
        va_end(args);

        // log formatted message
        doLogMsg(msgBuffer, nullptr);
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

        chrono::system_clock::time_point now = chrono::system_clock::now();

        lock_guard<recursive_mutex> lck(theMutex);     // lock the log 

        if (isSqlLog && isLastEnabled) {
            if (!isLastCreated) isLastEnabled = isLastCreated = logFileCreate(lastPath);
            if (isLastEnabled) isLastEnabled = logToFile(false, now, i_sql);
        }
    }
    catch (...) { }
}

// log to file, return false on error
bool Log::logToFile(
    bool i_isToStamped, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
    ) throw()
{
    try {
        ofstream logSt;
        exit_guard<ofstream> onExit(&logSt, &ofstream::close);  // close on exit

        logSt.open((i_isToStamped ? stampedPath : lastPath), ios::out | ios::app);
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

/** cleanup trace log resources */
TraceLog::~TraceLog(void) throw()
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        
        try { if (isLastCreated) lastSt.close(); }
        catch (...) { }
 
        try { if (isStampedCreated) stampedSt.close(); }
        catch (...) { }
    }
    catch (...) { }
}

/** re-initialize log file name(s) and other log settings.
*
* @param[in]   i_logToConsole  if true then log to console
* @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
* @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
* @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
* @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
*/
void TraceLog::init(
    bool i_logToConsole, const char * i_basePath, bool i_useTimeStamp, bool i_usePidStamp, bool i_noMsgTime
    ) throw()
{
    LogBase::init(i_logToConsole, i_basePath, i_useTimeStamp, i_usePidStamp, i_noMsgTime);
}

/** log message */
void TraceLog::logMsg(const char * i_msg, const char * i_extra) throw()
{
    try {
        doLogMsg(i_msg, i_extra);
    }
    catch (...) { }
}

/** log message formatted with vsnprintf() */
void TraceLog::logFormatted(const char * i_format, ...) throw()
{
    try {
        if (i_format == nullptr) return;       // nothing to log

        lock_guard<recursive_mutex> lck(theMutex); // lock the log 

        // format message for the log
        va_list args;
        va_start(args, i_format);
        formatTo(msgBufferSize, msgBuffer, i_format, args);
        va_end(args);

        // log formatted message
        doLogMsg(msgBuffer, nullptr);
    }
    catch (...) { }
}

// log to file, return false on error
bool TraceLog::logToFile(
    bool i_isToStamped, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
    ) throw()
{
    try {
        ofstream & logSt = i_isToStamped ? stampedSt : lastSt;

        if (!logSt.is_open()) {
            logSt.open((i_isToStamped ? stampedPath : lastPath), ios::out | ios::app);
            if (logSt.fail()) return false;
        }

        writeToLog(logSt, i_msgTime, i_msg, i_extra);
        return !logSt.fail();
    }
    catch (...) {
        return false;   // log failed
    }
}
