/**
 * @file
 * OpenM++ common helper utilities.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_H_HELPER_H
#define OM_H_HELPER_H

#include <algorithm>
#include <cstring>
#include <ctime>
#include <chrono>
#include <functional>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <thread>
#include <iomanip>
#include <sstream>
#include <climits>
#include <vector>
#include <regex>

using namespace std;

namespace openm
{
    /** convert string to lower case */
    extern void toLower(string & io_str);

    /** convert string to lower case */
    extern void toLower(char * io_str);

    /** convert string to upper case */
    extern void toUpper(string & io_str);

    /** convert string to upper case */
    extern void toUpper(char * io_str);

    /** case neutral string comparison */
    extern int compareNoCase(const char * i_left, const char * i_right, size_t i_length = 0);

    /** case neutral string equal comparison */
    extern bool equalNoCase(const char * i_left, const char * i_right, size_t i_length = 0);

    /** check if string end with i_end, using case neutral string comparison */
    extern bool endWithNoCase(const string & i_str, const char * i_end);

    /** trim leading blank and space characters */
    extern string trimLeft(const string & i_str);

    /** trim trailing blank and space characters */
    extern string trimRight(const string & i_str);

    /** trim leading and trailing blank and space characters */
    extern string trim(const string & i_str);

    /** make quoted string using sql single ' quote by default, ie: 'O''Brien' */
    extern const string toQuoted(const string & i_str, char i_quote = '\'');

    /** make unquoted string using sql single ' quote by default, ie: 'O''Brien' into O'Brien */
    extern const string toUnQuoted(const string & i_str, char i_quote = '\'');

    /** convert float type to string: exist to fix std::to_string conversion losses. */
    extern const string toString(double i_val);

    /** replace all non [A-Z,a-z,0-9] by _ underscore and remove repetitive underscores. */
    string toAlphaNumeric(const string & i_str, int i_maxSize = 0);

    /** replace all occurence of i_oldValue by i_newValue, both old and new values must be not empty */
    extern const string replaceAll(const string & i_src, const char * i_oldValue, const char * i_newValue);

    /** make date-time string, ie: 2012-08-17 16:04:59.0148 */
    extern const string makeDateTime(const chrono::system_clock::time_point & i_time);

    /** make timestamp string, ie: _20120817_160459_0148 */
    extern const string makeTimeStamp(const chrono::system_clock::time_point & i_time);

    /** make date-time string from timestamp string, ie: _20120817_160459_0148 => 2012-08-17 16:04:59.0148 */
    extern const string toDateTimeString(const string & i_timestamp);

    /** format message into supplied buffer using vsnprintf() */
    extern void formatTo(size_t i_size, char * io_buffer, const char * i_format, va_list io_args);

    // this function exist only because g++ below 4.9 does not support std::regex
    extern string regexReplace(const string & i_srcText, const char * i_pattern, const char * i_replaceWith);

    /**   
    * split and trim comma-separated list of values (other delimiters can be used too, ie: semicolon).
    *
    * RFC 4180 difference: it does skip space-only lines and trim values unless it is " quoted ".
    *
    * @param[in] i_values       source string of comma separated values.
    * @param[in] i_delimiters   list of delimiters, default: comma.
    * @param[in] i_isUnquote    if true then do "unquote ""csv"" ", default: false.
    * @param[in] i_quote        quote character, default: sql single ' quote.
    */
    extern list<string> splitCsv(const string & i_values, const char * i_delimiters = ",", bool i_isUnquote = false, char i_quote = '\'');

    /** case neutral less comparator for strings */
    struct LessNoCase
    {
        bool operator() (const string & i_left, const string & i_right) const
        { 
            return compareNoCase(i_left.c_str(), i_right.c_str()) < 0; 
        }
    };

    /** map of key-value strings with case neutral key search */
    typedef map<string, string, LessNoCase> NoCaseMap;

    /** simple resource exit guard implementation */
    template <class ResourceHolder> 
    class exit_guard
    {
    public:

        /** resource release method of resource holder class */
        typedef void(ResourceHolder::*CleanupHandler)(void);

        /** create resource guard to release resource on block exit */
        exit_guard(ResourceHolder * const i_holder, CleanupHandler i_cleanupFnc) : 
            isHold(false),
            holder(i_holder), 
            cleanup(i_cleanupFnc)
            { }

        /** release resource if exit guad hold() was not called */
        ~exit_guard(void) throw() { 
            try {
                if (!isHold) (holder->*cleanup)(); 
            }
            catch(...) { }
        }

        /** hold resource on exit instead of relesing it */
        void hold(void) throw() { isHold = true; }

    private:
        bool isHold;                    // if true then hold resource
        ResourceHolder * const holder;  // resource holder class
        CleanupHandler cleanup;         // resource release method

    private:
        exit_guard(const exit_guard & i_guard) = delete;
        exit_guard & operator=(const exit_guard & i_guard) = delete;
    };
}

#endif  // OM_H_HELPER_H
