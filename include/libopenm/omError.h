/**
 * @file
 * OpenM++: public interface for errors and exceptions.
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_H_ERROR_H
#define OM_H_ERROR_H

#include <cstdarg>
#include "common/omHelper.h"   // define formatTo
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
        const char * what(void) const throw() { return msg; }

    private:
        char msg[maxMsgSize];   // error message
    };

    /** helper library exception default error message: "unknown error in helper method" */
    extern const char helperUnknownErrorMessage[];   

    /** helper library exception */
    typedef OpenmException<4000, helperUnknownErrorMessage> HelperException;
}

#endif  // OM_H_ERROR_H
