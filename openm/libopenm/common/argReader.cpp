/**
 * @file
 * OpenM++ arguments reader: parse command line and ini-file arguments
 */
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/omLog.h"
#include "libopenm/omError.h"
#include "libopenm/common/omFile.h"
#include "libopenm/common/argReader.h"
#include "libopenm/common/iniReader.h"

using namespace std;
using namespace openm;

namespace openm
{
    /** run options file name, ie: model.ini */
    const char * ArgKey::iniFile = "OpenM.IniFile";

    /** process run stamp, usually time stamp  */
    const char * ArgKey::runStamp = "OpenM.RunStamp";

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
    loadIniFile(strOption(ArgKey::iniFile).c_str(), i_keyArrSize, i_keyArr);

    // dependency in log options: if LogToFile is true then file name must be non-empty else must be empty
    adjustLogSettings(argc, argv);
}

/** return true if option with this key exist. */
bool ArgReader::isOptionExist(const char * i_key) const noexcept
{
    try {
        return i_key != NULL && args.find(i_key) != args.cend();
    }
    catch (...) { 
        return false;
    }
}

/** return string value by key or default value if not found. */
string ArgReader::strOption(const char * i_key, const string & i_default) const noexcept
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

/** return boolean value by key or false if not found or value not "yes", "1", "true" (case insensitive). */
bool ArgReader::boolOption(const char * i_key) const noexcept
{
    try {
        return boolOptionToInt(i_key) > 0;
    }
    catch (...) { 
        return false;
    }
}

/** 
* return  1 if key found and value is one of: "yes", "1", "true" or empty value,
* return  0 if key found and value is one of: "no", "0", "false",
* return -1 if key not found, 
* return -2 otherwise.
*/
int ArgReader::boolOptionToInt(const char * i_key) const noexcept
{
    try {
        if (!isOptionExist(i_key)) return -1;   // not found

        string sVal = strOption(i_key);
        if (sVal.empty() || equalNoCase(sVal.c_str(), "1") || equalNoCase(sVal.c_str(), "yes") || equalNoCase(sVal.c_str(), "true")) return 1;
        if (equalNoCase(sVal.c_str(), "0") || equalNoCase(sVal.c_str(), "no") || equalNoCase(sVal.c_str(), "false")) return 0;

        return -2;  // incorrect value
    }
    catch (...) {
        return false;
    }
}

/** return long value by key or default if not found or can not be converted to long. */
long long ArgReader::longOption(const char * i_key, long long i_default) const noexcept
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
double ArgReader::doubleOption(const char * i_key, double i_default) const noexcept
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
* @param[in] argc               number of command line arguments
* @param[in] argv               array of command line arguments
* @param[in] i_isThrowUnknown   if true then throw exception when key is unknown (except i_prefixToCopy below)
* @param[in] i_isStoreUnknown   if true then store unknown key values (always stored i_prefixToCopy below)
* @param[in] i_keyArrSize       size of i_keyArr, must be positive
* @param[in] i_keyArr           array of allowed keys: full key names
* @param[in] i_shortArrSize     size of i_shortPairArr, can be zero
* @param[in] i_shortPairArr     array of short and full key names to remap from short to full name
* @param[in] i_prefixSize       size of options "prefix." array to be allowed
* @param[in] i_prefixArr        allow options started from "prefix." i.e.: "Parameter." or "SubValue."
* @param[in] i_suffixSize       size of options ".suffix" array to be allowed
* @param[in] i_suffixArr        allow options ended with ".suffix" i.e.: "EN.RunDescription"
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
    const size_t i_prefixSize,
    const char ** i_prefixArr,
    const size_t i_suffixSize,
    const char ** i_suffixArr
)
{
    // if no command line arguments then return empty options
    if (argc <= 1) return;

    // validate parameters
    if (argc < 1 || argv == nullptr) throw HelperException(LT("Invalid (empty) list of command line arguments, expected at least one"));
    if (i_keyArrSize <= 0 || i_keyArrSize > SHRT_MAX || i_keyArr == nullptr) throw HelperException(LT("Invalid (or empty) list of option keys"));
    if (i_shortArrSize < 0 || i_shortArrSize > SHRT_MAX || (i_shortArrSize != 0 && i_shortPairArr == nullptr)) throw HelperException(LT("Invalid (or empty) list of option short keys"));
    if (i_prefixSize < 0 || i_prefixSize > SHRT_MAX || (i_prefixSize != 0 && i_prefixArr == nullptr)) throw HelperException(LT("Invalid (or empty) list of option prefixes"));
    if (i_suffixSize < 0 || i_suffixSize > SHRT_MAX || (i_suffixSize != 0 && i_suffixArr == nullptr)) throw HelperException(LT("Invalid (or empty) list of option suffixes"));

    // past the end of option names array
    const char ** endOfKeyArr = i_keyArr + i_keyArrSize;
    const pair<const char *, const char *> * endOfShortArr = i_shortPairArr + i_shortArrSize;

    // make of "Prefix." and ".Suffix" options
    vector<string> preVec;
    if (i_prefixSize > 0 && i_prefixArr != nullptr) {
        for (size_t k = 0; k < i_prefixSize; k++) {
            preVec.push_back(string(i_prefixArr[k]) + ".");
        }
    }
    vector<string> sfxVec;
    if (i_suffixSize > 0 && i_suffixArr != nullptr) {
        for (size_t k = 0; k < i_suffixSize; k++) {
            sfxVec.push_back(string(".") + i_suffixArr[k]);
        }
    }

    // find keys in the names array and append key-value pairs into result
    for (int nArg = 1; nArg < argc; nArg++) {

        // command line key must start from -
        string sKey = toUtf8(argv[nArg]);
        if (sKey.empty() || sKey[0] != '-') throw HelperException(LT("Invalid command line option %s"), argv[nArg]);

        sKey = sKey.substr(1);  // remove leading -

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

            bool isUnk = true;
            for (vector<string>::const_iterator it = preVec.cbegin(); isUnk && it != preVec.cend(); it++) {
                isUnk = !startWithNoCase(sKey, it->c_str());
            }
            for (vector<string>::const_iterator it = sfxVec.cbegin(); isUnk && it != sfxVec.cend(); it++) {
                isUnk = !endWithNoCase(sKey, it->c_str());
            }
            if (isUnk) {
                if (i_isThrowUnknown) throw HelperException(LT("Invalid command line option %s"), argv[nArg]);
                if (!i_isStoreUnknown) continue;
            }
        }
        
        // get parameter value and save it in the run options map
        // if next command line argument starts with - then it is a next key, store "" empty value for current key
        string sVal = "";
        if (nArg + 1 < argc) {
            if (argv[nArg + 1] != nullptr && argv[nArg + 1][0] != '-') sVal = toUtf8(argv[++nArg]);
        }
        args[sKey] = sVal;
    }
}

/**
* read ini-file and merge with command line arguments.
*
* @param[in] i_filePath           path to ini-file
* @param[in] i_keyArrSize         size of i_keyArr, must be positive
* @param[in] i_keyArr             array of allowed keys: full key names
* @param[in] i_sectionToMergeSize number of section names to merge
* @param[in] i_sectionToMergeArr  if not NULL then merge section from ini-file, ex: "Parameter" or "SubValue"
* @param[in] i_multiKeySize       size of i_multiKeyArr array
* @param[in] i_multiKeyArr        array of keys which can be used in any sections, ex: "EN.RunDescription"
*/
void ArgReader::loadIniFile(
    const char * i_filePath,
    const size_t i_keyArrSize,
    const char ** i_keyArr,
    const size_t i_sectionToMergeSize,
    const char ** i_sectionToMergeArr,
    const size_t i_multiKeySize,
    const char ** i_multiKeyArr
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
    if (i_sectionToMergeSize > 0 && i_sectionToMergeArr != nullptr) {

        for (size_t k = 0; k < i_sectionToMergeSize; k++) {
            NoCaseMap sect = iniRd.getSection(i_sectionToMergeArr[k]);
            for (const auto & ent : sect) {
                string optKey = string(i_sectionToMergeArr[k]) + "." + ent.first;
                if (args.find(optKey) == args.cend()) args[optKey] = ent.second;
            }
        }
    }

    // there are options which distiguished by suffix, ex: "-EN.RunDescription"
    // corresponding ini-flie keys can exist in multiple sections
    // append such ini-file option if it is not already defined on command line
    if (i_sectionToMergeSize > 0 && i_sectionToMergeArr != nullptr) {
        for (const auto & ini : iniRd.rowsCRef()) {

            bool isFound = false;
            for (size_t n = 0; !isFound && n < i_multiKeySize; n++) {
                isFound = equalNoCase(i_multiKeyArr[n], ini.key.c_str());   // is ini key a one of suffix-based options
            }
            if (!isFound) continue;     // skip this ini key, it is not a suffix-based option

            string optKey = ini.section + "." + ini.key;
            if (args.find(optKey) == args.cend()) args[optKey] = ini.val;   // add from ini-file to options map
        }
    }
}

/** adjust log file settings, ie: make default log file path if required. */
void ArgReader::adjustLogSettings(int argc, char ** argv)
{
    // validate parameters
    if (argc < 1 || argv == nullptr) throw HelperException(LT("Invalid (empty) list of command line arguments, expected at least one"));

    // dependency in log options: 
    // if log to stamped file then timestamp or pid stamp must be defined
    if (boolOption(ArgKey::logToStamped) && !boolOption(ArgKey::logUseTs) && !boolOption(ArgKey::logUsePid)) {
        args[ArgKey::logUseTs] = "true";
    }
     if (!boolOption(ArgKey::logToStamped) && (boolOption(ArgKey::logUseTs) || boolOption(ArgKey::logUsePid))) {
        args[ArgKey::logToStamped] = "true";
    }

     // dependency in log options: 
     // if log to file not empty and no any log-to flags set then set log to file as true
     if (isOptionExist(ArgKey::logFilePath) && !boolOption(ArgKey::logToFile) && !boolOption(ArgKey::logToStamped)) {
         args[ArgKey::logToFile] = "true";
     }
}
