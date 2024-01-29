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
#include <set>
#include <string>
#include <stdexcept>
#include <thread>
#include <iomanip>
#include <sstream>
#include <climits>
#include <vector>
#include <cstdarg>

// define max size of file path
#ifdef PATH_MAX
    #define OM_PATH_MAX (FILENAME_MAX < PATH_MAX ? FILENAME_MAX : PATH_MAX)
#endif
#ifndef OM_PATH_MAX
    #ifdef _MAX_PATH
        #define OM_PATH_MAX (FILENAME_MAX < _MAX_PATH ? FILENAME_MAX : _MAX_PATH)
    #else
        #define OM_PATH_MAX (FILENAME_MAX < 4096 ? FILENAME_MAX : 4096)
    #endif 
#endif 

#define OM_STRLEN_MAX           INT_MAX /** max string length supported */
#define OM_STR_DB_MAX           32000   /** max database string length: notes varchar (clob, text) */
#define OM_CODE_DB_MAX          32      /** max database length for codes: language code, digests, date-time string */
#define OM_NAME_DB_MAX          255     /** max database length for names: parameter name, table name, etc. */
#define OM_DESCR_DB_MAX         255     /** max database length for description: parameter description, table description, etc. */
#define OM_OPTION_DB_MAX        32000   /** max database length for option value: profile_option, run_option */
#define OM_SQL_EXPR_DB_MAX      2048    /** max database length for sql expression: accumulator or output table expression */
#define OM_MAX_BUILTIN_TYPE_ID  100     /** max type id for built-int types, ie: int, double, logical */

#define OM_STATE_SAVE_TIME 1811L        /** msec, interval between run state save if any changes in run status */
#define OM_STATE_BEAT_TIME 1823L        /** msec, interval between run state heart beats update */

namespace openm
{
    /** convert string to lower case */
    extern void toLower(std::string & io_str, const std::locale & i_locale = std::locale(""));

    /** convert string to lower case */
    extern void toLower(char * io_str, const std::locale & i_locale = std::locale(""));

    /** convert string to upper case */
    extern void toUpper(std::string & io_str, const std::locale & i_locale = std::locale(""));

    /** convert string to upper case */
    extern void toUpper(char * io_str, const std::locale & i_locale = std::locale(""));

    /** case neutral string comparison */
    extern int compareNoCase(const char * i_left, const char * i_right, size_t i_length = 0);

    /** case neutral string equal comparison */
    extern bool equalNoCase(const char * i_left, const char * i_right, size_t i_length = 0);

    /** check if string start with i_start, using case neutral string comparison */
    extern bool startWithNoCase(const std::string & i_str, const char * i_start);

    /** check if string end with i_end, using case neutral string comparison */
    extern bool endWithNoCase(const std::string & i_str, const char * i_end);

    /** trim leading blank and space characters */
    extern std::string trimLeft(const std::string & i_str, const std::locale & i_locale = std::locale(""));

    /** trim trailing blank and space characters */
    extern std::string trimRight(const std::string & i_str, const std::locale & i_locale = std::locale(""));

    /** trim leading and trailing blank and space characters */
    extern std::string trim(const std::string & i_str, const std::locale & i_locale = std::locale(""));

    /** replace all <cr> or <lf> with blank space character */
    extern void blankCrLf(std::string & io_str);

    /** make quoted string using sql single ' quote by default, ie: 'O''Brien' */
    extern const std::string toQuoted(const std::string & i_str, char i_quote = '\'');

    /** make unquoted string using sql single ' quote by default, ie: 'O''Brien' into O'Brien */
    extern const std::string toUnQuoted(const std::string & i_str, char i_quote = '\'', const std::locale & i_locale = std::locale(""));

    /** convert float type to string: exist to fix std::to_string conversion losses. */
    extern const std::string toString(double i_val);

    /** replace all non [A-Z,a-z,0-9] by _ underscore and remove repetitive underscores. */
    extern const std::string toAlphaNumeric(const std::string & i_str, int i_maxSize = 0);

    /** replace all non non-printable and any of "'`$}{@><:|?*&^;/\ by _ underscore. */
    extern const std::string cleanPathChars(const std::string & i_str, int i_maxSize = 0);

    /** replace all occurence of i_oldValue by i_newValue, both old and new values must be not empty */
    extern const std::string replaceAll(const std::string & i_src, const char * i_oldValue, const char * i_newValue);

    /** return true if model type is boolean (logical) */
    extern bool isBoolType(const char * i_typeName);

    /** return true if model type is string (varchar) */
    extern bool isStringType(const char * i_typeName);

    /** return true if model type is bigint (64 bit) */
    extern bool isBigIntType(const char * i_typeName);

    /** return true if model type is integer: not float, string, boolean, bigint
    * (if type is not a built-in then it must be integer enums)
    */
    extern bool isIntType(const char * i_typeName, int i_typeId);

    /** return true if model type id is built-in type id, ie: int, double, logical */
    extern bool isBuiltInType(int i_typeId);

    /** return true if model type is float (float, real, double or time) */
    extern bool isFloatType(const char * i_typeName);

    /** return true if model type is Time */
    extern bool isTimeType(const char * i_typeName);

    /** return true if i_value string represent valid integer constant */
    extern bool isIntValid(const char* i_value);

    /** return true if i_value string represent valid floating point constant */
    extern bool isFloatValid(const char* i_value);

    /** return true if lower case of source string one of: "yes" "1" "true" */
    extern bool isBoolTrue(const char* i_str);

    /** return true if lower case of source string one of: "yes" "1" "true" "no" "0" "false" */
    extern bool isBoolValid(const char* i_str);

    /** convert i_value string represnting boolean option and return one of:    \n
    * return  1 if i_value is one of: "yes", "1", "true" or empty "" value,     \n
    * return  0 if i_value is one of: "no", "0", "false",                       \n
    * return -1 if i_value is nullptr,                                          \n
    * return -2 otherwise.
    */
    extern int boolStringToInt(const char * i_value);

    /** copy bytes source into destination and return next destination offset */
    ptrdiff_t memCopyTo(uint8_t * io_dst, ptrdiff_t i_offset, const void * i_src, size_t i_size);

    /** make date-time string, ie: 2012-08-17 16:04:59.148 */
    extern const std::string makeDateTime(const std::chrono::system_clock::time_point & i_time);

    /** make timestamp string, ie: 2012_08_17_16_04_59_148 */
    extern const std::string makeTimeStamp(const std::chrono::system_clock::time_point & i_time);

    /** make date-time string from timestamp string, ie: 2012_08_17_16_04_59_148 => 2012-08-17 16:04:59.148 */
    extern const std::string toDateTimeString(const std::string & i_timestamp);

    /** format message into supplied buffer using vsnprintf() */
    extern void formatTo(size_t i_size, char * io_buffer, const char * i_format, va_list io_args);

    /** format message into string result using vsnprintf() */
    extern std::string formatToString(const std::string format, ...);

    /** if source string exceed max size than return ellipted copy into the buffer */
    extern const char * elliptString(const char * i_src, size_t i_size, char * io_buffer);

    /** normalize language name by removing encoding part, replace _ by - and lower case: "en-ca" from "en_CA.UTF-8" */
    extern const std::string normalizeLanguageName(const std::string & i_srcLanguage);

    /** normalize language name and split it into list of prefered languages: en_CA => [en-ca, en] */
    extern const std::list<std::string> splitLanguageName(const std::string & i_srcLanguage);

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
    extern std::list<std::string> splitCsv(
        const std::string & i_values, const char * i_delimiters = ",", bool i_isUnquote = false, char i_quote = '\''
    );

    /**
    * split and trim comma-separated list of values (other delimiters can be used too, ie: semicolon).
    *
    * RFC 4180 difference: it does skip space-only lines and trim values unless it is " quoted ".
    *
    * @param[in] i_values         source string of comma separated values.
    * @param[in,out] io_resultLst source string of comma separated values.
    * @param[in] i_delimiters     list of delimiters, default: comma.
    * @param[in] i_isUnquote      if true then do "unquote ""csv"" ", default: false.
    * @param[in] i_quote          quote character, default: sql single ' quote.
    * @param[in] i_locale         locale to trim space characters, defaut: user default locale.
    */
    extern void splitCsv(
        const std::string & i_values,
        std::list<std::string> & io_resultLst,
        const char * i_delimiters = ",", 
        bool i_isUnquote = false, 
        char i_quote = '\'', 
        const std::locale & i_locale = std::locale(""));

    /** case neutral less comparator for strings */
    struct LessNoCase
    {
        bool operator() (const std::string & i_left, const std::string & i_right) const
        {
            return compareNoCase(i_left.c_str(), i_right.c_str()) < 0;
        }
    };

    /** map of key-value strings with case neutral key search */
    typedef std::map<std::string, std::string, LessNoCase> NoCaseMap;

    /** set of strings with case neutral key search */
    typedef std::set<std::string, LessNoCase> NoCaseSet;

    /** db-row abstract base */
    struct IRowBase
    {
        virtual ~IRowBase(void) noexcept = 0;
    };

    /** unique pointer to db row */
    typedef std::unique_ptr<IRowBase> IRowBaseUptr;

    /** db rows: vector of unique pointers to db row */
    typedef std::vector<IRowBaseUptr> IRowBaseVec;

    /** db rows: list of unique pointers to db row */
    typedef std::list<IRowBaseUptr> IRowBaseList;

    /** final model run state public interface: thread safe */
    struct IFinalState
    {
        virtual ~IFinalState(void) noexcept = 0;

        /** return true if status is one of exiting: done, exit, error */
        virtual bool isFinal(void) = 0;

        /** return true if status is an error */
        virtual bool isError(void) = 0;
    };

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

        /** release resource if exit guard hold() was not called */
        ~exit_guard(void) noexcept
        {
            try {
                if (!isHold) (holder->*cleanup)();
            }
            catch (...) { }
        }

        /** hold resource on exit instead of relesing it */
        void hold(void) noexcept { isHold = true; }

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
