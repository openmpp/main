/**
 * @file
 * OpenM++ common helper utilities.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_HELPER_H
#define OM_HELPER_H

#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <chrono>
#include <functional>
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

    /** make sql quoted string, ie: 'O''Brien' */
    extern const string toQuoted(const string & i_str);

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

    /** sleep for specified number miliiseconds (exist only to fix gcc 4.4, it is fixed in gcc 4.8) */
    extern void sleepMilli(long i_sleepTime);

    /** format message into supplied buffer using vsnprintf() */
    extern void formatTo(size_t i_size, char * io_buffer, const char * i_format, va_list io_args);

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

    /** openM++ exceptions */
    template <size_t maxMsgSize, const char * defaultMessage>
    class OpenmException : public exception
    {
    public:

        /** create openM++ exception.  
        *
        * if i_format is NULL then use default error message \n
        * else make formatted message by vsnprintf()
        */
        OpenmException(const char * i_format, ...)
        {
            try {
                if (i_format == NULL) {
                    strncpy(msg, defaultMessage, maxMsgSize);
                }
                else {
                    va_list args;
                    va_start(args, i_format);
                    formatTo(maxMsgSize, msg, i_format, args);
                    va_end(args);
                }
                msg[maxMsgSize - 1] = '\0';
            } 
            catch (...) { }
        }

        /** return error message */
        const char * what(void) const throw() { return msg; }

    private:
        char msg[maxMsgSize];   // error message
    };

    /** helper library exception default error message: "unknown error in helper method" */
    extern const char helperUnknownErrorMessage[];   

    /** helper library exception */
    typedef OpenmException<4000, helperUnknownErrorMessage> HelperException;
}

#endif  // OM_HELPER_H
