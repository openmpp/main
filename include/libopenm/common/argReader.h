/**
 * @file
 * OpenM++ arguments reader: parse command line and ini-file arguments
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#ifndef OM_H_ARG_READER_H
#define OM_H_ARG_READER_H

#include "omHelper.h"

using namespace std;

namespace openm
{
    /** common openM++ keys for runtime arguments */
    struct ArgKey
    {
        /** run ini file name, ie: model.ini */
        static constexpr const char * iniFile = "OpenM.IniFile";

        /** if true then allow any key in the ini-file, read all ini-file entries and pass it to the model */
        static constexpr const char * iniAnyKey = "OpenM.IniAnyKey";

        /** process run stamp, usually time stamp  */
        static constexpr const char * runStamp = "OpenM.RunStamp";

        /** log to console */
        static constexpr const char * logToConsole = "OpenM.LogToConsole";

        /** log to file */
        static constexpr const char * logToFile = "OpenM.LogToFile";

        /** log file path */
        static constexpr const char * logFilePath = "OpenM.LogFilePath";

        /** log to "stamped" file */
        static constexpr const char * logToStamped = "OpenM.LogToStampedFile";

        /** use time-stamp in log "stamped" file name */
        static constexpr const char * logUseTs = "OpenM.LogUseTimeStamp";

        /** use pid-stamp in log "stamped" file name */
        static constexpr const char * logUsePid = "OpenM.LogUsePidStamp";

        /** do not prefix log messages with date-time */
        static constexpr const char * logNoMsgTime = "OpenM.LogNoMsgTime";

        /** log sql */
        static constexpr const char * logSql = "OpenM.LogSql";
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
        ~ArgReader(void) noexcept { }

        /** arguments as key-value string pairs with case-neutral search */
        NoCaseMap args;

        /** return true if option with this key exist. */
        bool isOptionExist(const char * i_key) const noexcept;

        /** return string value by key or default value if not found. */
        string strOption(const char * i_key, const string & i_default = "") const noexcept;

        /** return boolean value by key or false if not found or value not "yes", "1", "true" (case insensitive). */
        bool boolOption(const char * i_key) const noexcept;

        /**
        * return  1 if key found and value is one of: "yes", "1", "true" or empty value,
        * return  0 if key found and value is one of: "no", "0", "false",
        * return -1 if key not found,
        * return -2 otherwise.
        */
        int boolOptionToInt(const char * i_key) const noexcept;
           
        /** return int value by key or default if not found or can not be converted to int. */
        int intOption(const char * i_key, int i_default) const noexcept { return (int)longOption(i_key, i_default); }

        /** return long value by key or default if not found or can not be converted to long. */
        long long longOption(const char * i_key, long long i_default) const noexcept;

        /** return double value by key or default if not found or can not be converted to double. */
        double doubleOption(const char * i_key, double i_default) const noexcept;

        /** get run options from command line, ie: number of cases. */
        void parseCommandLine(
            int argc,
            char ** argv,
            bool i_isThrowUnknown,
            bool i_isStoreUnknown,
            const size_t i_keyArrSize,
            const char ** i_keyArr,
            const size_t i_shortArrSize = 0,
            const pair<const char *, const char *> * i_shortPairArr = nullptr,
            const size_t i_prefixSize = 0,
            const char ** i_prefixArr = nullptr,
            const size_t i_suffixSize = 0,
            const char ** i_suffixArr = nullptr
        );

        /** read ini-file and merge command line arguments. */
        void loadIniFile(
            const char * i_filePath,
            const size_t i_keyArrSize, 
            const char ** i_keyArr,
            bool i_isStoreUnknown = false,
            const size_t i_sectionToMergeSize = 0,
            const char ** i_sectionToMergeArr = nullptr,
            const size_t i_multiKeySize = 0,
            const char ** i_multiKeyArr = nullptr
        );

        /** adjust log file settings, ie: make default log file path if required. */
        void adjustLogSettings(int argc, char ** argv);
    };
}

#endif  // OM_H_ARG_READER_H
