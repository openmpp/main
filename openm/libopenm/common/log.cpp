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
static Log defaultLog(true, "openm.log", true);

/** public log interface */
ILog * theLog = &defaultLog;

/** actual trace log instance: disabled by default */
static TraceLog defaultTrace;

/** public trace log interface */
ITrace * theTrace = &defaultTrace;

// log public interfaces
ILogBase::~ILogBase(void) noexcept { }
ILog::~ILog(void) noexcept { }
ITrace::~ITrace(void) noexcept { }

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
LogBase::LogBase(
    bool i_logToConsole, const char * i_basePath, bool i_logToFile, bool i_useTimeStamp, bool i_usePidStamp, bool i_noMsgTime
    ) :
    isConsoleEnabled(i_logToConsole),
    isLastEnabled(false),
    isLastCreated(false),
    isErrorLastCreate(false),
    isStampedEnabled(false),
    isStampedCreated(false),
    isErrorStampedCreate(false),
    lastPath((i_basePath != nullptr) ? i_basePath : ""),
    stampedPath(""),
    isNoMsgTime(false),
    isMsgRank(false)
{
    lock_guard<recursive_mutex> lck(theMutex);

    tsPart = makeTimeStamp(chrono::system_clock::now());
    pidPart = to_string(getpid());
    msgBuffer[0] = msgBuffer[msgBufferSize] = '\0';

    init(i_logToConsole, i_basePath, i_logToFile, i_useTimeStamp, i_usePidStamp, i_noMsgTime);
}

/** cleanup log resources */
LogBase::~LogBase(void) noexcept
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
* @param[in]   i_logToFile     if true then enable log to "last" file
* @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
* @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
* @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
*/
void LogBase::init(
    bool i_logToConsole, const char * i_basePath, bool i_logToFile, bool i_useTimeStamp, bool i_usePidStamp, bool i_noMsgTime
    ) noexcept
{
    try {
        // reset log status flags
        lock_guard<recursive_mutex> lck(theMutex);

        isConsoleEnabled = i_logToConsole;
        isNoMsgTime = i_noMsgTime;

        isLastEnabled = isLastCreated = isErrorLastCreate = false;
        isStampedEnabled = isStampedCreated = isErrorStampedCreate = false;

        // if no base log file name specified then log to file(s) disabled
        size_t len = (i_basePath != nullptr) ? strnlen(i_basePath, OM_PATH_MAX) : 0;
        if (len <= 0 || len >= OM_PATH_MAX) return;

        lastPath = i_basePath;          // last log file name
        isLastEnabled = i_logToFile;    // enable use of last log file

        // file suffix with timestamp and pid stamp: .2012_08_17_16_04_59_148.1234
        string stamp = ((i_useTimeStamp ? ('.' + tsPart) : "") + (i_usePidStamp ? ('.' + pidPart) : ""));

        // if suffix not empty then make stamped log file name by inserting suffix before file extension
        if (!stamp.empty()) {

            stampedPath = lastPath;
            string::size_type namePos = stampedPath.find_last_of("/\\");
            string::size_type extPos = stampedPath.rfind('.');

            if (extPos != string::npos && (namePos == string::npos || (namePos != string::npos && extPos > namePos))) {
                stampedPath.insert(extPos, stamp);
            }
            else {
                stampedPath.append(stamp);
            }
            isStampedEnabled = true;    // enable use of stamped log file
        }
    }
    catch (...) { }
}

/** use process rank as log message prefix */
void LogBase::setRank(int i_rank, int i_worldSize) noexcept
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        isMsgRank = true;

        ostringstream st;
        st.imbue(locale::classic());

        if (i_worldSize < 10) st << '[' << i_rank << "] ";
        if (10 <= i_worldSize && i_worldSize < 100) st << setfill(' ') << '[' << setw(2) << i_rank << "] ";
        if (100 <= i_worldSize && i_worldSize < 1000) st << setfill(' ') << '[' << setw(3) << i_rank << "] ";
        if (1000 <= i_worldSize && i_worldSize < 10000) st << setfill(' ') << '[' << setw(4) << i_rank << "] ";
        if (10000 <= i_worldSize) st << setfill(' ') << '[' << setw(5) << i_rank << "] ";

        rankPrefix = st.str();
    }
    catch (...) { }
}

/** return timestamp part of log file name: 2012_08_17_16_04_59_148.
*
* it is never return empty "" string, even no log enabled or timestamp disabled for log file
*/
const string LogBase::timeStamp(void) noexcept
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        return tsPart.substr();
    }
    catch (...) {
    }
    return "";  // return on exception
}

/** return true if log to console or to file enabled. */
const bool LogBase::isEnabled(void) noexcept
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        return isConsoleEnabled || isLastEnabled || isStampedEnabled;
    }
    catch (...) {
    }
    return false;  // return on exception
}

/** error at file create of last log or stamped log file */
const bool LogBase::isCreateError(void) noexcept
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        return isErrorLastCreate || isErrorStampedCreate;
    }
    catch (...) {
    }
    return false;  // return on exception
}

/** implement log message: log to console and log files */
bool LogBase::doLogMsg(const char * i_msg, const char * i_extra) noexcept
{
    if (i_msg == nullptr && i_extra == nullptr) return true;    // nothing to log

    chrono::system_clock::time_point now = chrono::system_clock::now();

    lock_guard<recursive_mutex> lck(theMutex);     // lock the log 

    bool isOk = true;
    if (isConsoleEnabled) {
        isConsoleEnabled = logToConsole(now, i_msg, i_extra);
        isOk = isOk && isConsoleEnabled;
    }
    if (isLastEnabled) {
        if (!isLastCreated) {
            isLastEnabled = isLastCreated = logFileCreate(lastPath);
            isErrorLastCreate = !isLastCreated;
        }
        if (isLastEnabled) isLastEnabled = logToFile(false, now, i_msg, i_extra);
        isOk = isOk && isLastEnabled;
    }
    if (isStampedEnabled) {
        if (!isStampedCreated) {
            isStampedEnabled = isStampedCreated = logFileCreate(stampedPath);
            isErrorStampedCreate = !isStampedCreated;
        }
        if (isStampedEnabled) isStampedEnabled = logToFile(true, now, i_msg, i_extra);
        isOk = isOk && isStampedEnabled;
    }
    return isOk;
}

// create log file or truncate existing, return false on error
bool LogBase::logFileCreate(const string & i_path) noexcept
{
    try {
        ofstream logSt;
        exit_guard<ofstream> onExit(&logSt, &ofstream::close);  // close on exit

        logSt.open(i_path, ios::out | ios::trunc);
        return !logSt.fail();
    }
    catch (...) {
        return false;   // log failed
    }
}

// write date-time and message to output stream, return false on error
bool LogBase::writeToLog(
    ostream & i_ost, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
    )
{
    if (i_msg == nullptr && i_extra == nullptr) return true;    // nothing to log

    // check is date-time prefix required
    if (!isNoMsgTime) {

        time_t now_time = chrono::system_clock::to_time_t(i_msgTime);
        tm * now_tm = localtime(&now_time);

        i_ost << setfill('0') <<
            setw(4) << now_tm->tm_year + 1900 << '-' << setw(2) << now_tm->tm_mon + 1 << '-' << setw(2) << now_tm->tm_mday <<
            ' ' <<
            setw(2) << now_tm->tm_hour << ':' << setw(2) << now_tm->tm_min << ':' << setw(2) << now_tm->tm_sec <<
            '.' <<
            setw(3) << chrono::duration_cast<chrono::milliseconds>(i_msgTime.time_since_epoch()).count() % 1000LL <<
            ' ';
        if (i_ost.fail()) return false;
    }

    // check is process rank prefix required
    if (isMsgRank) {
        i_ost << rankPrefix;
        if (i_ost.fail()) return false;
    }

    // write message and extra info
    if (i_msg != nullptr && i_extra == nullptr) i_ost << i_msg << "\n";
    if (i_msg != nullptr && i_extra != nullptr) i_ost << i_msg << ": " << i_extra << "\n";
    if (i_msg == nullptr && i_extra != nullptr) i_ost << i_extra << "\n";

    return !i_ost.fail();
}

/**
* create log instance.
*
* @param[in]   i_logToConsole  if true then log to console
* @param[in]   i_basePath      path to "last" log file, if NULL or empty "" then no log file
* @param[in]   i_logToFile     if true then enable log to "last" file
* @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
* @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
*/
Log::Log(
    bool i_logToConsole, const char * i_basePath, bool i_logToFile, bool i_useTimeStamp, bool i_usePidStamp
    ) : 
    LogBase(i_logToConsole, i_basePath, i_logToFile, i_useTimeStamp, i_usePidStamp, false),
    isSqlLog(false)
{
    lock_guard<recursive_mutex> lck(theMutex);
    init(i_logToConsole, i_basePath, i_logToFile, i_useTimeStamp, i_usePidStamp, isNoMsgTime, isSqlLog);
}

/** cleanup log resources */
Log::~Log(void) noexcept
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
* @param[in]   i_logToFile     if true then enable log to "last" file
* @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
* @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
* @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
* @param[in]   i_isLogSql      if true then log SQL into log file
*/
void Log::init(
    bool i_logToConsole, 
    const char * i_basePath, 
    bool i_logToFile, 
    bool i_useTimeStamp, 
    bool i_usePidStamp, 
    bool i_noMsgTime, 
    bool i_isLogSql
    ) noexcept
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        LogBase::init(i_logToConsole, i_basePath, i_logToFile, i_useTimeStamp, i_usePidStamp, i_noMsgTime);
        isSqlLog = i_isLogSql && isLastEnabled;
    }
    catch (...) { }
}

/** if log to file enabled return "last" log file path. */
const string Log::lastLogPath(void) noexcept
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        return isLastEnabled ? lastPath.substr() : "";
    }
    catch (...) {
    }
    return "";  // return on exception
}

/** if log to "stamped" file enabled return "stamped" log file path. */
const string Log::stampedLogPath(void) noexcept
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        return isStampedEnabled ? stampedPath.substr() : "";
    }
    catch (...) {
    }
    return "";  // return on exception
}

/** get language-specific message by source non-translated message */
const string Log::getMessage(const char * i_sourceMsg) noexcept
{
    try {
        if (i_sourceMsg == nullptr) return "";  // empty result on null

        lock_guard<recursive_mutex> lck(theMutex);     // lock the log 

        // if translation exist then return copy of translated message else return original message (no translation)
        const unordered_map<string, string>::const_iterator msgIt = msgMap.find(i_sourceMsg);
        return 
            (msgIt != msgMap.cend()) ? msgIt->second.c_str() : i_sourceMsg;
     }
    catch (...) {
        return "";
    }
}

/** get list of language name for the messages, eg: (en-ca, en) */
const list<string> Log::getLanguages(void) noexcept
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
const unordered_map<string, string> Log::getLanguageMessages(void) noexcept
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
void Log::swapLanguageMessages(const list<string> & i_langLst, unordered_map<string, string> & io_msgMap) noexcept
{
    try {
        lock_guard<recursive_mutex> lck(theMutex);
        msgLangLst.assign(i_langLst.cbegin(), i_langLst.cend());
        msgMap.swap(io_msgMap);
    }
    catch (...) { }
}

/** log message */
void Log::logMsg(const char * i_msg, const char * i_extra) noexcept
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

/** log message formatted with vsnprintf(), return false on error */
bool Log::logFormatted(const char * i_format, ...) noexcept
{
    try {
        if (i_format == nullptr) return true;       // nothing to log

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

        return doLogMsg(msgBuffer, nullptr);    // log formatted message, return false on error
    }
    catch (...) {
        return false;   // log failed
    }
}

/** log exception */
void Log::logErr(const exception & i_ex, const char * i_msg) noexcept
{
    try {
        logMsg(i_msg, i_ex.what());
    }
    catch (...) { }
}

/** log sql query */
void Log::logSql(const char * i_sql) noexcept
{
    try {
        if (i_sql == nullptr) return;  // nothing to log

        chrono::system_clock::time_point now = chrono::system_clock::now();

        lock_guard<recursive_mutex> lck(theMutex);     // lock the log 

        if (isSqlLog && isLastEnabled) {
            if (!isLastCreated) {
                isLastEnabled = isLastCreated = logFileCreate(lastPath);
                isErrorLastCreate = !isLastCreated;
            }
            if (isLastEnabled) isLastEnabled = logToFile(false, now, i_sql);
        }
    }
    catch (...) { }
}

// log to console and flush each line, return false on error
bool Log::logToConsole(
    const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
    ) noexcept
{
    try {
        writeToLog(cout, i_msgTime, i_msg, i_extra);
        cout.flush();
    }
    catch (...) {
        return false;   // log failed
    }
    return true;
}

// log to file, return false on error
bool Log::logToFile(
    bool i_isToStamped, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
    ) noexcept
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
TraceLog::~TraceLog(void) noexcept
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

/** re-initialize trace file name(s) and other log settings, it is also create or truncate existing trace file.
*
* @param[in]   i_logToConsole  if true then log to console
* @param[in]   i_basePath      path to "last" trace file, if NULL or empty "" then no trace file
* @param[in]   i_logToFile     if true then enable log to "last" file
* @param[in]   i_useTimeStamp  if true then use timestamp suffix in "stamped" file name
* @param[in]   i_usePidStamp   if true then use PID suffix in "stamped" file name
* @param[in]   i_noMsgTime     if true then not prefix log messages with date-time
*/
void TraceLog::init(
    bool i_logToConsole, const char * i_basePath, bool i_logToFile, bool i_useTimeStamp, bool i_usePidStamp, bool i_noMsgTime
    ) noexcept
{
    LogBase::init(i_logToConsole, i_basePath, i_logToFile, i_useTimeStamp, i_usePidStamp, i_noMsgTime);

    // create or truncate trace file
    if (isLastEnabled && !isLastCreated) {
        isLastEnabled = isLastCreated = logFileCreate(lastPath);
        isErrorLastCreate = !isLastCreated;

    }
}

/** log message */
void TraceLog::logMsg(const char * i_msg, const char * i_extra) noexcept
{
    try {
        doLogMsg(i_msg, i_extra);
    }
    catch (...) { }
}

/** log message formatted with vsnprintf(), return false on error */
bool TraceLog::logFormatted(const char * i_format, ...) noexcept
{
    try {
        if (i_format == nullptr) return true;       // nothing to log

        lock_guard<recursive_mutex> lck(theMutex);  // lock the log

        // format message for the log
        va_list args;
        va_start(args, i_format);
        formatTo(msgBufferSize, msgBuffer, i_format, args);
        va_end(args);

        return doLogMsg(msgBuffer, nullptr);    // log formatted message, return false on error
    }
    catch (...) {
        return false;   // log failed
    }
}


/** log message formatted with vsnprintf(), throw exception on error */
void TraceLog::logFormattedOrFail(const char * i_format, ...)
{
    try {
        if (i_format == nullptr) return;            // nothing to log

        lock_guard<recursive_mutex> lck(theMutex);  // lock the log

        if (isErrorLastCreate || isErrorStampedCreate) throw HelperException(LT("Error at debug trace write"));

        // format message for the log
        va_list args;
        va_start(args, i_format);
        formatTo(msgBufferSize, msgBuffer, i_format, args);
        va_end(args);

        // log formatted message
        if (!doLogMsg(msgBuffer, nullptr)) throw HelperException(LT("Error at debug trace write"));
    }
    catch (...) {
        return throw HelperException(LT("Error at debug trace write")); // log failed
    }
}


// log to console, return false on error
bool TraceLog::logToConsole(
    const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
) noexcept
{
    try {
        writeToLog(cout, i_msgTime, i_msg, i_extra);
    }
    catch (...) {
        return false;   // log failed
    }
    return true;
}

// log to file, return false on error
bool TraceLog::logToFile(
    bool i_isToStamped, const chrono::system_clock::time_point & i_msgTime, const char * i_msg, const char * i_extra
) noexcept
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
