/**
 * @file
 * OpenM++: public interface for errors and exceptions.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_H_ERROR_H
#define OM_H_ERROR_H

#include <cstdarg>
#include "common/omHelper.h"
using namespace std;

namespace openm
{
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
        const char * what(void) const noexcept { return msg; }

    private:
        char msg[maxMsgSize];   // error message
    };

    /** helper library exception default error message: "unknown error in helper method" */
    extern const char helperUnknownErrorMessage[];   

    /** helper library exception */
    typedef OpenmException<4000, helperUnknownErrorMessage> HelperException;

    /** helper to return ellipted string if source string exceed max size.
    * it may return pointer to shared buffer and can be used only once per error message.
    */
    struct MessageEllipter 
    {
        /** if source string exceed max size than return ellipted copy.
        * it may return pointer to shared buffer and can be used only once per error message.
        */
        const char * ellipt(const char * i_src) { return elliptString(i_src, maxItemSize, msgItem); }

        /** if source string exceed max size than return ellipted copy.
        * it may return pointer to shared buffer and can be used only once per error message.
        */
        const char * ellipt(const string & i_src) { return elliptString(i_src.c_str(), maxItemSize, msgItem); }

    private:
        static const size_t maxItemSize = 40;   // max size of string item inside of error message
        char msgItem[maxItemSize + 1];          // string item inside of error message, truncated to max size and ellipted
    };
}

#endif  // OM_H_ERROR_H
