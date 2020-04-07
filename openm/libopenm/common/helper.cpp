/**
 * @file
 * OpenM++ common helper functions.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "helper.h"

#ifdef _WIN32
    #define strNcmpNoCase   strnicmp
#else
    #define strNcmpNoCase   strncasecmp
#endif  // _WIN32

// convert string to lower case
void openm::toLower(string & io_str, const locale & i_locale)
{
    transform(
        io_str.begin(), 
        io_str.end(), 
        io_str.begin(), 
        bind(tolower<char>, placeholders::_1, i_locale)
        );
}

// convert string to lower case
void openm::toLower(char * io_str, const locale & i_locale)
{
    if (io_str != NULL) {
        use_facet<ctype<char> >(i_locale).tolower(io_str, io_str + strnlen(io_str, OM_STRLEN_MAX));
    }
}

// convert string to upper case
void openm::toUpper(string & io_str, const locale & i_locale)
{
    transform(
        io_str.begin(),
        io_str.end(),
        io_str.begin(),
        bind(toupper<char>, placeholders::_1, i_locale)
        );
}

// convert string to upper case
void openm::toUpper(char * io_str, const locale & i_locale)
{
    if (io_str != NULL) {
        use_facet<ctype<char> >(i_locale).toupper(io_str, io_str + strnlen(io_str, OM_STRLEN_MAX));
    }
}

// case neutral string comparison
int openm::compareNoCase(const char * i_left, const char * i_right, size_t i_length)
{
    return
        strNcmpNoCase(i_left != NULL ? i_left : "", i_right != NULL ? i_right : "", ((i_length > 0) ? i_length : OM_STRLEN_MAX));
}

// case neutral string equal comparison
bool openm::equalNoCase(const char * i_left, const char * i_right, size_t i_length)
{
    return compareNoCase(i_left, i_right, i_length) == 0;
}

// check if string start with i_start, using case neutral string comparison
bool openm::startWithNoCase(const string & i_str, const char * i_start)
{
    if (i_start == NULL) return false;

    string::size_type len = strnlen(i_start, OM_STRLEN_MAX);
    return 
        (i_str.length() >= len) && 
        0 == compareNoCase(i_str.c_str(), i_start, len);
}

// check if string end with i_end, using case neutral string comparison
bool openm::endWithNoCase(const string & i_str, const char * i_end)
{
    if (i_end == NULL) return false;

    string::size_type len = strnlen(i_end, OM_STRLEN_MAX);
    return 
        (i_str.length() >= len) && 
        0 == compareNoCase(i_str.c_str() + (i_str.length() - len), i_end);
}

// Trim leading space characters
string openm::trimLeft(const string & i_str, const locale & i_locale)
{
    for (string::const_iterator it = i_str.begin(); it != i_str.end(); ++it) {
        if (!isspace<char>(*it, i_locale)) return i_str.substr(it - i_str.begin());
    }
    return "";  // string is empty or has only space chars
}

// Trim trailing space characters
string openm::trimRight(const string & i_str, const locale & i_locale)
{
    for (string::const_reverse_iterator revIt = i_str.rbegin(); revIt != i_str.rend(); ++revIt) {
        if (!isspace<char>(*revIt, i_locale)) return i_str.substr(0, i_str.rend() - revIt);
    }
    return "";  // string is empty or has only space chars
}

// Trim leading and trailing space characters
string openm::trim(const string & i_str, const locale & i_locale)
{
    string::const_iterator begIt;
    for (begIt = i_str.begin(); begIt != i_str.end(); ++begIt) {
        if (!isspace<char>(*begIt, i_locale)) break;
    }

    if (begIt == i_str.end()) return "";  // string is empty or has only space chars

    string::const_reverse_iterator endIt;
    for (endIt = i_str.rbegin(); endIt != i_str.rend(); ++endIt) {
        if (!isspace<char>(*endIt, i_locale)) break;
    }

    return i_str.substr(begIt - i_str.begin(), (i_str.rend() - endIt) - (begIt - i_str.begin()));
}

// Replace all <cr> or <lf> with blank space character
void openm::blankCrLf(string & io_str)
{
    for (string::value_type & it : io_str) {
        if (it == '\r' || it == '\n') it = '\x20';
    }
}

/** make quoted string using sql single ' quote by default, ie: 'O''Brien' */
const string openm::toQuoted(const string & i_str, char i_quote)
{
    string sRet;
    for (string::const_iterator it = i_str.begin(); it != i_str.end(); ++it) {
        sRet += *it;
        if (*it == i_quote) sRet += i_quote;
    }
    return i_quote + sRet + i_quote;
}

/** make unquoted string using sql single ' quote by default, ie: 'O''Brien' into O'Brien */
const string openm::toUnQuoted(const string & i_str, char i_quote, const locale & i_locale)
{
    // trim and remove surrounding quotes
    string sVal = trim(i_str, i_locale);

    size_t nLen = sVal.length();
    bool isQuoted = nLen >= 2 && sVal[0] == i_quote && sVal[nLen - 1] == i_quote;

    if (!isQuoted) return sVal;         // string is not quoted
    sVal = sVal.substr(1, nLen - 2);    // remove surrounding quotes

    string sRet;
    bool isPrevQuote = false;

    for (string::const_iterator it = sVal.begin(); it != sVal.end(); ++it) {
        if (*it != i_quote) {
            sRet += *it;
            isPrevQuote = false;
        }
        else {
            if (!isPrevQuote) sRet += *it;
            isPrevQuote = !isPrevQuote;
        }
    }
    return sRet;
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
const string openm::toAlphaNumeric(const string & i_str, int i_maxSize)
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

// replace all non non-printable and any of "'`$}{@><:|?*&^;/\ by _ underscore
const string openm::cleanPathChars(const string & i_str, int i_maxSize)
{
    string sRet;
    int len = 0;

    for (char nowCh : i_str) {

        bool isOk = isprint(nowCh, locale::classic());
        if (isOk) {
            for (char c : "\"'`$}{@><:|?*&^;/\\") {
                if (isOk = nowCh != c; !isOk) break;
            }
        }

        sRet += isOk ? nowCh : '_';
        len++;

        if (i_maxSize > 0 && len >= i_maxSize) return sRet;     // if max size supplied then return up to max size chars
    }
    return sRet;
}

// make date-time string, ie: 2012-08-17 16:04:59.148
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
        setw(3) << chrono::duration_cast<chrono::milliseconds>(i_time.time_since_epoch()).count() % 1000LL << flush;

    return st.str();
}

// make timestamp string, ie: 2012_08_17_16_04_59_148
const string openm::makeTimeStamp(const chrono::system_clock::time_point & i_time)
{
    time_t sys_time = chrono::system_clock::to_time_t(i_time);
    tm * sys_tm = localtime(&sys_time);

    ostringstream st;
    st << setfill('0') << 
        setw(4) << sys_tm->tm_year + 1900 << '_' << setw(2) << sys_tm->tm_mon + 1 << '_' << setw(2) << sys_tm->tm_mday <<
        '_' <<
        setw(2) << sys_tm->tm_hour << '_' << setw(2) << sys_tm->tm_min << '_' << setw(2) << sys_tm->tm_sec <<
        '_' << 
        setw(3) << chrono::duration_cast<chrono::milliseconds>(i_time.time_since_epoch()).count() % 1000LL << flush;

    return st.str();
}

// make date-time string from timestamp string, ie: 2012_08_17_16_04_59_148 => 2012-08-17 16:04:59.148
const string openm::toDateTimeString(const string & i_timestamp)
{
    string dtStr = i_timestamp;
    size_t dtLen = dtStr.length();

    if (dtLen >= 19) {
        dtStr[4] = '-';
        dtStr[7] = '-';
        dtStr[10] = ' ';
        dtStr[13] = ':';
        dtStr[16] = ':';
    }
    if (dtLen >= 21) dtStr[19] = '.';

    return dtStr;
}

// return number of milliseconds since epoch to measure intervals
int64_t openm::getMilliseconds(void)
{
#ifndef _WIN32
    const int64_t NANO_PER_SECOND = 1000000000;
    const int64_t MILLI_PER_NANO = 1000000;

    struct timespec ts;
    if (!timespec_get(&ts, TIME_UTC)) return 0;

    return ((int64_t)ts.tv_sec * NANO_PER_SECOND + (int64_t)ts.tv_nsec) / MILLI_PER_NANO;
#else

    return chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
#endif
}

// format message into supplied buffer using vsnprintf()
void openm::formatTo(size_t i_size, char * io_buffer, const char * i_format, va_list io_args)
{
    // if buffer is null or too short or too long (error in size) then exit
    if (i_size < 1 || i_size >= OM_STRLEN_MAX || io_buffer == NULL) return;

    // if format is empty then return empty "" string
    io_buffer[0] = '\0';
    if (i_format == NULL) return;

    // format message
    vsnprintf(io_buffer, i_size, i_format, io_args);
    io_buffer[i_size - 1] = '\0';
}

/** if source string exceed max size than return ellipted copy into the buffer */
const char * openm::elliptString(const char * i_src, size_t i_size, char * io_buffer)
{
    if (i_src == nullptr || i_src[0] == '\0') return "";        // source is empty, return empty
    if (i_size < 1 || i_size >= OM_STRLEN_MAX) return "";       // size is too short or too long (error in size)
    if (strnlen(i_src, i_size + 1) <= i_size) return i_src;     // source does not exceed max size, return as is
    if (io_buffer == nullptr) return "";                        // buffer is null, return empty

    // return ellipted copy of source
    strncpy(io_buffer, i_src, i_size - 3);
    io_buffer[i_size - 1] = io_buffer[i_size - 2] = io_buffer[i_size - 3] = '.';
    io_buffer[i_size] = '\0';

    return io_buffer;
}

// this function exists only because g++ below 4.9 does not support std::regex
//
// replace all occurence of i_oldValue by i_newValue, both old and new values must be not empty
const string openm::replaceAll(const string & i_src, const char * i_oldValue, const char * i_newValue)
{
    if (i_oldValue == NULL || i_oldValue[0] == '\0') return i_src;  // nothing to find
    if (i_newValue == NULL || i_newValue[0] == '\0') return i_src;  // nothing to replace

    size_t len = strnlen(i_oldValue, OM_STRLEN_MAX);
    string dst = i_src;

    bool isFound = false;
    do {
        size_t pos = dst.find(i_oldValue);
        isFound = pos != string::npos;

        if (isFound) dst = dst.replace(pos, len, i_newValue);

    } while (isFound);

    return dst;
}

// this function exists only because g++ below 4.9 does not support std::regex
#ifdef _WIN32

string openm::regexReplace(const string & i_srcText, const char * i_pattern, const char * i_replaceWith)
{
    regex pat(i_pattern);
    return regex_replace(i_srcText, pat, i_replaceWith);
}

#else

#include <regex.h>
#include "libopenm/omError.h"

#define MAX_RE_ERROR_MSG    1024

string openm::regexReplace(const string & i_srcText, const char * i_pattern, const char * i_replaceWith)
{
    // prepare regex
    regex_t re;

    int nRet = regcomp(&re, i_pattern, REG_EXTENDED | REG_NEWLINE);
    if (nRet != 0) {
        char errMsg[MAX_RE_ERROR_MSG + 1];
        regerror(nRet, &re, errMsg, sizeof(errMsg));
        throw HelperException("Regular expression error: %s", errMsg);
    }

    // replace first occurrence of pattern in source text
    string sResult;
    regmatch_t matchPos;

    nRet = regexec(&re, i_srcText.c_str(), 1, &matchPos, 0);
    if (nRet == REG_NOMATCH) {
        regfree(&re);           // cleanup
        return i_srcText;       // pattern not found - return source text as is
    }
    if (nRet != 0) {            // error 
        regfree(&re);           // cleanup
        throw HelperException("Regular expression FAILED: %s", i_pattern);
    }
    // at this point nRet == 0 and we can replace first occurrence with replacement string
    if (matchPos.rm_so >= 0 && matchPos.rm_so < (int)i_srcText.length()) {

        string sResult =
            i_srcText.substr(0, matchPos.rm_so) +
            i_replaceWith +
            ((matchPos.rm_eo >= 0 && matchPos.rm_eo < (int)i_srcText.length()) ? i_srcText.substr(matchPos.rm_eo) : "");

        regfree(&re);       // cleanup
        return sResult;
    }
    else {                  // starting offset out of range - pattern not found
        regfree(&re);       // cleanup
        return i_srcText;   // return source text as is
    }
}

#endif // _WIN32

// normalize language name by removing encoding part, replace _ by - and lower case: "en-ca" from "en_CA.UTF-8"
const string openm::normalizeLanguageName(const string & i_srcLanguage)
{
    string lang = replaceAll(i_srcLanguage, "_", "-");

    size_t dotPos = lang.find_last_of('.');
    if (dotPos != string::npos && dotPos < lang.length()) lang = lang.substr(0, dotPos);

    toLower(lang);
    return lang;
}

// normalize language name and split it into list of prefered languages: en_CA => [en-ca, en] */
const list<string> openm::splitLanguageName(const string & i_srcLanguage)
{
    // normalize: convert form en_CA.UTF-8 into en-ca
    string lang = normalizeLanguageName(i_srcLanguage);

    // shorten language code: simple (naive) way to get more generic language
    list<string> langLst;
    while (!lang.empty()) {

        langLst.push_back(lang);

        size_t np = lang.find_last_of("-_");
        if (np == string::npos || np <= 0 || np >= lang.length()) break;
        lang = lang.substr(0, np);
    }
    return langLst;
}

#ifndef _WIN32

// get user prefered locale name: en-CA en-CA or en_CA.UTF-8 or empty "" string on error
const string openm::getDefaultLocaleName(void)
{
    return locale("").name();
}

#else   // _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// get user prefered locale name: en-CA en-CA or en_CA.UTF-8 or empty "" string on error
const string openm::getDefaultLocaleName(void)
{
    // try user locale and on error system default locale
    string name;
    wchar_t wlcn[LOCALE_NAME_MAX_LENGTH + 1];

    if (GetUserDefaultLocaleName(wlcn, LOCALE_NAME_MAX_LENGTH) <= 0) {
        if (GetSystemDefaultLocaleName(wlcn, LOCALE_NAME_MAX_LENGTH) <= 0) return name; // empty value on error
    }
    wlcn[LOCALE_NAME_MAX_LENGTH] = '\0';

    // convert from Windows wchar to normal string
    char lcn[LOCALE_NAME_MAX_LENGTH + 1];

    size_t nLcn = std::wcstombs(lcn, wlcn, LOCALE_NAME_MAX_LENGTH);
    if (nLcn >= LOCALE_NAME_MAX_LENGTH || nLcn == static_cast<size_t>(-1)) return name; // empty value on error

    lcn[LOCALE_NAME_MAX_LENGTH] = '\0';
    
    name = lcn;
    return name;
}
#endif  // _WIN32

