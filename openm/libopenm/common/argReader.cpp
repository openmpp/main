/**
 * @file
 * OpenM++ arguments reader: parse command line and ini-file arguments
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/omError.h"
#include "libopenm/common/omFile.h"
#include "libopenm/common/argReader.h"
#include "libopenm/common/iniReader.h"

using namespace std;
using namespace openm;

namespace openm
{
    /** run options file name, ie: model.ini */
    const char * ArgKey::optionsFile = "OpenM.OptionsFile";

    /** log to console */
    const char * ArgKey::logToConsole = "OpenM.LogToConsole";

    /** log to file */
    const char * ArgKey::logToFile = "OpenM.LogToFile";

    /** log file path */
    const char * ArgKey::logFilePath = "OpenM.LogFilePath";

    /** log to "stamped" file */
    const char * ArgKey::logToStamped = "OpenM.LogToStampedFile";

    /** use time-stamp in log "stamped" file name */
    const char * ArgKey::logUseTs = "OpenM.LogUseTimeStamp";

    /** use pid-stamp in log "stamped" file name */
    const char * ArgKey::logUsePid = "OpenM.LogUsePidStamp";

    /** do not prefix log messages with date-time */
    const char * ArgKey::logNoMsgTime = "OpenM.LogNoMsgTime";

    /** log sql */
    const char * ArgKey::logSql = "OpenM.LogSql";
}

/**
 * initialize run arguments from command line and ini-file.
 * 
 * @param[in] argc             number of command line arguments
 * @param[in] argv             array of command line arguments
 * @param[in] i_keyArrSize     size of i_keyArr, must be positive
 * @param[in] i_keyArr         array of allowed keys: full key names
 * @param[in] i_shortArrSize   size of i_shortPairArr, can be zero
 * @param[in] i_shortPairArr   array of short and full key names to remap from short to full name
 */
ArgReader::ArgReader(
        int argc, 
        char ** argv, 
        const size_t i_keyArrSize, 
        const char ** i_keyArr, 
        const size_t i_shortArrSize,
        const pair<const char *, const char *> * i_shortPairArr
        )
{
    // get command line options
    parseCommandLine(argc, argv, true, false, i_keyArrSize, i_keyArr, i_shortArrSize, i_shortPairArr);

    // load options from ini-file
    loadIniFile(strOption(ArgKey::optionsFile).c_str(), i_keyArrSize, i_keyArr);

    // dependency in log options: if LogToFile is true then file name must be non-empty else must be empty
    adjustLogSettings(argc, argv);
}

/** return true if option with this key exist. */
bool ArgReader::isOptionExist(const char * i_key) const throw()
{
    try {
        return i_key != NULL && args.find(i_key) != args.cend();
    }
    catch (...) { 
        return false;
    }
}

/** return string value by key or default value if not found. */
string ArgReader::strOption(const char * i_key, const string & i_default) const throw()
{
    try {
        if (i_key == NULL) return i_default;    // invalid key

        NoCaseMap::const_iterator argIt = args.find(i_key);
        return 
            (argIt != args.cend()) ? argIt->second : i_default;
    }
    catch (...) { 
        return i_default;
    }
}

/** return boolean value by key or false if not found or value not "yes", "1", "true". */
bool ArgReader::boolOption(const char * i_key) const throw()
{
    try {
        string sVal = strOption(i_key);
        return 
            equalNoCase(sVal.c_str(), "1") || equalNoCase(sVal.c_str(), "yes") || equalNoCase(sVal.c_str(), "true");
    }
    catch (...) { 
        return false;
    }
}

/** return long value by key or default if not found or can not be converted to long. */
long long ArgReader::longOption(const char * i_key, long long i_default) const throw()
{
    try {
        string sVal = strOption(i_key);
        return 
            (sVal != "") ? std::stoll(sVal) : i_default;
    }
    catch (...) { 
        return i_default;
    }
}

/** return double value by key or default if not found or can not be converted to double. */
double ArgReader::doubleOption(const char * i_key, double i_default) const throw()
{
    try {
        string sVal = strOption(i_key);
        return 
            (sVal != "") ? std::stod(sVal) : i_default;
    }
    catch (...) { 
        return i_default;
    }
}

/**   
* get run options from command line, ie: number of cases.     
*
* @param[in] argc             number of command line arguments
* @param[in] argv             array of command line arguments
* @param[in] i_isThrowUnknown if true then throw exception when key is unknown (except i_prefixToCopy below)
* @param[in] i_isStoreUnknown if true then store unknown key values (always stored i_prefixToCopy below)
* @param[in] i_keyArrSize     size of i_keyArr, must be positive
* @param[in] i_keyArr         array of allowed keys: full key names
* @param[in] i_shortArrSize   size of i_shortPairArr, can be zero
* @param[in] i_shortPairArr   array of short and full key names to remap from short to full name
* @param[in] i_prefixToCopy   if not NULL then copy options started from "prefix." i.e.: "Parameter."
*/
void ArgReader::parseCommandLine(
    int argc, 
    char ** argv, 
    bool i_isThrowUnknown, 
    bool i_isStoreUnknown,
    const size_t i_keyArrSize, 
    const char ** i_keyArr, 
    const size_t i_shortArrSize, 
    const pair<const char *, const char *> * i_shortPairArr,
    const char * i_prefixToCopy
    )
{
    // if no command line arguments then return empty options
    if (argc <= 1) return;

    // validate parameters
    if (argc < 1 || argv == nullptr) throw HelperException("Invalid (empty) list of command line arguments, expected at least one");
    if (i_keyArrSize <= 0 || i_keyArrSize > SHRT_MAX || i_keyArr == nullptr) throw HelperException("Invalid (or empty) list of option keys");
    if (i_shortArrSize < 0 || i_shortArrSize > SHRT_MAX || (i_shortArrSize != 0 && i_shortPairArr == nullptr)) throw HelperException("Invalid (or empty) list of option short keys");

    // check if there prefix to copy specified, i.e.: "Parameter"
    bool isCopyPrefix = i_prefixToCopy != nullptr && i_prefixToCopy[0] != '\0';
    string copyPrefix = isCopyPrefix ? string(i_prefixToCopy) + "." : "";
    size_t nPrefix = copyPrefix.length();

    // past the end of option names array
    const char ** endOfKeyArr = i_keyArr + i_keyArrSize;
    const pair<const char *, const char *> * endOfShortArr = i_shortPairArr + i_shortArrSize;

    // find keys in the names array and append key-value pairs into result
    for (int nArg = 1; nArg < argc; nArg++) {

        // command line key must start from - or /
        string sKey = toUtf8(argv[nArg]);
        if (sKey.empty() || (sKey[0] != '-' && sKey[0] != '/')) throw HelperException("Invalid command line parameter %s", argv[nArg]);

        sKey = sKey.substr(1);  // remove leading - or /

        // if this is short name then replace it with full name
        const pair<const char *, const char *> * pairIt = std::find_if(
            i_shortPairArr, 
            endOfShortArr, 
            [sKey](const pair<const char *, const char *> i_pair) -> bool { return equalNoCase(i_pair.first, sKey.c_str()); }
            );

        if (pairIt != endOfShortArr) sKey = pairIt->second;

        // find command line key in the list of known keys
        const char ** keyIt = std::find_if(
            i_keyArr, 
            endOfKeyArr, 
            [sKey](const char * i_key) -> bool { return equalNoCase(i_key, sKey.c_str()); }
            );

        if (keyIt != endOfKeyArr) {
            sKey = *keyIt;  // key found: use "standard" key representation, which may be in a different case
        }
        else {              // key not found: raise error, store or ignore unknown key

            if (!isCopyPrefix || !equalNoCase(sKey.c_str(), copyPrefix.c_str(), nPrefix)) {
                if (i_isThrowUnknown) throw HelperException("Invalid command line parameter %s", argv[nArg]);
                if (!i_isStoreUnknown) continue;
            }
        }
        
        // get parameter value and save it in the run options map
        string sVal = "";
        if (nArg + 1 < argc) sVal = toUtf8(argv[++nArg]);

        args[sKey] = sVal;
    }
}

/**
* read ini-file and merge with command line arguments.
*
* @param[in] i_filePath       path to ini-file
* @param[in] i_keyArrSize     size of i_keyArr, must be positive
* @param[in] i_keyArr         array of allowed keys: full key names
* @param[in] i_sectionToMerge if not NULL then merge section from ini-file, i.e.: "Parameter"
*/
void ArgReader::loadIniFile(
    const char * i_filePath,
    const size_t i_keyArrSize,
    const char ** i_keyArr,
    const char * i_sectionToMerge
)
{
    // load options from ini-file
    IniFileReader iniRd(i_filePath);

    // merge ini-file options with command line options
    // append ini-file option only if it is not defined on command line
    for (size_t nKey = 0; nKey < i_keyArrSize; nKey++) {

        if (args.find(i_keyArr[nKey]) != args.cend()) continue;     // option specified at command line
        if (!iniRd.isExist(i_keyArr[nKey])) continue;               // option not exist in ini-file

        args[i_keyArr[nKey]] = iniRd.strValue(i_keyArr[nKey]);      // add from ini-file to options map
    }

    // merge pairs of (section.key, value) from section of ini-file if such section.key not already exists
    if (i_sectionToMerge != nullptr) {
        NoCaseMap sect = iniRd.getSection(i_sectionToMerge);
        for (const auto & ent : sect) {
            string key = string(i_sectionToMerge) + "." + ent.first;
            if (args.find(key) == args.cend()) args[key] = ent.second;
        }
    }
}

/** adjust log file settings, ie: make default log file path if required. */
void ArgReader::adjustLogSettings(int argc, char ** argv)
{
    // validate parameters
    if (argc < 1 || argv == nullptr) throw HelperException("Invalid (empty) list of command line arguments, expected at least one");

    // dependency in log options: if LogToFile is true then file name must be non-empty else must be empty
    if (boolOption(ArgKey::logToFile)) {
        string fn = strOption(ArgKey::logFilePath);
        if (fn.empty()) args[ArgKey::logFilePath] = makeDefaultPath(argv[0], ".log");
    }
    else {
        if (isOptionExist(ArgKey::logFilePath)) args[ArgKey::logFilePath] = "";
    }
}
