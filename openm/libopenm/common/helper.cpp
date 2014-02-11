// OpenM++ common helper utilities
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "libopenm/common/omHelper.h"

#ifdef _WIN32
    #define strcmpNoCase    stricmp
    #define strNcmpNoCase   strnicmp
#else
    #define strcmpNoCase    strcasecmp
    #define strNcmpNoCase   strncasecmp
#endif  // _WIN32

// helper exception default error message
const char openm:: helperUnknownErrorMessage[] = "unknown error in helper method";

// convert string to lower case
void openm::toLower(string & io_str)
{
    locale defaultLoc = locale("");
    transform(
        io_str.begin(), 
        io_str.end(), 
        io_str.begin(), 
        bind(tolower<char>, placeholders::_1, cref(defaultLoc))
        );
}

// convert string to lower case
void openm::toLower(char * io_str)
{
    if (io_str != NULL) 
        use_facet<ctype<char> >(locale("")).tolower(io_str, io_str + strlen(io_str));
}

// convert string to upper case
void openm::toUpper(string & io_str)
{
    locale defaultLoc = locale("");
    transform(
        io_str.begin(),
        io_str.end(),
        io_str.begin(),
        bind(toupper<char>, placeholders::_1, cref(defaultLoc))
        );
}

// convert string to upper case
void openm::toUpper(char * io_str)
{
    if (io_str != NULL) 
        use_facet<ctype<char> >(locale("")).toupper(io_str, io_str + strlen(io_str));
}

// case neutral string comparison
int openm::compareNoCase(const char * i_left, const char * i_right, int i_length)
{
    return (i_length <= 0) ?
        strcmpNoCase(i_left != NULL ? i_left : "", i_right != NULL ? i_right : "") :
        strNcmpNoCase(i_left != NULL ? i_left : "", i_right != NULL ? i_right : "", i_length);
}

// case neutral string equal comparison
bool openm::equalNoCase(const char * i_left, const char * i_right, int i_length)
{
    return compareNoCase(i_left, i_right, i_length) == 0;
}

// check if string end with i_end, using case neutral string comparison
bool openm::endWithNoCase(const string & i_str, const char * i_end)
{
    if (i_end == NULL) return false;

    string::size_type len = strlen(i_end);
    return 
        (i_str.length() >= len) && 
        0 == compareNoCase(i_str.c_str() + (i_str.length() - len), i_end);
}

// Trim leading space characters
string openm::trimLeft(const string & i_str)
{
    locale defaultLoc = locale("");
    const ctype<char> & chType = use_facet<ctype<char> >(defaultLoc);

    for (string::const_iterator it = i_str.begin(); it != i_str.end(); ++it) {
        if (!chType.is(ctype<char>::space, *it)) 
            return i_str.substr(i_str.begin() - it);
    }
    return "";  // string is empty or has only space chars
}

// Trim trailing space characters
string openm::trimRight(const string & i_str)
{
    locale defaultLoc = locale("");
    const ctype<char> & chType = use_facet<ctype<char> >(defaultLoc);

    for (string::const_reverse_iterator revIt = i_str.rbegin(); revIt != i_str.rend(); ++revIt) {
        if (!chType.is(ctype<char>::space, *revIt)) 
            return i_str.substr(0, i_str.rend() - revIt);
    }
    return "";  // string is empty or has only space chars
}

// Trim leading and trailing space characters
string openm::trim(const string & i_str)
{
    locale defaultLoc = locale("");
    const ctype<char> & chType = use_facet<ctype<char> >(defaultLoc);

    string::const_iterator begIt;
    for (begIt = i_str.begin(); begIt != i_str.end(); ++begIt) {
        if (!chType.is(ctype<char>::space, *begIt)) break;
    }

    if (begIt == i_str.end()) return "";  // string is empty or has only space chars

    string::const_reverse_iterator endIt;
    for (endIt = i_str.rbegin(); endIt != i_str.rend(); ++endIt) {
        if (!chType.is(ctype<char>::space, *endIt)) break;
    }

    return i_str.substr(begIt - i_str.begin(), i_str.rend() - endIt);
}

// make sql quoted string, ie: 'O''Brien'
const string openm::toQuoted(const string & i_str)
{
    string sRet;
    for (string::const_iterator it = i_str.begin(); it != i_str.end(); ++it) {
        sRet += *it;
        if (*it == '\'') sRet += '\'';
    }
    return '\'' + sRet + '\'';
}

/** convert float type to string: exist to fix std::to_string conversion losses. */
const string openm::toString(double i_val)
{
    ostringstream st;
    st.imbue(locale::classic());
    st << setprecision(numeric_limits<double>::max_digits10) << i_val << flush;
    return st.str();
}

// replace all non [A-Z,a-z,0-9] by _ underscore and remove repetitive underscores
string openm::toAlphaNumeric(const string & i_str, int i_maxSize)
{
    string sRet;
    bool isPrevUnder = false;
    int len = 0;

    for (char nowCh : i_str) {
        if (isalnum(nowCh, locale::classic()) ) {
            sRet += nowCh;
            len++;
            isPrevUnder = false;
        }
        else {
            if (!isPrevUnder) {
                sRet += '_';
                len++;
            }
            isPrevUnder = true;
        }
        if (i_maxSize > 0 && len >= i_maxSize) return sRet;     // if max size supplied then return up to max size chars
    }
    return sRet;
}

// replace all occurence of i_oldValue by i_newValue, both old and new values must be not empty
const string openm::replaceAll(const string & i_src, const char * i_oldValue, const char * i_newValue)
{
    if (i_oldValue == NULL || i_oldValue[0] == '\0') return i_src;  // nothing to find
    if (i_newValue == NULL || i_newValue[0] == '\0') return i_src;  // nothing to replace

    size_t len = strlen(i_oldValue);
    string dst = i_src;

    bool isFound = false;
    do {
        size_t pos = dst.find(i_oldValue);
        isFound = pos != string::npos;

        if (isFound) dst = dst.replace(pos, len, i_newValue);

    } while (isFound);

    return dst;
}

// make date-time string, ie: 2012-08-17 16:04:59.0148
const string openm::makeDateTime(const chrono::system_clock::time_point & i_time)
{
    time_t sys_time = chrono::system_clock::to_time_t(i_time);
    tm * sys_tm = localtime(&sys_time);

    ostringstream st;
    st << setfill('0') << 
        setw(4) << sys_tm->tm_year + 1900 << '-' << setw(2) << sys_tm->tm_mon + 1 << '-' << setw(2) << sys_tm->tm_mday << 
        ' ' <<
        setw(2) << sys_tm->tm_hour << ':' << setw(2) << sys_tm->tm_min << ':' << setw(2) << sys_tm->tm_sec << 
        '.' << 
        setw(4) << chrono::duration_cast<chrono::milliseconds>(i_time.time_since_epoch()).count() % 1000LL << flush;

    return st.str();
}

// make timestamp string, ie: _20120817_160459_0148
const string openm::makeTimeStamp(const chrono::system_clock::time_point & i_time)
{
    time_t sys_time = chrono::system_clock::to_time_t(i_time);
    tm * sys_tm = localtime(&sys_time);

    ostringstream st;
    st << '_' << setfill('0') << 
        setw(4) << sys_tm->tm_year + 1900 << setw(2) << sys_tm->tm_mon + 1 << setw(2) << sys_tm->tm_mday <<
        '_' <<
        setw(2) << sys_tm->tm_hour << setw(2) << sys_tm->tm_min << setw(2) << sys_tm->tm_sec <<
        '_' << 
        setw(4) << chrono::duration_cast<chrono::milliseconds>(i_time.time_since_epoch()).count() % 1000LL << flush;

    return st.str();
}

// make date-time string from timestamp string, ie: _20120817_160459_0148 => 2012-08-17 16:04:59.0148
const string openm::toDateTimeString(const string & i_timestamp)
{
    string dtStr = i_timestamp;
    size_t dtLen = dtStr.length();

    if (dtLen >= 16) {
        dtStr = 
            dtStr.substr(1, 4) + '-' +  dtStr.substr(5, 2) + '-' + dtStr.substr(7, 2) + ' ' +
            dtStr.substr(10, 2) + ':' + dtStr.substr(12, 2) + ':' + dtStr.substr(14, 2) +
            ((dtLen >= 18) ? '.' + dtStr.substr(17) : "");
    }

    return dtStr;
}

// sleep for specified number miliiseconds (exist only to fix gcc 4.4, it is fixed in gcc 4.8)
void openm::sleepMilli(long i_sleepTime)
{
#ifdef _WIN32
    chrono::milliseconds ms(i_sleepTime); 
    this_thread::sleep_for(ms);
#else
    timespec ms;
    ms.tv_sec = 0;
    ms.tv_nsec = i_sleepTime * 1000000L;
    nanosleep(&ms, NULL);
#endif  // _WIN32
}

// format message into supplied buffer using vsnprintf()
void openm::formatTo(size_t i_size, char * io_buffer, const char * i_format, va_list io_args)
{
    // if buffer is null or too short or too large (error in size) then exit
    if (i_size < 1 || i_size >= INT_MAX || io_buffer == NULL) return;

    // if format is empty then return empty "" string
    io_buffer[0] = '\0';
    if (i_format == NULL) return;

    // format message
    vsnprintf(io_buffer, i_size, i_format, io_args);
    io_buffer[i_size - 1] = '\0';
}
