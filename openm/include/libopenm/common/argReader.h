/**
 * @file
 * OpenM++ arguments reader: parse command line and ini-file arguments
 */
// Copyright (c) 2013-2014 OpenM++
// This code is licensed under MIT license (see LICENSE.txt for details)

#ifndef OM_ARG_READER_H
#define OM_ARG_READER_H

#include "libopenm/common/omHelper.h"

using namespace std;

namespace openm
{
    /** common openM++ keys for runtime arguments */
    struct ArgKey
    {
        /** run options file name, ie: model.ini */
        static const char * optionsFile;

        /** log to console */
        static const char * logToConsole;

        /** log to file */
        static const char * logToFile;

        /** log file path */
        static const char * logFilePath;

        /** log to "stamped" file */
        static const char * logToStamped;

        /** use time-stamp in log "stamped" file name */
        static const char * logUseTs;

        /** use pid-stamp in log "stamped" file name */
        static const char * logUsePid;

        /** log sql */
        static const char * logSql;
    };

    /** arguments reader to get runtime arguments from command line and ini-file. */
    struct ArgReader
    {
        /** initialize empty list of run arguments. */
        ArgReader(void) { }

        /** initialize run arguments from command line and ini-file. */
        ArgReader(
            int argc, 
            char ** argv, 
            const size_t i_keyArrSize, 
            const char ** i_keyArr, 
            const size_t i_shortArrSize = 0,
            const pair<const char *, const char *> * i_shortPairArr = nullptr
            );

        /** cleanup argument holder resources */
        ~ArgReader(void) throw() { }

        /** arguments as key-value string pairs with case-neutral search */
        NoCaseMap args;

        /** return true if option with this key exist. */
        bool isOptionExist(const char * i_key) const throw();

        /** return string value by key or default value if not found. */
        string strOption(const char * i_key, const string & i_default = "") const throw();

        /** return boolean value by key or false if not found or value not "yes", "1", "true". */
        bool boolOption(const char * i_key) const throw();

        /** return int value by key or default if not found or can not be converted to int. */
        int intOption(const char * i_key, int i_default) const throw() { return (int)longOption(i_key, i_default); }

        /** return long value by key or default if not found or can not be converted to long. */
        long long longOption(const char * i_key, long long i_default) const throw();

        /** return double value by key or default if not found or can not be converted to double. */
        double doubleOption(const char * i_key, double i_default) const throw();

        /** get run options from command line, ie: number of cases. */
        void parseCommandLine(
            int argc, 
            char ** argv, 
            bool i_isThrowUnknown, 
            bool i_isStoreUnknown,
            const size_t i_keyArrSize, 
            const char ** i_keyArr, 
            const size_t i_shortArrSize = 0, 
            const pair<const char *, const char *> * i_shortPairArr = nullptr
            );

        /** read ini-file and merge command line arguments. */
        void loadIniFile(
            const char * i_filePath,
            const size_t i_keyArrSize, 
            const char ** i_keyArr
            );

        /** adjust log file settings, ie: make default log file path if required. */
        void adjustLogSettings(int argc, char ** argv);
    };
}

#endif  // OM_ARG_READER_H
